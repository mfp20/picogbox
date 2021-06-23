/**
 * - pico-examples, Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * - modified by Mark Komus 2021: captures data and outputs to a CSV sigrok/pulseview format
 * - modified by Anichang 2021: runs on core1 and outputs to USB CDC in realtime
 */

#include "picogbox.h"
#include "log.h"
#include "manager.h"
#include "pico_led.h"
#include "bin_cdc_microshell.h"
#include "bin_cdc_capture.h"

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

#include <unistd.h>

enum capture_state_e {
    STATE_OFF = 0,
    STATE_IDLE,
    STATE_PREPARE,
    STATE_START,
    STATE_RUNNING,
    STATE_CLEAN
};

enum capture_cmd_e {
    CMD_START = 0,
    CMD_ABORT = 255
};

// defaults
static uint8 cdc_no = 0;
static uint8 dma_chan = 0;
static PIO pio;
static struct pio_program *pio_prog;
static uint pio_offset;
static uint32_t *capture_buf = 0;
static uint32_t capture_buf_size = 0;
static uint32_t capture_buf_read_index = 0;
static uint32_t capture_buf_write_index = 0;
static uint capture_pin_base = 17;              // first pin and trigger pin
static uint capture_pin_count = 2;
static uint capture_frequency = 1000000;
static float capture_freq_div = 125.0f;         // Divide 125Mhz by this to get your freq
static bool capture_polarity = false;           // true = high : false = low
static uint capture_max_samples = 200000;
static uint32_t capture_current_sample = 0;
static uint8 state = STATE_OFF;

// called on core0 to receive each sample and store in buffer for tx (to usb cdc host) on next loop
static void logic_analyser_rx_isr() {
    // store in buffer the latest entry
    while (multicore_fifo_rvalid()) {
        if (capture_buf_write_index<capture_buf_size)
            capture_buf[capture_buf_write_index++] = multicore_fifo_pop_blocking();
        else
            // abort, buffer full
            LOG_ERR("buffer full, aborting");
            state = STATE_CLEAN;
    }
    // clear interrupt request
    multicore_fifo_clear_irq();
}

// called on core1 to receive commands
static void logic_analyser_cmd_isr(void) {
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

// called on core1 to send each sample to core0
static void logic_analyser_tx_isr(void) {
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(capture_current_sample);
    } else {
        // if write fifo is full, data is lost: abort
        dma_channel_abort(dma_chan);
        dma_channel_set_irq0_enabled(dma_chan, false);
        irq_set_enabled(SIO_IRQ_PROC1, false);
        pio_remove_program(pio, pio_prog, pio_offset);
    }
    // clear interrupt request
    //dma_hw->ints0 = 1u << dma_chan;
    irq_clear(DMA_IRQ_0);
}

// core1 main entry
static void logic_analyser_session(void) {
    // pio state machine
    uint sm = 0;

    // setup core fifo
    multicore_fifo_clear_irq();
    // enable multicore IRQ
    irq_set_exclusive_handler(SIO_IRQ_PROC1, logic_analyser_cmd_isr);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    // setup PIO: load a program to capture n pins.
    // This is just a single `in pins, n` instruction with a wrap.
    uint16_t capture_prog_instr = pio_encode_in(pio_pins, capture_pin_count);
    struct pio_program capture_prog = {
        .instructions = &capture_prog_instr,
        .length = 1,
        .origin = -1
    };
    pio_prog = &capture_prog;
    pio_offset = pio_add_program(pio, &capture_prog);
    // Configure state machine to loop over this `in` instruction forever, with autopush enabled.
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, capture_pin_base);
    sm_config_set_wrap(&c, pio_offset, pio_offset);
    sm_config_set_clkdiv(&c, capture_freq_div);
    sm_config_set_in_shift(&c, true, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, pio_offset, &c);
    // configuration done
    uint32_t hz = clock_get_hz(clk_sys);
    LOG_INF("Logic Analyser - clock speed %d - capture speed %f.2", hz, (float)hz/capture_freq_div);
    
    // wait for start command
    multicore_fifo_pop_blocking();
    LOG_INF("Arming trigger");

    // arm capture trigger and start capture on first pulse
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_clear_fifos(pio, sm);
    //
    dma_channel_config dma_c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_c, false);
    channel_config_set_write_increment(&dma_c, false);
    channel_config_set_dreq(&dma_c, pio_get_dreq(pio, sm, false));
    //
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, logic_analyser_tx_isr);
    irq_set_enabled(DMA_IRQ_0, true);
    //
    dma_channel_configure(dma_chan, &dma_c,
        &capture_current_sample,                                // Destinatinon pointer
        &pio->rxf[sm],                                          // Source pointer
        (capture_pin_count * capture_max_samples + 31) / 32,    // Number of transfers
        true                                                    // Start immediately
    );
    //
    pio_sm_exec(pio, sm, pio_encode_wait_gpio(capture_polarity, capture_pin_base));
    pio_sm_set_enabled(pio, sm, true);
    // wait for finish
    dma_channel_wait_for_finish_blocking(dma_chan);
    // disable IRQs
    dma_channel_set_irq0_enabled(dma_chan, false);
    irq_set_enabled(SIO_IRQ_PROC1, false);
    // clear PIO
    pio_remove_program(pio, pio_prog, pio_offset);
}

