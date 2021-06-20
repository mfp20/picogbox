#ifndef APP_CDC_BRIDGE_UART_H
#define APP_CDC_BRIDGE_UART_H

#include "picogbox.h"

#include <pico/stdio/driver.h>
#include <tusb.h>

void bin_cdc_bridge_uart_init(uint8 uart, uint8 cdc);

#endif
