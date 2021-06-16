/**
 * - pico-examples, Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * - modified by Mark Komus 2021: captures data and outputs to a CSV sigrok/pulseview format
 */

#include "config.h"

#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/clocks.h>
#include <microshell.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Boost the baud rate to try to get the data out faster
// Probably should just call the init with the baud rate option set
#undef PICO_DEFAULT_UART_BAUD_RATE
#define PICO_DEFAULT_UART_BAUD_RATE 921600

// Defaults - just what I tested with any legal value is fine
uint CAPTURE_PIN_BASE = 17;
uint CAPTURE_PIN_COUNT = 2;
uint CAPTURE_N_SAMPLES = 200000;
float FREQ_DIV = 125.0f; // Divide 125Mhz by this to get your freq
uint FREQUENCY = 1000000;
bool TRIGGER = false; // true = high : false = low

uint offset;
struct pio_program *capture_prog_2;

static uint32_t *capture_buf = 0;
static PIO pio = pio0;
static uint sm = 0;
static uint dma_chan = 0;

extern ush_object_ptr_t ush;

void logic_analyser_init(PIO pio, uint sm, uint pin_base, uint pin_count, float div) {
    // Load a program to capture n pins. This is just a single `in pins, n`
    // instruction with a wrap.
    uint16_t capture_prog_instr = pio_encode_in(pio_pins, pin_count);
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
    sm_config_set_in_pins(&c, pin_base);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, div);
    sm_config_set_in_shift(&c, true, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);
}

void logic_analyser_arm(PIO pio, uint sm, uint dma_chan, uint32_t *capture_buf, size_t capture_size_words, uint trigger_pin, bool trigger_level) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_clear_fifos(pio, sm);

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));

    dma_channel_configure(dma_chan, &c,
        capture_buf,        // Destinatinon pointer
        &pio->rxf[sm],      // Source pointer
        capture_size_words, // Number of transfers
        true                // Start immediately
    );

    pio_sm_exec(pio, sm, pio_encode_wait_gpio(trigger_level, trigger_pin));
    pio_sm_set_enabled(pio, sm, true);
}

void print_capture_buf_csv(const uint32_t *buf, uint pin_base, uint pin_count, uint32_t n_samples) {
    for (int sample = 0; sample < n_samples; ++sample) {
        for (int pin = 0; pin < pin_count; ++pin) {
            uint bit_index = pin + sample * pin_count;
            bool level = !!(buf[bit_index / 32] & 1u << (bit_index % 32));
            printf(level ? "1" : "0");
            printf(",");
        }

        // Blink the LED every 2500 samples to show something is happening
        // Good for a serial capture where you cannot see if it is still outputting
        if ((sample % 5000) == 0)
            gpio_put(LED_PIN, 1);
        else if ((sample % 5000) == 2500)
            gpio_put(LED_PIN, 0);

        printf("\n");
    }
}

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
        CAPTURE_PIN_BASE = pin;
    }
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
        CAPTURE_PIN_COUNT = number;
    }
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
        FREQUENCY = freq;
        FREQ_DIV = clock_get_hz(clk_sys) / (float)FREQUENCY;
        printf("Frequency is %d div is %f\n", FREQUENCY, FREQ_DIV);
    }
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
        TRIGGER = t;
        printf("Trigger set to %d\n", TRIGGER);
    }
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
        CAPTURE_N_SAMPLES = number;
    }
}

static void ush_handler_exec_go(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
}

// files descriptor and node
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

static void ush_handler_exec_sigrock(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
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

void cdc_sigrock_init(void) {
    ush_node_mount(ush, "/bin/sigrock", &ush_node_sigrock_files, ush_files_sigrock, sizeof(ush_files_sigrock) / sizeof(ush_files_sigrock[0]));
    ush_commands_add(ush, &ush_node_sigrock_cmds, ush_cmds_sigrock, sizeof(ush_cmds_sigrock) / sizeof(ush_cmds_sigrock[0]));
}

void cdc_sigrock_task(void) {
    gpio_put(LED_PIN, 1);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);

    uint32_t capture_buf_memory_size = (CAPTURE_PIN_COUNT * CAPTURE_N_SAMPLES + 31) / 32 * sizeof(uint32_t);
    capture_buf = malloc(capture_buf_memory_size);
    if (capture_buf == NULL) {
        printf("Error allocating capture buffer size %d\n", capture_buf_memory_size);
    }

    logic_analyser_init(pio, sm, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, FREQ_DIV);

    uint32_t hz = clock_get_hz(clk_sys);
    printf("Clock speed is   %d\n", hz);
    float caphz = (float)hz/FREQ_DIV;
    printf("Capture speed is %f.2\n", caphz);

    printf("Arming trigger\n");
    gpio_put(LED_PIN, 1);

    logic_analyser_arm(pio, sm, dma_chan, capture_buf,
        (CAPTURE_PIN_COUNT * CAPTURE_N_SAMPLES + 31) / 32,
        CAPTURE_PIN_BASE, TRIGGER);

    dma_channel_wait_for_finish_blocking(dma_chan);

    gpio_put(LED_PIN, 0);
    print_capture_buf_csv(capture_buf, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, CAPTURE_N_SAMPLES);

    pio_remove_program(pio, capture_prog_2, offset);

    free(capture_buf);
}