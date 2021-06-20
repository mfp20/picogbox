#include "log.h"
#include "manager.h"
#include "managed_uart.h"
#include "bin_cdc_bridge_uart.h"

#include <tusb.h>
#include <pico/stdlib.h>

#define MAX_UART_PKT 128

typedef struct cdc_uart_bridge_s {
    uint8 uart_id;
    uint8 cdc_id;
} cdc_uart_bridge_t;

cdc_uart_bridge_t bridge[2];
uint8_t rx_buf[2][MAX_UART_PKT];
uint8_t tx_buf[2][MAX_UART_PKT];
uint rx_len[2] = {0, 0};
uint tx_len[2] = {0, 0};

static void cdc_bridge_uart_task(uint8 id) {    
    uart_inst_t *uart;
    if (id) {
        uart = uart1;
    } else {
        uart = uart0;
    }

    // Consume uart fifo regardless even if not connected
    while(uart_is_readable(uart) && (rx_len[id] < MAX_UART_PKT)) {
        rx_buf[id][rx_len[id]++] = uart_getc(uart);
    }

    if (tud_cdc_n_connected(bridge[id].cdc_id)) {
        // Do we have anything to display on the host's terminal?
        if (rx_len[id]) {
            for (uint i = 0; i < rx_len[id]; i++) {
                tud_cdc_n_write_char(bridge[id].cdc_id, rx_buf[id][i]);
            }
            tud_cdc_n_write_flush(bridge[id].cdc_id);
        }
        if (tud_cdc_n_available(bridge[id].cdc_id)) {
            // Is there any data from the host for us to tx
            tx_len[id] = tud_cdc_n_read(bridge[id].cdc_id, tx_buf[id], sizeof(tx_buf[id]));
            uart_write_blocking(uart, tx_buf[id], tx_len[id]);
        }
    }
}
static void cdc_bridge_uart0_task(void *data) {
    cdc_bridge_uart_task(0);
}
static void cdc_bridge_uart1_task(void *data) {
    cdc_bridge_uart_task(1);
}

static const consumer_meta_t user0 = {
    .name = "USB-UART0 BRIDGE",
    .task = cdc_bridge_uart0_task
};
static const consumer_meta_t user1 = {
    .name = "USB-UART1 BRIDGE",
    .task = cdc_bridge_uart1_task
};

void bin_cdc_bridge_uart_init(uint8 uart, uint8 cdc) {
    const consumer_meta_t *user;
    // init uart
    if (uart) {
        user = &user1;
    } else {
        user = &user0;
    }
    managed_uart_init(uart, user, false);
    usb_cdc_alloc(cdc, user);

    // setup ids
    bridge[uart].uart_id = uart;
    bridge[uart].cdc_id = cdc;
    LOG_INF("Bridging UART%d to USB CDC%d", uart, cdc);
}
