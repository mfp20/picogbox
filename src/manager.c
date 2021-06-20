#include "log.h"
#include "manager.h"

#include <pico.h>
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>

#include <stdarg.h>

// allocations register
alloc_meta_t alloc[64];
uint8 alloc_no = 0;

static int8 alloc_search_by_user(uint8 type, char *name) {
    for (int i=0;i<alloc_no;i++) {
        if ((alloc[i].type==type)&&(alloc[i].user->name==name)) return i;
    }
    return -1;
}
static void alloc_compact(uint8 id) {
    for (int i=id;i<alloc_no;i++) {
        alloc[i] = alloc[i+1];
    }
}
static void alloc_remove(uint8 type, char *name) {
    int8 alloc_id = alloc_search_by_user(type, name);
    alloc_compact(alloc_id);
    alloc_no--;
}


// PIN
typedef struct pin_def_s {
    uint8 id;
    char *name;
    bool digital;
    bool analog;
    bool input;
    bool output;
    uint8 func[9];
    bool onboard;
    bool wired;
} pin_def_t;
static const pin_def_t PIN_DEF[PIN_NO] = {
    {
        .id = 0,
        .name = "PIN0",
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
        .name = "PIN1",
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
        .name = "PIN2",
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
        .name = "PIN3",
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
        .name = "PIN4",
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
        .name = "PIN5",
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
        .name = "PIN6",
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
        .name = "PIN7",
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
        .name = "PIN8",
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
        .name = "PIN9",
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
        .name = "PIN10",
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
        .name = "PIN11",
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
        .name = "PIN12",
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
        .name = "PIN13",
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
        .name = "PIN14",
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
        .name = "PIN15",
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
        .name = "PIN16",
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
        .name = "PIN17",
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
        .name = "PIN18",
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
        .name = "PIN19",
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
        .name = "PIN20",
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
        .name = "PIN21",
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
        .name = "PIN22",
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
        .name = "PIN23",
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
        .name = "PIN24",
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
        .name = "PIN25",
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
        .name = "PIN26",
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
        .name = "PIN27",
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
        .name = "PIN28",
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
        .name = "PIN29",
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
int pin_alloc(uint8 id, consumer_meta_t const* user) {
    if (id>PIN_NO) return -1;
    if (pin_avail[id]) {
        pin_avail[id] = false;
        alloc[alloc_no].type = RSRC_PIN;
        alloc[alloc_no].rsrc.name = PIN_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("PIN %d allocated", id);
        return id;
    }
    LOG_INF("PIN %d already in use", id);
    return -1;
}
void pin_free(uint8 id) {
    pin_avail[id] = true;
    alloc_remove(RSRC_PIN, PIN_DEF[id].name);
    LOG_NOT("PIN %d free'ed", id);
}
int pin_get(consumer_meta_t const* user, bool digital, bool analog, bool input, bool output, uint8 func) {
    LOG_INF("Searching for pin...");
    for (uint8 i=0;i<PIN_NO;i++) {
        if (pin_avail[i]) {
            if ( (digital==PIN_DEF[i].digital) &&
                (analog==PIN_DEF[i].analog) &&
                (input==PIN_DEF[i].input) &&
                (output==PIN_DEF[i].output)
                // TODO add func
            ) {
                return pin_alloc(i, user);
            }
        }
    }
    return -1;
}


// USB_CDC
typedef struct usb_cdc_def_s {
    uint8 id;
    char *name;
} usb_cdc_def_t;
static const usb_cdc_def_t USB_CDC_DEF[6] = {
    {
        .id = 0,
        .name = "CDC0"
    },
    {
        .id = 1,
        .name = "CDC1"
    },
    {
        .id = 2,
        .name = "CDC2"
    },
    {
        .id = 3,
        .name = "CDC3"
    },
    {
        .id = 4,
        .name = "CDC4"
    },
    {
        .id = 5,
        .name = "CDC5"
    }
};
bool usb_cdc_avail[USB_CDC_NO];
int usb_cdc_alloc(uint8 id, consumer_meta_t const* user) {
    if (id>USB_CDC_NO) {
        LOG_INF("USB CDC %d doesn't exist", id);
        return -1;
    }
    if (usb_cdc_avail[id]) {
        usb_cdc_avail[id] = false;
        alloc[alloc_no].type = RSRC_CDC;
        alloc[alloc_no].rsrc.name = USB_CDC_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("USB CDC %d allocated", id);
        return id;
    }
    LOG_INF("USB CDC %d already in use", id);
    return -1;
}
void usb_cdc_free(uint8 id) {
    usb_cdc_avail[id] = true;
    alloc_remove(RSRC_CDC, USB_CDC_DEF[id].name);
    LOG_NOT("USB CDC %d free'ed", id);
}
int usb_cdc_get(consumer_meta_t const* user) {
    LOG_INF("Searching USB CDC...");
    uint8 i = USB_CDC_NO;
    int res = 0;
    while (i>0) {
        if (usb_cdc_avail[i]) {
            res = usb_cdc_alloc(i, user);
            if (res>=0) return res;
            i--;
        }
    }
    return -1;
}


// USB_VENDOR
typedef struct usb_vendor_def_s {
    uint8 id;
    char *name;
} usb_vendor_def_t;
static const usb_vendor_def_t USB_VENDOR_DEF[1] = {
    {
        .id = 0,
        .name = "VENDOR0"
    }
};
bool usb_vendor_avail[USB_VENDOR_NO];
int usb_vendor_alloc(uint8 id, consumer_meta_t const* user) {
    if (usb_vendor_avail[id]) {
        usb_vendor_avail[id] = false;
        alloc[alloc_no].type = RSRC_VEN;
        alloc[alloc_no].rsrc.name = USB_VENDOR_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("USB VENDOR %d allocated", id);
        return id;
    }
    return -1;
}
void usb_vendor_free(uint8 id) {
    usb_vendor_avail[id] = true;
    alloc_remove(RSRC_VEN, USB_VENDOR_DEF[id].name);
    LOG_NOT("USB VENDOR %d free'ed", id);
}
int usb_vendor_get(consumer_meta_t const* user) {
    LOG_INF("Searching USB VENDOR...");
    uint8 i = USB_VENDOR_NO;
    int res = 0;
    while (i>=0) {
        res = usb_vendor_alloc(i, user);
        if (res>=0) return res;
        i--;
    }
    return -1;
}


// UART
typedef struct uart_def_s {
    uart_inst_t *id;
    char *name;
} uart_def_t;
static const uart_def_t UART_DEF[2] = {
    {
        .id = uart0,
        .name = "UART0"
    },
    {
        .id = uart1,
        .name = "UART1"
    }
};
bool uart_avail[UART_NO];
int UARTDEV_ALLOC(uint8 id, consumer_meta_t const* user) {
    if (uart_avail[id]) {
        uart_avail[id] = false;
        alloc[alloc_no].type = RSRC_UART;
        alloc[alloc_no].rsrc.name = UART_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("UART DEV%d allocated", id);
        return id;
    }
    LOG_INF("UART DEV%d already in use", id);
    return -1;
}
void UARTDEV_FREE(uint8 id) {
    uart_avail[id] = true;
    alloc_remove(RSRC_UART, UART_DEF[id].name);
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
uart_alloc_t uart[UART_NO];
int uart_alloc(uint8 id, consumer_meta_t const* user, uint8 tx, uint8 rx) {
    if (UARTDEV_ALLOC(id, user)>=0) {
        if (pin_alloc(tx, user)>=0) {
            if (pin_alloc(rx, user)>=0) {
                uart[id].tx = tx;
                uart[id].rx = rx;
                //
                gpio_set_function(tx, GPIO_FUNC_UART);
                gpio_set_function(rx, GPIO_FUNC_UART);
                if (id) {
                    uart_init(uart1, UART_DEFAULT_BAUDRATE);
                } else {
                    uart_init(uart0, UART_DEFAULT_BAUDRATE);
                }
                //
                LOG_NOT("UART %d allocated (tx=%d, rx=%d)", id, tx, rx);
                return id;
            }
            pin_free(tx);
        }
        UARTDEV_FREE(id);
    }
    LOG_INF("UART %d already in use", id);
    return -1;
}
void uart_free(uint8 id) {
    pin_free(uart[id].tx);
    pin_free(uart[id].rx);
    UARTDEV_FREE(id);
    LOG_NOT("UART %d (tx=%d, rx=%d) free'ed", id, uart[id].tx, uart[id].rx);
}
int uart_get(consumer_meta_t const* user) {
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
                res = uart_alloc(uartdev, user, uart_tx[uartdev][i], uart_rx[uartdev][i]);
                if (res>=0) return res;
                i++;
            }
        }
    }
    return -1;
}