void bin_cdc_capture_task(void *data);
static const consumer_meta_t user = {
    .name = "LOGIC CAPTURE",
    .task = bin_cdc_capture_task,
    .allow_multiple_exec = false
};

// setters and FSM
static void logic_analyser_set_cdc(uint8 cdc) {
    // free previous alloc
    usb_cdc_free(cdc);
    //
    cdc_no = cdc;
    // allocate cdc
    if (usb_cdc_alloc(cdc_no, &user)<0) {
        LOG_ERR("Coudn't bind Capture to USB CDC %d", cdc_no);
    } 
}
static void logic_analyser_set_pin(uint8 pin) {
    if (pin > 29) {
        LOG_ERR("Pin number is not valid");
    } else {
        // free previous alloc
        pin_free(capture_pin_base);
        //
        capture_pin_base = pin;
        // allocate first/trigger pin
        if (pin_alloc(capture_pin_base, &user)<0) {
            LOG_ERR("%s can't allocate pin %d", user.name, capture_pin_base);
        }
        LOG_INF("Start pin is %d", pin);
    }
}
static void logic_analyser_set_pin_count(uint8 number) {
    if (number > 29) {
        LOG_ERR("Number of pins is not valid");
    } else {
        // free previous alloc
        for (int i=1;i<capture_pin_count;i++) {
            pin_free(capture_pin_base+i);
        }
        //
        capture_pin_count = number;
        // allocate pins
        for (int i=1;i<capture_pin_count;i++) {
            if (pin_alloc(capture_pin_base+i, &user)<0) {
                LOG_WAR("%s can't allocate pin %d", user.name, capture_pin_base+i);
                for (int k=0;k<=i;k++) {
                    pin_free(capture_pin_base+k);
                }
                return;
            }
        }
        LOG_INF("Total pins is %d", number);
    }
}
static void logic_analyser_set_freq(uint32 freq) {
    if (freq > clock_get_hz(clk_sys)) {
        LOG_ERR("Frequency is not valid");
    } else {
        capture_frequency = freq;
        capture_freq_div = clock_get_hz(clk_sys) / (float)capture_frequency;
        LOG_INF("Frequency is %d div is %f", capture_frequency, capture_freq_div);
    }
}
static void logic_analyser_set_polarity(uint8 t) {
    if (t < 0) {
        LOG_ERR("Trigger value is not valid");
    } else {
        capture_polarity = t;
        LOG_INF("Trigger set to %d\n", capture_polarity);
    }
}
static void logic_analyser_set_samples(uint32 number) {
    if (number < 0) {
        LOG_ERR("Sample number is not valid");
    } else {
        capture_max_samples = number;
        LOG_INF("Sample number is %d", number);
    }
}
static void logic_analyser_start(void) {
    state = STATE_PREPARE;
}

// app capture folder
static size_t ush_handler_get_capture_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info = "Logic analyser with sigrok output. Setup all params then call 'go'\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_capture[] = {
    {
        .name = "README",
        .description = "use 'cat README' to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_capture_readme,
    }
};

