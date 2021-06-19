#include "log.h"
#include "manager.h"

#include <stdarg.h>

// PIN
typedef struct pin_def_s {
    uint8 id;
    bool digital;
    bool analog;
    bool input;
    bool output;
    uint8 func[9];
    bool onboard;
    bool wired;
} pin_def_t;
const pin_def_t PIN_DEF[PIN_NO] = {
    {
        .id = 0,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true, 
        .func[0] = PIN_CAP_SPI0_RX,
        .func[1] = PIN_CAP_UART0_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM0A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 1,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_CS,
        .func[1] = PIN_CAP_UART0_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM0B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 2,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_SCK,
        .func[1] = PIN_CAP_UART0_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM1A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 3,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_TX,
        .func[1] = PIN_CAP_UART0_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM1B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 4,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_RX,
        .func[1] = PIN_CAP_UART1_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM2A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 5,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_CS,
        .func[1] = PIN_CAP_UART1_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM2B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 6,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_SCK,
        .func[1] = PIN_CAP_UART1_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM3A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO1,
        .func[6] = PIN_CAP_PIO0,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 7,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_TX,
        .func[1] = PIN_CAP_UART1_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM3B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 8,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_RX,
        .func[1] = PIN_CAP_UART1_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM4A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 9,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_CS,
        .func[1] = PIN_CAP_UART1_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM4B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 10,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_SCK,
        .func[1] = PIN_CAP_UART1_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM5A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 11,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_TX,
        .func[1] = PIN_CAP_UART1_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM5B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 12,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_RX,
        .func[1] = PIN_CAP_UART0_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM6A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 13,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_CS,
        .func[1] = PIN_CAP_UART0_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM6B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 14,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_SCK,
        .func[1] = PIN_CAP_UART0_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM7A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 15,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_TX,
        .func[1] = PIN_CAP_UART0_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM7B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 16,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_RX,
        .func[1] = PIN_CAP_UART0_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM0A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 17,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_CS,
        .func[1] = PIN_CAP_UART0_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM0B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 18,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_SCK,
        .func[1] = PIN_CAP_UART0_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM1A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 19,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_TX,
        .func[1] = PIN_CAP_UART0_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM1B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 20,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_RX,
        .func[1] = PIN_CAP_UART1_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM2A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_IN0,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    },
    {
        .id = 21,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_CS,
        .func[1] = PIN_CAP_UART1_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM2B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_OUT0,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 22,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_SCK,
        .func[1] = PIN_CAP_UART1_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM3A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_IN1,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 23,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI0_TX,
        .func[1] = PIN_CAP_UART1_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM3B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_OUT1,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = false
    },
    {
        .id = 24,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_RX,
        .func[1] = PIN_CAP_UART1_TX,
        .func[2] = PIN_CAP_I2C0_SDA,
        .func[3] = PIN_CAP_PWM4A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_OUT2,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = false
    },
    {
        .id = 25,
        .digital = true,
        .analog = false,
        .input = true,
        .output = true,
        .func[0] = PIN_CAP_SPI1_CS,
        .func[1] = PIN_CAP_UART1_RX,
        .func[2] = PIN_CAP_I2C0_SCL,
        .func[3] = PIN_CAP_PWM4B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_CLOCK_OUT3,
        .func[8] = PIN_CAP_USB_VBUS_DET,
        .wired = true,
        .onboard = true
    },
    {
        .id = 26,
        .digital = true,
        .analog = true, 
        .input =true, 
        .output =true, 
        .func[0] =PIN_CAP_SPI1_SCK,
        .func[1] = PIN_CAP_UART1_CTS,
        .func[2] = PIN_CAP_I2C1_SDA,
        .func[3] = PIN_CAP_PWM5A,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = false
    },
    {
        .id = 27,
        .digital = true,
        .analog = true, 
        .input = true, 
        .output = true, 
        .func[0] = PIN_CAP_SPI1_TX, 
        .func[1] = PIN_CAP_UART1_RTS,
        .func[2] = PIN_CAP_I2C1_SCL,
        .func[3] = PIN_CAP_PWM5B,
        .func[4] = PIN_CAP_SIO,
        .func[5] = PIN_CAP_PIO0,
        .func[6] = PIN_CAP_PIO1,
        .func[7] = PIN_CAP_NONE,
        .func[8] = PIN_CAP_USB_OVCUR_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 28,
        .digital = true,
        .analog = true, 
        .input =true, 
        .output =true, 
        .func[0] =PIN_CAP_SPI1_RX, 
        .func[1] =PIN_CAP_UART0_TX, 
        .func[2] =PIN_CAP_I2C0_SDA, 
        .func[3] =PIN_CAP_PWM6A, 
        .func[4] =PIN_CAP_SIO, 
        .func[5] =PIN_CAP_PIO0, 
        .func[6] =PIN_CAP_PIO1, 
        .func[7] =PIN_CAP_NONE, 
        .func[8] =PIN_CAP_USB_VBUS_DET,
        .wired = false,
        .onboard = true
    },
    {
        .id = 29,
        .digital = true,
        .analog = true, 
        .input =true, 
        .output =true, 
        .func[0] =PIN_CAP_SPI1_CS, 
        .func[1] =PIN_CAP_UART0_RX, 
        .func[2] =PIN_CAP_I2C0_SCL, 
        .func[3] =PIN_CAP_PWM6B, 
        .func[4] =PIN_CAP_SIO, 
        .func[5] =PIN_CAP_PIO0, 
        .func[6] =PIN_CAP_PIO1, 
        .func[7] =PIN_CAP_NONE, 
        .func[8] =PIN_CAP_USB_VBUS_EN,
        .wired = false,
        .onboard = true
    }
};
bool pin_avail[PIN_NO];
bool pin_wired[PIN_NO];
int PIN_ALLOC(uint8 id) {
    if (id>PIN_NO) return -1;
    if (pin_avail[id]) {
        pin_avail[id] = false;
        LOG_NOT("PIN %d allocated", id);
        return id;
    }
    LOG_INF("PIN %d already in use", id);
    return -1;
}
void PIN_FREE(uint8 id) {
    pin_avail[id] = true;
    LOG_NOT("PIN %d free'ed", id);
}
int PIN_GET(bool digital, bool analog, bool input, bool output, uint8 func) {
    LOG_INF("Searching for pin...");
    for (uint8 i=0;i<PIN_NO;i++) {
        if (pin_avail[i]) {
            if ( (digital==PIN_DEF[i].digital) &&
                (analog==PIN_DEF[i].analog) &&
                (input==PIN_DEF[i].input) &&
                (output==PIN_DEF[i].output)
                // TODO add func
            ) {
                return PIN_ALLOC(i);
            }
        }
    }
    return -1;
}


