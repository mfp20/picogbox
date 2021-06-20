
#include "log.h"
#include "manager.h"
#include "managed_uart.h"

#include <pico/stdlib.h>

static void stdio_uart0_out_chars(const char *buf, int len) {
    for (int i=0;i<len;i++) {
        uart_putc(uart0, buf[i]);
    }
}
static void stdio_uart0_out_flush(void) {
}
static int stdio_uart0_in_chars(char *buf, int len) {
    int i=0;
    while (i<len && uart_is_readable(uart0)) {
        buf[i++] = uart_getc(uart0);
    }
    return i ? i : PICO_ERROR_NO_DATA;
}
stdio_driver_t stdio_uart0_driver = { 
    .out_chars = stdio_uart0_out_chars,
    .out_flush = stdio_uart0_out_flush,
    .in_chars = stdio_uart0_in_chars,
    .crlf_enabled = true
};

static void stdio_uart1_out_chars(const char *buf, int len) {
    for (int i = 0; i <len; i++) {
        uart_putc(uart1, buf[i]);
    }
}
static void stdio_uart1_out_flush(void) {
}
static int stdio_uart1_in_chars(char *buf, int len) {
    int i=0;
    while (i<len && uart_is_readable(uart1)) {
        buf[i++] = uart_getc(uart1);
    }
    return i ? i : PICO_ERROR_NO_DATA;
}
stdio_driver_t stdio_uart1_driver = { 
    .out_chars = stdio_uart1_out_chars,
    .out_flush = stdio_uart1_out_flush,
    .in_chars = stdio_uart1_in_chars,
    .crlf_enabled = true
};

void managed_uart_init(uint8 id, consumer_meta_t const* user, bool stdio) {
    if (id) {
        uart_alloc(1, user, 8, 9);
    } else {
        uart_alloc(0, user, 0, 1);
    }
    LOG_INF("UART%d init", id);
    if (stdio) {
        managed_uart_stdio(id, stdio);
    }
}

void managed_uart_stdio(uint8 id, bool stdio) {
    if (id) {
        stdio_set_driver_enabled(&stdio_uart1_driver, stdio);
        LOG_INF("Standard I/O on UART1");
    } else {
        stdio_set_driver_enabled(&stdio_uart0_driver, stdio);
        LOG_INF("Standard I/O on UART0");
    }
    LOG_EME("emergency log entry");
    LOG_ALE("alert log entry");
    LOG_CRI("critical log entry");
    LOG_ERR("error log entry");
    LOG_WAR("warning log entry");
    LOG_NOT("notice log entry");
    LOG_INF("info log entry");
    LOG_DEB("debug log entry");
    unsigned char data[20] = {32, 1, 24, 56, 102, 5, 78, 92, 200, 0, 32, 1, 24, 56, 102, 5, 78, 92, 200, 0};
    LOG_HEX(data, 20, "20 bytes hex %s", "log entry");
}