// capture command
static void ush_handler_exec_capture(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // parse options
    int c;
    bool broken = false;
    while ((c = getopt(argc, argv, "u:p:a:f:t:s:")) != -1) {
        //ush_printf("ush_printf %c %s\n\r", optopt, optarg);
        switch (c) {
            case 'u':
                logic_analyser_set_cdc(strtol(optarg, NULL, 10));
            break;
            case 'p':
                logic_analyser_set_pin(strtol(optarg, NULL, 10));
            break;
            case 'a':
                logic_analyser_set_pin_count(strtol(optarg, NULL, 10));
            break;
            case 'f':
                logic_analyser_set_freq(strtol(optarg, NULL, 10));
            break;
            case 't':
                if (optarg[0] == 'h' || optarg[0] == 't' || optarg[0] == '1') {
                    logic_analyser_set_polarity(1);
                } else if (optarg[0] == 'l' || optarg[0] == 'f' || optarg[0] == '0') {
                    logic_analyser_set_polarity(0);
                }
            break;
            case 's':
                logic_analyser_set_samples(strtol(optarg, NULL, 10));
            break;
            case '?':
                if ((optopt == 'p')||(optopt == 'a')||(optopt == 'f')||(optopt == 't')||(optopt == 's')) {
                    ush_printf("Option -%c requires an argument.\n\r", optopt);
                } else if (isprint (optopt)) {
                    ush_printf("Unknown option.\n\r");
                } else {
                    ush_printf("Unknown option character `\\x%x'.\n\r", optopt);
                }
                broken = true;
            break;
            default:
                ush_printf("Wrong option syntax.\n\r");
            break;
        }
        if (broken) return;
    }

    ush_printf("options parsed\n\r");

    // first run
    if (state==STATE_OFF) {
        state=STATE_IDLE;
    }

    // go?
    if (strcmp(argv[optind], "go")==0) {
        //ush_printf("%s!\n\r", argv[optind]);
        //logic_analyser_start();
    }

    ush_printf("capture started\n\r");

    // reset optarg global
    optind = 0;
}
static const struct ush_file_descriptor ush_cmds_capture[] = {
    {
        .name = "capture",
        .description = "start capture app with existing parameters or providing inline new params",
        .help = "usage: capture cdc [pin num freq polarity samples [go]]",
        .exec = ush_handler_exec_capture,
    },
};
static struct ush_node_object ush_node_capture_files, ush_node_capture_cmds;

void bin_cdc_capture_task(void *data) {
    int8 dma_id;
    switch (state) {
        case STATE_OFF:
            tight_loop_contents();
        break;
        case STATE_IDLE:
            tight_loop_contents();
        break;
        case STATE_PREPARE:
            LOG_INF("STATE_PREPARE");
            // alloc dma and pio
            dma_id = dma_get(&user);
            if (dma_id<0) {
                LOG_WAR("%s can't allocate a DMA channel", user.name)
                return;
            } else {
                dma_chan = DMA_DEF[dma_id].id;
            }
            int8 pio_n = pio_get(&user);
            if (pio_n<0) {
                LOG_WAR("%s can't allocate PIO%d", user.name, BIN_VENDOR_SWD_PIO)
                for (int i=0;i<capture_pin_count;i++) {
                    pin_free(capture_pin_base+i);
                }
                dma_free(dma_chan);
                return;
            } else {
                pio = PIO_DEF[pio_n].id;
            }
            // alloc buffer
            capture_buf_size  = (capture_pin_count * capture_max_samples + 31) / 32;
            capture_buf = malloc(capture_buf_size * sizeof(uint32_t));
            if (capture_buf == NULL) {
                LOG_ERR("Error allocating capture buffer size %d", capture_buf_size * sizeof(uint32_t));
            }
            // run code on core1
            multicore_fifo_clear_irq();
            multicore_launch_core1(logic_analyser_session);
            irq_set_exclusive_handler(SIO_IRQ_PROC0, logic_analyser_rx_isr);
            irq_set_enabled(SIO_IRQ_PROC0, true);
            state = STATE_START;
        break;
        case STATE_START:
            LOG_INF("STATE_START");
            // start
            multicore_fifo_push_blocking(CMD_START);
            state = STATE_RUNNING;
        break;
        case STATE_RUNNING:
            LOG_INF("STATE_RUNNING");
            if (capture_buf_read_index<capture_buf_write_index) {
                for (int pin = 0; pin < capture_pin_count; ++pin) {
                    uint bit_index = pin + capture_buf_write_index * capture_pin_count;
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
            }
            if (capture_buf_write_index>=capture_buf_size) {
                state = STATE_CLEAN;
            }
        break;
        case STATE_CLEAN:
            LOG_INF("STATE_CLEAN");
            // reset core1
            multicore_reset_core1();
            // free buffer
            free(capture_buf);
            // reset counters
            capture_buf_read_index = capture_buf_write_index = 0;
            //
            state = STATE_IDLE;
        break;
    }
}

void bin_cdc_capture_init(uint8 cdc) {
    cdc_no = cdc;
    //task_add((task_t *)&user.task);
    ush_node_mount(ush, "/bin/capture", &ush_node_capture_files, ush_files_capture, sizeof(ush_files_capture) / sizeof(ush_files_capture[0]));
    ush_commands_add(ush, &ush_node_capture_cmds, ush_cmds_capture, sizeof(ush_cmds_capture) / sizeof(ush_cmds_capture[0]));
    LOG_INF("Capture init");
}
