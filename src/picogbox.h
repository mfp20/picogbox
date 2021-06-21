#ifndef PICOGBOX_H_
#define PICOGBOX_H_

// log config
#ifdef BUILD_DEBUG
#define LOG_LEVEL (7)
#else
#define LOG_LEVEL (6)
#endif
#define LOG_COLOR (1)

// UARTx bridge
#define APP_CDC_BRIDGE_UART0_INTF       1
#define APP_CDC_BRIDGE_UART0_INST       uart0
#define APP_CDC_BRIDGE_UART1_INTF       2
#define APP_CDC_BRIDGE_UART1_INST       uart1
#define UART_DEFAULT_BAUDRATE           115200
#define UART_MAX_BAUDRATE               921600

// SWD protocol
#define BIN_VENDOR_SWD_SM 0
#define BIN_VENDOR_SWD_PIN_OFFSET 2
#define BIN_VENDOR_SWD_PIN_SWCLK BIN_VENDOR_SWD_PIN_OFFSET + 0 // 2
#define BIN_VENDOR_SWD_PIN_SWDIO BIN_VENDOR_SWD_PIN_OFFSET + 1 // 3
#define BIN_VENDOR_SWD_PIN_RESET 28
#define BIN_VENDOR_SWD_PIO 0

// sigrock
#define BIN_VENDOR_SIGROCK_PIO 1

// SUMP protocol
#define APP_CDC_SUMP_INTF		        5
#define APP_CDC_SUMP_PIN_SAMPLING_FIRST	6
#define APP_CDC_SUMP_PIN_SAMPLING_LAST  21
#define APP_CDC_SUMP_PIN_SAMPLING_TEST  22

#include "gcc.h"
#include "types.h"

uint32 monotonic32(void);
uint64 monotonic64(void);

#endif