// USB_CDC
bool usb_cdc_avail[USB_CDC_NO];
int USB_CDC_ALLOC(uint8 id) {
    if (id>USB_CDC_NO) {
        LOG_INF("USB CDC %d doesn't exist", id);
        return -1;
    }
    if (usb_cdc_avail[id]) {
        usb_cdc_avail[id] = false;
        LOG_NOT("USB CDC %d allocated", id);
        return id;
    }
    LOG_INF("USB CDC %d already in use", id);
    return -1;
}
void USB_CDC_FREE(uint8 id) {
    usb_cdc_avail[id] = true;
    LOG_NOT("USB CDC %d free'ed", id);
}
int USB_CDC_GET(void) {
    LOG_INF("Searching USB CDC...");
    uint8 i = USB_CDC_NO;
    int res = 0;
    while (i>0) {
        if (usb_cdc_avail[i]) {
            res = USB_CDC_ALLOC(i);
            if (res>=0) return res;
            i--;
        }
    }
    return -1;
}
int USB_CDC_GET_BRIDGE(void) {
    LOG_INF("Searching USB CDC for bridge...");
    uint8 i = 1;
    while (i<5) {
        if (USB_CDC_ALLOC(i)>0) return i;
        i++;
    }
    return -1;
}
int USB_CDC_GET_APP(void) {
    LOG_INF("Searching USB CDC for app...");
    if (USB_CDC_ALLOC(5)>=0) {
        return 5;
    }
    return -1;
}


