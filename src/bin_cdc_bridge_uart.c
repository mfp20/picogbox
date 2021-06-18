#include "picogbox.h"
#include "log.h"
#include "tusb.h"
#include "bin_cdc_bridge_uart.h"

#include <pico/stdlib.h>
//#include <pico/stdio.h>
//#include <pico/stdio_uart.h>
#include <pico/stdio/driver.h>

//APP_CDC_BRIDGE_UART0_INTF
//APP_CDC_BRIDGE_UART1_INTF

void cdc_uart_init(void) {
    gpio_set_function(APP_CDC_BRIDGE_UART0_PIN_TX, GPIO_FUNC_UART);
    gpio_set_function(APP_CDC_BRIDGE_UART0_PIN_RX, GPIO_FUNC_UART);
    uart_init(APP_CDC_BRIDGE_UART0_INST, APP_CDC_BRIDGE_UART0_BAUDRATE);

    gpio_set_function(APP_CDC_BRIDGE_UART1_PIN_TX, GPIO_FUNC_UART);
    gpio_set_function(APP_CDC_BRIDGE_UART1_PIN_RX, GPIO_FUNC_UART);
    uart_init(APP_CDC_BRIDGE_UART1_INST, APP_CDC_BRIDGE_UART1_BAUDRATE);
}

#define MAX_UART_PKT 64
void cdc_uart_task(void) {
    uint8_t rx0_buf[MAX_UART_PKT];
    uint8_t tx0_buf[MAX_UART_PKT];
    uint8_t rx1_buf[MAX_UART_PKT];
    uint8_t tx1_buf[MAX_UART_PKT];

    // Consume uart fifo regardless even if not connected
    uint rx0_len = 0;
    uint rx1_len = 0;
    while(uart_is_readable(APP_CDC_BRIDGE_UART0_INST) && (rx0_len < MAX_UART_PKT)) {
        rx0_buf[rx0_len++] = uart_getc(APP_CDC_BRIDGE_UART0_INST);
    }
    while(uart_is_readable(APP_CDC_BRIDGE_UART1_INST) && (rx1_len < MAX_UART_PKT)) {
        rx1_buf[rx1_len++] = uart_getc(APP_CDC_BRIDGE_UART1_INST);
    }

    if (tud_cdc_n_connected(APP_CDC_BRIDGE_UART0_INTF)) {
        // Do we have anything to display on the host's terminal?
        if (rx0_len) {
            for (uint i = 0; i < rx0_len; i++) {
                tud_cdc_n_write_char(APP_CDC_BRIDGE_UART0_INTF, rx0_buf[i]);
            }
            tud_cdc_n_write_flush(APP_CDC_BRIDGE_UART0_INTF);
        }
        if (tud_cdc_n_available(APP_CDC_BRIDGE_UART0_INTF)) {
            // Is there any data from the host for us to tx
            uint tx0_len = tud_cdc_n_read(APP_CDC_BRIDGE_UART0_INTF, tx0_buf, sizeof(tx0_buf));
            uart_write_blocking(APP_CDC_BRIDGE_UART0_INST, tx0_buf, tx0_len);
        }
    }

    if (tud_cdc_n_connected(APP_CDC_BRIDGE_UART1_INTF)) {
        // Do we have anything to display on the host's terminal?
        if (rx1_len) {
            for (uint i = 0; i < rx1_len; i++) {
                tud_cdc_n_write_char(APP_CDC_BRIDGE_UART1_INTF, rx1_buf[i]);
            }
            tud_cdc_n_write_flush(APP_CDC_BRIDGE_UART1_INTF);
        }
        if (tud_cdc_n_available(APP_CDC_BRIDGE_UART1_INTF)) {
            // Is there any data from the host for us to tx
            uint tx1_len = tud_cdc_n_read(APP_CDC_BRIDGE_UART1_INTF, tx1_buf, sizeof(tx1_buf));
            uart_write_blocking(APP_CDC_BRIDGE_UART1_INST, tx1_buf, tx1_len);
        }
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding) {
    LOG_INF("CDC %d: new baud rate %d", itf, line_coding->bit_rate);
    if (itf == APP_CDC_BRIDGE_UART0_INTF) {
        uart_init(APP_CDC_BRIDGE_UART0_INST, line_coding->bit_rate);
    } else if (itf == APP_CDC_BRIDGE_UART1_INTF) {
        uart_init(APP_CDC_BRIDGE_UART1_INST, line_coding->bit_rate);
    } else if (itf == APP_CDC_SUMP_INTF) {
    }
}

static void stdio_uart0_out_chars(const char *buf, int len) {
}
static void stdio_uart0_out_flush(void) {
}
static int stdio_uart0_in_chars(char *buf, int len) {
}
stdio_driver_t stdio_uart0_driver = { 
    .out_chars = stdio_uart0_out_chars,
    .out_flush = stdio_uart0_out_flush,
    .in_chars = stdio_uart0_in_chars
};