// I2C
typedef struct i2c_def_s {
    i2c_inst_t *id;
    char *name;
} i2c_def_t;
static const i2c_def_t I2C_DEF[2] = {
    {
        .id = i2c0,
        .name = "I2C0"
    },
    {
        .id = i2c1,
        .name = "I2C1"
    }
};
bool i2c_avail[I2C_NO];
int I2CDEV_ALLOC(uint8 id, consumer_meta_t const* user) {
    if (i2c_avail[id]) {
        i2c_avail[id] = false;
        alloc[alloc_no].type = RSRC_I2C;
        alloc[alloc_no].rsrc.name = I2C_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("I2C DEV%d allocated", id);
        return id;
    }
    LOG_INF("I2C DEV%d already in use", id);
    return -1;
}
void I2CDEV_FREE(uint8 id) {
    i2c_avail[id] = true;
    alloc_remove(RSRC_I2C, I2C_DEF[id].name);
    LOG_NOT("I2C DEV %d free'ed", id);
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
static i2c_alloc_t i2c[I2C_NO];
int i2c_alloc(uint8 id, consumer_meta_t const* user, uint8 sda, uint8 scl) {
    if (I2CDEV_ALLOC(id, user)>=0) {
        if (pin_alloc(sda, user)>=0) {
            if (pin_alloc(scl, user)>=0) {
                i2c[id].sda = sda;
                i2c[id].scl = scl;
                LOG_NOT("I2C %d allocated (sda=%d, scl=%d)", id, sda, scl);
                return id;
            }
            pin_free(sda);
        }
        I2CDEV_FREE(id);
    }
    LOG_INF("I2C %d already in use", id);
    return -1;
}
void i2c_free(uint8 id) {
    pin_free(i2c[id].sda);
    pin_free(i2c[id].scl);
    I2CDEV_FREE(id);
    LOG_NOT("I2C %d (sda=%d, scl=%d) free'ed", id, i2c[id].sda, i2c[id].scl);
}
int i2c_get(consumer_meta_t const* user) {
    LOG_INF("Searching I2C...");
    for (uint8 i2cdev=0;i2cdev<I2C_NO;i2cdev++) {
        if (i2c_avail[i2cdev]) {
            uint8 i = 0;
            int res = 0;
            while (i<6) {
                res = i2c_alloc(i2cdev, user, i2c_sda[i2cdev][i], i2c_scl[i2cdev][i]);
                if (res>=0) return res;
                i++;
            }
        }
    }
    return -1;
}


// SPI
typedef struct spi_def_s {
    spi_inst_t *id;
    char *name;
} spi_def_t;
static const spi_def_t SPI_DEF[2] = {
    {
        .id = spi0,
        .name = "SPI0"
    },
    {
        .id = spi1,
        .name = "SPI1"
    }
};
bool spi_avail[SPI_NO];
int SPIDEV_ALLOC(uint8 id, consumer_meta_t const* user) {
    if (spi_avail[id]) {
        spi_avail[id] = false;
        alloc[alloc_no].type = RSRC_SPI;
        alloc[alloc_no].rsrc.name = SPI_DEF[id].name;
        alloc[alloc_no].user = user;
        alloc_no++;
        LOG_NOT("SPI DEV%d allocated", id);
        return id;
    }
    LOG_INF("SPI DEV%d already in use", id);
    return -1;
}
void SPIDEV_FREE(uint8 id) {
    spi_avail[id] = true;
    alloc_remove(RSRC_SPI, SPI_DEF[id].name);
    LOG_NOT("UART DEV %d free'ed", id);
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
static spi_alloc_t spi[SPI_NO];
int spi_alloc(uint8 id, consumer_meta_t const* user, uint8 miso, uint8 mosi, uint8 sck, uint8 cs_count, ...) {
    if (cs_count>8) return -1;

    if (SPIDEV_ALLOC(id, user)>=0) {
        if (pin_alloc(miso, user)>=0) {
            if (pin_alloc(mosi, user)>=0) {
                if (pin_alloc(sck, user)>=0) {
                    spi[id].cs_no = 0;
                    va_list cs_va;
                    va_start(cs_va, cs_count);
                    while (spi[id].cs_no<cs_count) {
                        int8 cs = va_arg(cs_va, int8);
                        if (pin_alloc(cs, user)>=0) {
                            spi[id].cs[spi[id].cs_no] = cs;
                            spi[id].cs_no++;
                        } else {
                            break;
                        }
                    }
                    va_end(cs_va);
                    if (spi[id].cs_no == cs_count) {
                        spi[id].miso = miso;
                        spi[id].mosi = mosi;
                        spi[id].sck = sck;
                        LOG_NOT("SPI %d allocated (miso=%d, mosi=%d, sck=%d, cs_count=%d)", id, miso, mosi, sck, cs_count);
                        return id;
                    } else {
                        while (spi[id].cs_no>0) {
                            pin_free(spi[id].cs[spi[id].cs_no]);
                            spi[id].cs_no--;
                        }
                        pin_free(miso);
                    }
                }
                pin_free(mosi);
            }
            pin_free(sck);
        }
        SPIDEV_FREE(id);
    }
    LOG_INF("SPI %d already in use", id);
    return -1;
}
void spi_free(uint8 id) {
    pin_free(spi[id].miso);
    pin_free(spi[id].mosi);
    pin_free(spi[id].sck);
    while (spi[id].cs_no>0) {
        pin_free(spi[id].cs[spi[id].cs_no]);
        spi[id].cs_no--;
    }
    SPIDEV_FREE(id);
    LOG_NOT("SPI %d (miso=%d, mosi=%d, sck=%d, cs_count=%d) free'ed", id, spi[id].miso, spi[id].mosi, spi[id].sck, spi[id].cs_no);
}
int spi_get(consumer_meta_t const* user, uint8 cs_count) {
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
                res = spi_alloc(spidev, user, spi_miso[spidev][i], spi_mosi[spidev][i], spi_sck[spidev][i], 0);
                if (res>=0) break;
                i++;
            }
            if (res<0) {
                SPIDEV_FREE(spidev);
                break;
            }
            // cs pin(s)
            spi[spidev].cs_no = 0;
            for (int i=0;i<3;i++) {
                if (i==2) {
                    if (spi_sck[spidev][i]==255) break;
                }
                int8 cs = spi_cs[spidev][i];
                if (pin_alloc(cs, user)>=0) {
                    spi[spidev].cs[spi[spidev].cs_no] = cs;
                    spi[spidev].cs_no++;
                }
            }
            if (spi[spidev].cs_no==cs_count) return res;
            //
            spi_free(spidev);
        }
    }
    return -1;
}

// global microshell object ptr
ush_object_ptr_t ush;

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

    // hardware misc
    serialid_init();
    led_init();
}

/*
void manager_select_default(void) {
    usb_cdc_get_bridge();
    usb_cdc_get_bridge();
    usb_cdc_get_app();
    usb_vendor_get();
    i2c_alloc(0, 20, 21);
    i2c_alloc(1, 18, 19);
    spi_alloc(0, 4, 3, 2, 2, 5, 17);
    spi_alloc(1, 12, 11, 10, 1, 13);
    // TODO 26 - ADC0
    // TODO 27 - ADC1
    // TODO 28 - ADC2
}
*/