// USB_VENDOR
bool usb_vendor_avail[USB_VENDOR_NO];
int USB_VENDOR_ALLOC(uint8 id) {
    if (usb_vendor_avail[id]) {
        usb_vendor_avail[id] = false;
        LOG_NOT("USB VENDOR %d allocated", id);
        return id;
    }
    return -1;
}
void USB_VENDOR_FREE(uint8 id) {
    usb_vendor_avail[id] = true;
    LOG_NOT("USB VENDOR %d free'ed", id);
}
int USB_VENDOR_GET(void) {
    LOG_INF("Searching USB VENDOR...");
    uint8 i = USB_VENDOR_NO;
    int res = 0;
    while (i>0) {
        res = USB_VENDOR_ALLOC(i);
        if (res>=0) return res;
        i--;
    }
    return -1;
}


// UART
bool uart_avail[UART_NO];
int UARTDEV_ALLOC(uint8 id) {
    if (uart_avail[id]) {
        uart_avail[id] = false;
        LOG_NOT("UART DEV%d allocated", id);
        return id;
    }
    LOG_INF("UART DEV%d already in use", id);
    return -1;
}
void UARTDEV_FREE(uint8 id) {
    uart_avail[id] = true;
    LOG_NOT("UART DEV %d free'ed", id);
}
static const uint8 uart_tx[UART_NO][3] = {
    {
        0,
        12,
        16
    },
    {
        4,
        8,
        255
    }
};
static const uint8 uart_rx[UART_NO][3] = {
    {
        1,
        13,
        17
    },
    {
        5,
        9,
        255
    }
};
typedef struct uart_alloc_s {
    uint8 tx;
    uint8 rx;
} uart_alloc_t;
uart_alloc_t uart_alloc[UART_NO];
int UART_ALLOC(uint8 id, uint8 tx, uint8 rx) {
    if (UARTDEV_ALLOC(id)>=0) {
        if (PIN_ALLOC(tx)>=0) {
            if (PIN_ALLOC(rx)>=0) {
                uart_alloc[id].tx = tx;
                uart_alloc[id].rx = rx;
                LOG_NOT("UART %d allocated (tx=%d, rx=%d)", id, tx, rx);
                return id;
            }
            PIN_FREE(tx);
        }
        UARTDEV_FREE(id);
    }
    LOG_INF("UART %d already in use", id);
    return -1;
}
void UART_FREE(uint8 id) {
    PIN_FREE(uart_alloc[id].tx);
    PIN_FREE(uart_alloc[id].rx);
    UARTDEV_FREE(id);
    LOG_NOT("UART %d (tx=%d, rx=%d) free'ed", id, uart_alloc[id].tx, uart_alloc[id].rx);
}
int UART_GET(void) {
    LOG_INF("Searching UART...");
    for (uint8 uartdev=0;uartdev<I2C_NO;uartdev++) {
        if (uart_avail[uartdev]) {
            uint8 i = 0;
            int res = 0;
            while (i<3) {
                if (i==2) {
                    if (uart_tx[uartdev][i]==255) break;
                    if (uart_rx[uartdev][i]==255) break;
                }
                res = UART_ALLOC(uartdev, uart_tx[uartdev][i], uart_rx[uartdev][i]);
                if (res>=0) return res;
                i++;
            }
        }
    }
    return -1;
}


