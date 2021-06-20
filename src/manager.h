#ifndef MANAGER_H_
#define MANAGER_H_

#include "picogbox.h"

#include <pico.h>
#include <pico/stdlib.h>

#include "pico_power.h"
#include "pico_led.h"
#include "pico_serialid.h"

enum pin_e {
    P0 = 0,
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7,
    P8,
    P9,
    P10,
    P11,
    P12,
    P13,
    P14,
    P15,
    P16,
    P17,
    P18,
    P19,
    P20,
    P21,
    P22,
    P23,
    P24,
    P25,
    P26,
    P27,
    P28,
    P29,
    PIN_NO,
    PIN_NONE = UINT8_MAX
};

enum pin_cap_e {
    PIN_CAP_SIO = 0,
    PIN_CAP_PIO0,
    PIN_CAP_PIO1,
    PIN_CAP_UART0_TX,
    PIN_CAP_UART0_RX,
    PIN_CAP_UART0_CTS,
    PIN_CAP_UART0_RTS,
    PIN_CAP_UART1_TX,
    PIN_CAP_UART1_RX,
    PIN_CAP_UART1_CTS,
    PIN_CAP_UART1_RTS,
    PIN_CAP_I2C0_SDA,
    PIN_CAP_I2C0_SCL,
    PIN_CAP_I2C1_SDA,
    PIN_CAP_I2C1_SCL,
    PIN_CAP_SPI0_RX,
    PIN_CAP_SPI0_TX,
    PIN_CAP_SPI0_SCK,
    PIN_CAP_SPI0_CS,
    PIN_CAP_SPI1_RX,
    PIN_CAP_SPI1_TX,
    PIN_CAP_SPI1_SCK,
    PIN_CAP_SPI1_CS,
    PIN_CAP_PWM0A,
    PIN_CAP_PWM0B,
    PIN_CAP_PWM1A,
    PIN_CAP_PWM1B,
    PIN_CAP_PWM2A,
    PIN_CAP_PWM2B,
    PIN_CAP_PWM3A,
    PIN_CAP_PWM3B,
    PIN_CAP_PWM4A,
    PIN_CAP_PWM4B,
    PIN_CAP_PWM5A,
    PIN_CAP_PWM5B,
    PIN_CAP_PWM6A,
    PIN_CAP_PWM6B,
    PIN_CAP_PWM7A,
    PIN_CAP_PWM7B,
    PIN_CAP_CLOCK_IN0,
    PIN_CAP_CLOCK_IN1,
    PIN_CAP_CLOCK_OUT0,
    PIN_CAP_CLOCK_OUT1,
    PIN_CAP_CLOCK_OUT2,
    PIN_CAP_CLOCK_OUT3,
    PIN_CAP_USB_OVCUR_DET,
    PIN_CAP_USB_VBUS_DET,
    PIN_CAP_USB_VBUS_EN,
    PIN_CAP_NONE = UINT8_MAX
};

enum usb_cdc_e {
    C0 = 0,
    C1,
    C2,
    C3,
    C4,
    C5,
    USB_CDC_NO
};

enum usb_vendor_e {
    V0 = 0,
    USB_VENDOR_NO
};

enum uart_e {
    U0 = 0,
    U1,
    UART_NO    
};

enum i2c_e {
    I0 = 0,
    I1,
    I2C_NO
};

enum spi_e {
    S0 = 0,
    S1,
    SPI_NO
};

enum resource_type_e {
    RSRC_PIN = 0,
    RSRC_CDC = 0,
    RSRC_VEN = 0,
    RSRC_UART = 0,
    RSRC_I2C = 0,
    RSRC_SPI = 0
};

typedef struct resource_meta_s {
    char const* name;
} resource_meta_t;

typedef struct consumer_meta_s {
    char const* name;
    task_t task;
} consumer_meta_t;

typedef struct alloc_meta_s {
    uint8 type;
    resource_meta_t rsrc;
    consumer_meta_t const* user;
} alloc_meta_t;


//
extern alloc_meta_t alloc[64];
extern uint8 alloc_no;
extern ush_object_ptr_t ush;

void manager_init(void);

int pin_alloc(uint8 id, consumer_meta_t const* user);
void pin_free(uint8 id);
int pin_get(consumer_meta_t const* user, bool digital, bool analog, bool input, bool output, uint8 func);

int usb_cdc_alloc(uint8 id, consumer_meta_t const* user);
void usb_cdc_free(uint8 id);
int usb_cdc_get(consumer_meta_t const* user);

int usb_vendor_alloc(uint8 id, consumer_meta_t const* user);
void usb_vendor_free(uint8 id);
int usb_vendor_get(consumer_meta_t const* user);

int uart_alloc(uint8 id, consumer_meta_t const* user, uint8 tx, uint8 rx);
void uart_free(uint8 id);
int uart_get(consumer_meta_t const* user);

int i2c_alloc(uint8 id, consumer_meta_t const* user, uint8 sda, uint8 scl);
void i2c_free(uint8 id);
int i2c_get(consumer_meta_t const* user);

int spi_alloc(uint8 id, consumer_meta_t const* user, uint8 miso, uint8 mosi, uint8 sck, uint8 cs_count, ...);
void spi_free(uint8 id);
int spi_get(consumer_meta_t const* user, uint8 cs_count);

#endif
