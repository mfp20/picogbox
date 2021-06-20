#ifndef MANAGED_UART_H
#define MANAGED_UART_H

#include "picogbox.h"

#include <pico/stdio/driver.h>
#include <tusb.h>

extern stdio_driver_t stdio_uart0_driver;
extern stdio_driver_t stdio_uart1_driver;

void managed_uart_init(uint8 id, consumer_meta_t const* user, bool stdio);
void managed_uart_stdio(uint8 id, bool stdio);

#endif
