/**
 * - pico-examples, Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * - modified by Mark Komus 2021: captures data and outputs to a CSV sigrok/pulseview format
 */

#include "picogbox.h"
#include "log.h"
#include "manager.h"
#include "pico_led.h"
#include "bin_cdc_microshell.h"

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include <hardware/dma.h>
#include <hardware/clocks.h>
#include <tusb.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

enum sigrock_state_e {
    STATE_IDLE = 0,
    STATE_PREPARE,
    STATE_START,
    STATE_RUNNING,
    STATE_CLEAN
};

enum sigrock_cmd_e {
    CMD_START = 0,
    CMD_ABORT = 255
};

// defaults
static uint8 state = STATE_IDLE;
static PIO pio;
static uint32_t *capture_buf = 0;
static uint32_t capture_buf_size = 0;
static uint32_t capture_buf_read_index = 0;
static uint capture_pin_base = 17;
static uint capture_pin_count = 2;
static uint capture_frequency = 1000000;
static float capture_freq_div = 125.0f; // Divide 125Mhz by this to get your freq
static bool capture_trigger = false; // true = high : false = low
static uint capture_max_samples = 200000;
static uint32_t capture_current_sample = 0;
// CDC IO
static uint8 cdc_no = 0;

// runs on core0 to receive each sample and store in buffer for later tx on usb CDC
static uint32_t sample_count = 0;
void logic_analyser_rx_isr() {
    // store in buffer the latest entry
    while (multicore_fifo_rvalid()) {
        capture_buf[sample_count++] = multicore_fifo_pop_blocking();
    }
    // clear interrupt request
    multicore_fifo_clear_irq();
}

// runs on core1 to receive commands
void logic_analyser_cmd_isr(void) {
    uint32_t cmd;
    while (multicore_fifo_rvalid()) {
        cmd = multicore_fifo_pop_blocking();
    }
    switch (cmd) {
        default:
        break;
    }
    // clear interrupt request
    multicore_fifo_clear_irq();
}

// runs on core1 to send each sample to core0
void logic_analyser_tx_isr(void) {
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(capture_current_sample);
    } else {
        // if write fifo is full, abort
        // TODO data loss -> abort
    }
    // clear interrupt request
    irq_clear(DMA_IRQ_0);
}

static void logic_analyser_session(void) {
    uint offset;
    struct pio_program *capture_prog_2;
    uint sm = 0;
    uint dma_chan = 0;

    // setup core fifo
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, logic_analyser_cmd_isr);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    // setup PIO
    // Load a program to capture n pins. This is just a single `in pins, n`
    // instruction with a wrap.
    uint16_t capture_prog_instr = pio_encode_in(pio_pins, capture_pin_count);
    struct pio_program capture_prog = {
        .instructions = &capture_prog_instr,
        .length = 1,
        .origin = -1
    };
    capture_prog_2 = &capture_prog;
    offset = pio_add_program(pio, &capture_prog);
    // Configure state machine to loop over this `in` instruction forever,
    // with autopush enabled.
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, capture_pin_base);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, capture_freq_div);
    sm_config_set_in_shift(&c, true, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);

    // configuration done
    uint32_t hz = clock_get_hz(clk_sys);
    LOG_INF("Logic Analyser - clock speed %d - capture speed %f.2", hz, (float)hz/capture_freq_div);
    
    // wait for start command
    multicore_fifo_pop_blocking();
    LOG_INF("Arming trigger");

    // arm capture and fill the buffer
    size_t capture_size_words = (capture_pin_count * capture_max_samples + 31) / 32;
    uint trigger_pin = capture_pin_base;
    bool trigger_level = capture_trigger;
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_clear_fifos(pio, sm);
    dma_channel_config dma_c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_c, false);
    channel_config_set_write_increment(&dma_c, false);
    channel_config_set_dreq(&dma_c, pio_get_dreq(pio, sm, false));
    // enable IRQ
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, logic_analyser_tx_isr);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_configure(dma_chan, &dma_c,
        &capture_current_sample,    // Destinatinon pointer
        &pio->rxf[sm],      // Source pointer
        capture_size_words, // Number of transfers
        true                // Start immediately
    );
    pio_sm_exec(pio, sm, pio_encode_wait_gpio(trigger_level, trigger_pin));
    pio_sm_set_enabled(pio, sm, true);

    // wait for finish
    dma_channel_wait_for_finish_blocking(dma_chan);
    // disable dma IRQs
    dma_channel_set_irq0_enabled(dma_chan, false);
    // clear PIO
    pio_remove_program(pio, capture_prog_2, offset);
}

