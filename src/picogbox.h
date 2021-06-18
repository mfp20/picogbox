#ifndef PICOGBOX_CONFIG_H_
#define PICOGBOX_CONFIG_H_

// log config
#ifdef BUILD_DEBUG
#define LOG_LEVEL (7)
#else
#define LOG_LEVEL (5)
#endif
#define LOG_COLOR (1)

// LED config
#define LED_PIN 25

// Console
#define APP_CDC_MICROSHELL_INTF 0

// UARTx bridge
#define APP_CDC_BRIDGE_UART0_INTF       1
#define APP_CDC_BRIDGE_UART0_INST       uart0
#define APP_CDC_BRIDGE_UART0_PIN_TX     0
#define APP_CDC_BRIDGE_UART0_PIN_RX     1
#define APP_CDC_BRIDGE_UART0_BAUDRATE   115200
#define APP_CDC_BRIDGE_UART1_INTF       2
#define APP_CDC_BRIDGE_UART1_INST       uart1
#define APP_CDC_BRIDGE_UART1_PIN_TX     4
#define APP_CDC_BRIDGE_UART1_PIN_RX     5
#define APP_CDC_BRIDGE_UART1_BAUDRATE   115200

// SUMP protocol
#define APP_CDC_SUMP_INTF		        5
#define APP_CDC_SUMP_PIN_SAMPLING_FIRST	6
#define APP_CDC_SUMP_PIN_SAMPLING_LAST  21
#define APP_CDC_SUMP_PIN_SAMPLING_TEST  22

// SWD protocol
#define APP_VENDOR_SWD_SM 0
#define APP_VENDOR_SWD_PIN_OFFSET 2
#define APP_VENDOR_SWD_PIN_SWCLK APP_VENDOR_SWD_PIN_OFFSET + 0 // 2
#define APP_VENDOR_SWD_PIN_SWDIO APP_VENDOR_SWD_PIN_OFFSET + 1 // 3
#define APP_VENDOR_SWD_PIN_RESET 28

#if false
#define TURBO_200MHZ 1
#endif

#include "types.h"
#include "gcc.h"

uint32 monotonic32(void);
uint64 monotonic64(void);

#endif