// I2C
bool i2c_avail[I2C_NO];
int I2CDEV_ALLOC(uint8 id) {
    if (i2c_avail[id]) {
        i2c_avail[id] = false;
        LOG_NOT("I2C DEV%d allocated", id);
        return id;
    }
    LOG_INF("I2C DEV%d already in use", id);
    return -1;
}
void I2CDEV_FREE(uint8 id) {
    i2c_avail[id] = true;
}
static const uint8 i2c_sda[I2C_NO][6] = {
    {
        0,
        4,
        8,
        12,
        16,
        20
    },
    {
        2,
        6,
        10,
        14,
        18,
        26
    }
};
static const uint8 i2c_scl[I2C_NO][6] = {
    {
        1,
        5,
        9,
        13,
        17,
        21
    },
    {
        3,
        7,
        11,
        15,
        19,
        27
    }
};
typedef struct i2c_alloc_s {
    uint8 sda;
    uint8 scl;
} i2c_alloc_t;
static i2c_alloc_t i2c_alloc[I2C_NO];
int I2C_ALLOC(uint8 id, uint8 sda, uint8 scl) {
    if (I2CDEV_ALLOC(id)>=0) {
        if (PIN_ALLOC(sda)>=0) {
            if (PIN_ALLOC(scl)>=0) {
                i2c_alloc[id].sda = sda;
                i2c_alloc[id].scl = scl;
                LOG_NOT("I2C %d allocated (sda=%d, scl=%d)", id, sda, scl);
                return id;
            }
            PIN_FREE(sda);
        }
        I2CDEV_FREE(id);
    }
    LOG_INF("I2C %d already in use", id);
    return -1;
}
void I2C_FREE(uint8 id) {
    PIN_FREE(i2c_alloc[id].sda);
    PIN_FREE(i2c_alloc[id].scl);
    I2CDEV_FREE(id);
    LOG_NOT("I2C %d (sda=%d, scl=%d) free'ed", id, i2c_alloc[id].sda, i2c_alloc[id].scl);
}
int I2C_GET(void) {
    LOG_INF("Searching I2C...");
    for (uint8 i2cdev=0;i2cdev<I2C_NO;i2cdev++) {
        if (i2c_avail[i2cdev]) {
            uint8 i = 0;
            int res = 0;
            while (i<6) {
                res = I2C_ALLOC(i2cdev, i2c_sda[i2cdev][i], i2c_scl[i2cdev][i]);
                if (res>=0) return res;
                i++;
            }
        }
    }
    return -1;
}