// app sigrock folder
static void ush_handler_exec_pin(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    int pin = -1;
    if (isdigit(argv[1][0]) != 0) {
        pin = strtol(argv[1], NULL, 10);
        if (pin > 28)
            pin = -1;
    }

    if (pin == -1)
        printf("Pin number is not valid\n");
    else {
        printf("Start pin is %d\n", pin);
        capture_pin_base = pin;
    }
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_num(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    int number = -1;
    if (isdigit(argv[1][0]) != 0) {
        number = strtol(argv[1], NULL, 10);
        if (number > 28)
            number = -1;
    }

    if (number == -1)
        printf("Number of pins is not valid\n");
    else {
        printf("Total pins is %d\n", number);
        capture_pin_count = number;
    }
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_freq(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    uint freq = 0;
    if (isdigit(argv[1][0]) != 0) {
        freq = strtol(argv[1], NULL, 10);
        if (freq > clock_get_hz(clk_sys))
            freq = 0;
    }

    if (freq < 0)
        printf("Frequency is not valid\n");
    else {
        capture_frequency = freq;
        capture_freq_div = clock_get_hz(clk_sys) / (float)capture_frequency;
        printf("Frequency is %d div is %f\n", capture_frequency, capture_freq_div);
    }
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_trigger(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    int t = -1;
    if (argv[1][0] == 't' || argv[1][0] == '1')
        t = 1;
    else if (argv[1][0] == 'f' || argv[1][0] == '0')
        t = 0;

    if (t < 0)
        printf("Trigger value is not valid\n");
    else {
        capture_trigger = t;
        printf("Trigger set to %d\n", capture_trigger);
    }
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_samples(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    int number = -1;
    if (isdigit(argv[1][0]) != 0) {
        number = strtol(argv[1], NULL, 10);
        if (number < 0 || number > 500000)
            number = -1;
    }

    if (number == -1)
        printf("Sample number is not valid\n");
    else {
        printf("Sample number is %d\n", number);
        capture_max_samples = number;
    }
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_go(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static const struct ush_file_descriptor ush_files_sigrock[] = {
    {
        .name = "p",
        .description = "set the first pin to receive capture data",
        .help = NULL,
        .exec = ush_handler_exec_pin,
    },
    {
        .name = "n",
        .description = "set how many pins to receive capture data",
        .help = NULL,
        .exec = ush_handler_exec_num,
    },
    {
        .name = "f",
        .description = "set the freqency to capture data at in Hz",
        .help = NULL,
        .exec = ush_handler_exec_freq,
    },
    {
        .name = "t",
        .description = "set the trigger to high or low. Trigger happens off first pin",
        .help = NULL,
        .exec = ush_handler_exec_trigger,
    },
    {
        .name = "s",
        .description = "set how many samples to capture",
        .help = NULL,
        .exec = ush_handler_exec_samples,
    },
    {
        .name = "g",
        .description = "go!",
        .help = NULL,
        .exec = ush_handler_exec_go,
    },
};

// sigrock command
static void ush_handler_exec_sigrock(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static const struct ush_file_descriptor ush_cmds_sigrock[] = {
    {
        .name = "sigrock",
        .description = "start sigrock app",
        .help = NULL,
        .exec = ush_handler_exec_sigrock,
    },
};
static struct ush_node_object ush_node_sigrock_files, ush_node_sigrock_cmds;

void cdc_sigrock_task(void *data) {
    return;
    // TODO read_user_input();
    // 1: receive command
    // 2: parse command
    // 3: logic_analyser_session(void)

    switch (state) {
        case STATE_IDLE:
        break;
        case STATE_PREPARE:
            // alloc buffer
            capture_buf_size  = (capture_pin_count * capture_max_samples + 31) / 32;
            capture_buf = malloc(capture_buf_size * sizeof(uint32_t));
            if (capture_buf == NULL) {
                LOG_ERR("Error allocating capture buffer size %d", capture_buf_size * sizeof(uint32_t));
            }
            multicore_launch_core1(logic_analyser_session);
            irq_set_exclusive_handler(SIO_IRQ_PROC0, logic_analyser_rx_isr);
            irq_set_enabled(SIO_IRQ_PROC0, true);
        break;
        case STATE_START:
            multicore_fifo_push_blocking(CMD_START);
        break;
        case STATE_RUNNING:
            for (int pin = 0; pin < capture_pin_count; ++pin) {
                uint bit_index = pin + sample_count * capture_pin_count;
                bool level = !!(capture_buf[capture_buf_read_index++] & 1u << (bit_index % 32));
                if (tud_cdc_n_connected(cdc_no)) {
                    if (tud_cdc_n_write_available(cdc_no) > 0) {
                        // send data
                        tud_cdc_n_write_char(cdc_no, (level ? '1' : '0'));
                        tud_cdc_n_write_char(cdc_no, ',');
                        tud_cdc_n_write_flush(cdc_no);
                        // blink led
                        led_signal_activity(32);
                        continue;
                    }
                    LOG_WAR("CDC%d buffer full", cdc_no);
                    continue;
                }
                LOG_ERR("CDC%d not connected", cdc_no);
                // TODO abort current capture
            }
        break;
        case STATE_CLEAN:
            // free buffer
            free(capture_buf);
            // reset sample counter
            sample_count = 0;
        break;
    }
}

static const consumer_meta_t user = {
    .name = "SIGROCK LOGIC ANALYSER",
    .task = cdc_sigrock_task
};

void cdc_sigrock_init(uint8 cdc) {
    cdc_no = cdc;

    // allocate resources
    if (pin_alloc(BIN_VENDOR_SWD_PIN_SWCLK, &user)<0) {
        LOG_WAR("%s can't allocate SWCLK pin %d", user.name, BIN_VENDOR_SWD_PIN_SWCLK);
        return;
    }
    int8 pio_n = pio_alloc(BIN_VENDOR_SWD_PIO, &user);
    if (pio_n<0) {
        LOG_WAR("%s can't allocate PIO%d", user.name, BIN_VENDOR_SWD_PIO)
        pin_free(BIN_VENDOR_SWD_PIN_SWCLK);
        pin_free(BIN_VENDOR_SWD_PIN_SWDIO);
        pin_free(BIN_VENDOR_SWD_PIN_RESET);
        return;
    } else {
        pio = PIO_DEF[pio_n].id;
    }
    //
    ush_node_mount(ush, "/bin/sigrock", &ush_node_sigrock_files, ush_files_sigrock, sizeof(ush_files_sigrock) / sizeof(ush_files_sigrock[0]));
    ush_commands_add(ush, &ush_node_sigrock_cmds, ush_cmds_sigrock, sizeof(ush_cmds_sigrock) / sizeof(ush_cmds_sigrock[0]));
    LOG_INF("Sigrock init");

    // TODO Boost the baud rate to try to get the data out faster
}
