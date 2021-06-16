#ifndef PICOGW_CONFIG_H_
#define PICOGW_CONFIG_H_

#include "types.h"

// LED config
#define LED_PIN 25

// Console
#define APP_CDC_MICROSHELL_INTF 0

// UARTx bridge
#define APP_CDC_BRIDGE_UART0_INTF 1
#define APP_CDC_BRIDGE_UART0_INST uart0
#define APP_CDC_BRIDGE_UART0_PIN_TX 0
#define APP_CDC_BRIDGE_UART0_PIN_RX 1
#define APP_CDC_BRIDGE_UART0_BAUDRATE 115200
#define APP_CDC_BRIDGE_UART1_INTF 2
#define APP_CDC_BRIDGE_UART1_INST uart1
#define APP_CDC_BRIDGE_UART1_PIN_TX 4
#define APP_CDC_BRIDGE_UART1_PIN_RX 5
#define APP_CDC_BRIDGE_UART1_BAUDRATE 115200

// SUMP protocol
#define APP_CDC_SUMP_INTF		        2
#define APP_CDC_SUMP_PIN_SAMPLING_FIRST	6
#define APP_CDC_SUMP_PIN_SAMPLING_LAST	    21
#define APP_CDC_SUMP_PIN_SAMPLING_TEST	    22

// SWD protocol
#define APP_VENDOR_SWD_SM 0
#define APP_VENDOR_SWD_PIN_OFFSET 2
#define APP_VENDOR_SWD_PIN_SWCLK APP_VENDOR_SWD_PIN_OFFSET + 0 // 2
#define APP_VENDOR_SWD_PIN_SWDIO APP_VENDOR_SWD_PIN_OFFSET + 1 // 3
#define APP_VENDOR_SWD_PIN_RESET 28

// misc
#if false
#define picoprobe_info(format,args...) printf(format, ## args)
#else
#define picoprobe_info(format,...) ((void)0)
#endif

#if false
#define picoprobe_debug(format,args...) printf(format, ## args)
#else
#define picoprobe_debug(format,...) ((void)0)
#endif

#if false
#define picoprobe_dump(format,args...) printf(format, ## args)
#else
#define picoprobe_dump(format,...) ((void)0)
#endif

#if false
#define TURBO_200MHZ 1
#endif

#endif