// SPI
bool spi_avail[SPI_NO];
int SPIDEV_ALLOC(uint8 id) {
    if (spi_avail[id]) {
        spi_avail[id] = false;
        LOG_NOT("SPI DEV%d allocated", id);
        return id;
    }
    LOG_INF("SPI DEV%d already in use", id);
    return -1;
}
void SPIDEV_FREE(uint8 id) {
    spi_avail[id] = true;
}
static const uint8 spi_miso[SPI_NO][3] = {
    {
        0,
        4,
        16
    },
    {
        8,
        12,
        255
    }
};
static const uint8 spi_mosi[SPI_NO][3] = {
    {
        3,
        7,
        19
    },
    {
        11,
        15,
        255
    }
};
static const uint8 spi_sck[SPI_NO][3] = {
    {
        2,
        6,
        18
    },
    {
        10,
        14,
        255
    }
};
static const uint8 spi_cs[SPI_NO][3] = {
    {
        1,
        5,
        17
    },
    {
        9,
        13,
        255
    }
};
typedef struct spi_alloc_s {
    uint8 miso;
    uint8 mosi;
    uint8 sck;
    uint8 cs[8];
    uint8 cs_no;
} spi_alloc_t;
static spi_alloc_t spi_alloc[SPI_NO];
int SPI_ALLOC(uint8 id, uint8 miso, uint8 mosi, uint8 sck, uint8 cs_count, ...) {
    if (cs_count>8) return -1;

    if (SPIDEV_ALLOC(id)>=0) {
        if (PIN_ALLOC(miso)>=0) {
            if (PIN_ALLOC(mosi)>=0) {
                if (PIN_ALLOC(sck)>=0) {
                    spi_alloc[id].cs_no = 0;
                    va_list cs_va;
                    va_start(cs_va, cs_count);
                    while (spi_alloc[id].cs_no<cs_count) {
                        int8 cs = va_arg(cs_va, int8);
                        if (PIN_ALLOC(cs)>=0) {
                            spi_alloc[id].cs[spi_alloc[id].cs_no] = cs;
                            spi_alloc[id].cs_no++;
                        } else {
                            break;
                        }
                    }
                    va_end(cs_va);
                    if (spi_alloc[id].cs_no == cs_count) {
                        spi_alloc[id].miso = miso;
                        spi_alloc[id].mosi = mosi;
                        spi_alloc[id].sck = sck;
                        LOG_NOT("SPI %d allocated (miso=%d, mosi=%d, sck=%d, cs_count=%d)", id, miso, mosi, sck, cs_count);
                        return id;
                    } else {
                        while (spi_alloc[id].cs_no>0) {
                            PIN_FREE(spi_alloc[id].cs[spi_alloc[id].cs_no]);
                            spi_alloc[id].cs_no--;
                        }
                        PIN_FREE(miso);
                    }
                }
                PIN_FREE(mosi);
            }
            PIN_FREE(sck);
        }
        SPIDEV_FREE(id);
    }
    LOG_INF("SPI %d already in use", id);
    return -1;
}
void SPI_FREE(uint8 id) {
    PIN_FREE(spi_alloc[id].miso);
    PIN_FREE(spi_alloc[id].mosi);
    PIN_FREE(spi_alloc[id].sck);
    while (spi_alloc[id].cs_no>0) {
        PIN_FREE(spi_alloc[id].cs[spi_alloc[id].cs_no]);
        spi_alloc[id].cs_no--;
    }
    SPIDEV_FREE(id);
    LOG_NOT("SPI %d (miso=%d, mosi=%d, sck=%d, cs_count=%d) free'ed", id, spi_alloc[id].miso, spi_alloc[id].mosi, spi_alloc[id].sck, spi_alloc[id].cs_no);
}
int SPI_GET(uint8 cs_count) {
    LOG_INF("Searching SPI...");
    for (uint8 spidev=0;spidev<SPI_NO;spidev++) {
        if (spi_avail[spidev]) {
            if ((cs_count>3)||((spidev==1)&&(cs_count>2))) break;
            // 
            uint8 i = 0;
            int res = 0;
            while (i<3) {
                if (i==2) {
                    if (spi_miso[spidev][i]==255) break;
                    if (spi_mosi[spidev][i]==255) break;
                    if (spi_sck[spidev][i]==255) break;
                }
                res = SPI_ALLOC(spidev, spi_miso[spidev][i], spi_mosi[spidev][i], spi_sck[spidev][i], 0);
                if (res>=0) break;
                i++;
            }
            if (res<0) {
                SPIDEV_FREE(spidev);
                break;
            }
            // cs pin(s)
            spi_alloc[spidev].cs_no = 0;
            for (int i=0;i<3;i++) {
                if (i==2) {
                    if (spi_sck[spidev][i]==255) break;
                }
                int8 cs = spi_cs[spidev][i];
                if (PIN_ALLOC(cs)>=0) {
                    spi_alloc[spidev].cs[spi_alloc[spidev].cs_no] = cs;
                    spi_alloc[spidev].cs_no++;
                }
            }
            if (spi_alloc[spidev].cs_no==cs_count) return res;
            //
            SPI_FREE(spidev);
        }
    }
    return -1;
}


//
void manager_init(void) {
    for (int i=0;i<PIN_NO;i++) {
        if ((!PIN_DEF[i].wired)&&(PIN_DEF[i].onboard)) {
            pin_avail[i] = true;
        } else {
            pin_avail[i] = false;
            if (PIN_DEF[i].wired)
                pin_wired[i] = true;
            else
                pin_wired[i] = false;
        }
    }
    for (int i=0;i<USB_CDC_NO;i++)
        usb_cdc_avail[i] = true;
    for (int i=0;i<USB_VENDOR_NO;i++)
        usb_vendor_avail[i] = true;
    for (int i=0;i<UART_NO;i++)
        uart_avail[i] = true;
    for (int i=0;i<I2C_NO;i++)
        i2c_avail[i] = true;
    for (int i=0;i<SPI_NO;i++)
        spi_avail[i] = true;
}

void manager_select_default(void) {
    USB_CDC_GET_BRIDGE();
    USB_CDC_GET_BRIDGE();
    USB_CDC_GET_APP();
    USB_VENDOR_GET();
    UART_ALLOC(0, 0, 1);
    UART_ALLOC(1, 8, 9);
    I2C_ALLOC(0, 20, 21);
    I2C_ALLOC(1, 18, 19);
    SPI_ALLOC(0, 4, 3, 2, 2, 5, 17);
    SPI_ALLOC(1, 12, 11, 10, 1, 13);
    // TODO 26 - ADC0
    // TODO 27 - ADC1
    // TODO 28 - ADC2
}
