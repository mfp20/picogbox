#include "log.h"
#include "manager.h"
#include "pico_serialid.h"

#include <tusb.h>

#include <pico.h>
#include <pico/stdio/driver.h>


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// The default TinyUSB PID is -
//
//    01-- ---- --nv ihmc
//
// But we want to allow multiple CDC channels so we use -
//
//    r11w vunm ihdc ccba
//    |||| |||| ||||   |`-----  audio
//    |||| |||| ||||   `------  bth
//    |||| |||| |||`----------  cdc
//    |||| |||| ||`-----------  dfu
//    |||| |||| |`------------  hid
//    |||| |||| `-------------  midi
//    |||| |||`---------------  msc
//    |||| ||`----------------  net
//    |||| |`-----------------  usbtmc
//    |||| `------------------  vendor
//    |||`--------------------  web
//    ||`---------------------  ?
//    |`----------------------  ?
//    `-----------------------  ram-mode (no flash)

#define CFG_TUD_AUDIO   0
#define CFG_TUD_BTH     0
#define CFG_TUD_CDC     6
#define CFG_TUD_DFU     0
#define CFG_TUD_HID     0
#define CFG_TUD_MIDI    0
#define CFG_TUD_MSC     0
#define CFG_TUD_NET     0
#define CFG_TUD_USBTMC  0
#define CFG_TUD_VENDOR  1

#define PID_MAP(itf, n)         ( (CFG_TUD_##itf) << (n) )
#define USBD_VID                (0x2E8A)
#define USBD_PID                ( 0x6000              + \
                                  PID_MAP(AUDIO,   0) + \
                                  PID_MAP(BTH,     1) + \
                                  PID_MAP(CDC,     2) + \
                                  PID_MAP(DFU,     5) + \
                                  PID_MAP(HID,     6) + \
                                  PID_MAP(MIDI,    7) + \
                                  PID_MAP(MSC,     8) + \
                                  PID_MAP(NET,     9) + \
                                  PID_MAP(USBTMC, 10) + \
                                  PID_MAP(VENDOR, 11) + \
                                  PID_MAP(WEB,    12) + \
                                  ( 1 << 13         ) + \
                                  ( 1 << 14         ) + \
                                  ( 0 << 15         ) )

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // USB Specification version 2.0 (microframes each 0.125us)
    .bDeviceClass       = 0x00,   // Each interface specifies its own
    .bDeviceSubClass    = 0x00,   // Each interface specifies its own
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USBD_VID,
    .idProduct          = USBD_PID,
    .bcdDevice          = 0x0001, // Version 00.01
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}


//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
  ITFNUM_CDC0, ITFNUM_CDC0_DATA,
  ITFNUM_CDC1, ITFNUM_CDC1_DATA,
  ITFNUM_CDC2, ITFNUM_CDC2_DATA,
  ITFNUM_CDC3, ITFNUM_CDC3_DATA,
  ITFNUM_CDC4, ITFNUM_CDC4_DATA,
  ITFNUM_CDC5, ITFNUM_CDC5_DATA,
  ITFNUM_VENDOR,
  ITFNUM_TOTAL
};
#define EPNUM_CDC0_CMD   0x81
#define EPNUM_CDC0_DATA  0x82
#define EPNUM_CDC1_CMD   0x83
#define EPNUM_CDC1_DATA  0x84
#define EPNUM_CDC2_CMD   0x85
#define EPNUM_CDC2_DATA  0x86
#define EPNUM_CDC3_CMD   0x87
#define EPNUM_CDC3_DATA  0x88
#define EPNUM_CDC4_CMD   0x89
#define EPNUM_CDC4_DATA  0x8a
#define EPNUM_CDC5_CMD   0x8b
#define EPNUM_CDC5_DATA  0x8c
#define EPNUM_VENDOR0_OUT  0x0d
#define EPNUM_VENDOR0_IN 0x8e
#define CONFIG_TOTAL_LEN  ( (TUD_CONFIG_DESC_LEN) + \
                                                (TUD_HID_DESC_LEN     * CFG_TUD_HID   ) + \
                                                (TUD_CDC_DESC_LEN     * CFG_TUD_CDC   ) + \
                                                (TUD_MSC_DESC_LEN     * CFG_TUD_MSC   ) + \
                                                (TUD_MIDI_DESC_LEN     * CFG_TUD_MIDI ) + \
                                                (TUD_VENDOR_DESC_LEN  * CFG_TUD_VENDOR) )

uint8_t const desc_configuration[] = {
  // header
  TUD_CONFIG_DESCRIPTOR(1, ITFNUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
  // Interface 0 (console)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC0, 0, EPNUM_CDC0_CMD, 64, EPNUM_CDC0_DATA & 0x7F, EPNUM_CDC0_DATA, 64),
  // Interface 1 (bridge1)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC1, 0, EPNUM_CDC1_CMD, 64, EPNUM_CDC1_DATA & 0x7F, EPNUM_CDC1_DATA, 64),
  // Interface 2 (bridge2)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC2, 0, EPNUM_CDC2_CMD, 64, EPNUM_CDC2_DATA & 0x7F, EPNUM_CDC2_DATA, 64),
  // Interface 3 (bridge3)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC3, 0, EPNUM_CDC3_CMD, 64, EPNUM_CDC3_DATA & 0x7F, EPNUM_CDC3_DATA, 64),
  // Interface 4 (bridge4)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC4, 0, EPNUM_CDC4_CMD, 64, EPNUM_CDC4_DATA & 0x7F, EPNUM_CDC4_DATA, 64),
  // Interface 5 (serial data application)
  TUD_CDC_DESCRIPTOR(ITFNUM_CDC5, 0, EPNUM_CDC5_CMD, 64, EPNUM_CDC5_DATA & 0x7F, EPNUM_CDC5_DATA, 64),
  // Interface 6 (raw data application)
  TUD_VENDOR_DESCRIPTOR(ITFNUM_VENDOR, 0, EPNUM_VENDOR0_OUT, EPNUM_VENDOR0_IN, 64)
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations
  return desc_configuration;
}


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] = {
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "Anichang",       // 1: Manufacturer
  "Pico Good Box",  // 2: Product
  usb_serial,       // 3: Serial, uses flash unique ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;

  uint8_t chr_count;

  if ( index == 0) {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else {
    // Convert ASCII string into UTF-16

  if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

  return _desc_str;
}


//--------------------------------------------------------------------+
// callbacks
//--------------------------------------------------------------------+

// Invoked when received new data
//void tud_cdc_rx_cb(uint8_t itf) {}

// Invoked when received `wanted_char`
//void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {}

// Invoked when space becomes available in TX buffer
//void tud_cdc_tx_complete_cb(uint8_t itf) {}

// Invoked when line state DTR & RTS are changed via SET_CONTROL_LINE_STATE
//void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding) {
    LOG_INF("CDC %d: new baud rate %d", itf, line_coding->bit_rate);
    if (itf == APP_CDC_BRIDGE_UART0_INTF) {
        uart_set_baudrate(APP_CDC_BRIDGE_UART0_INST, line_coding->bit_rate);
    } else if (itf == APP_CDC_BRIDGE_UART1_INTF) {
        uart_set_baudrate(APP_CDC_BRIDGE_UART1_INST, line_coding->bit_rate);
    } else if (itf == APP_CDC_SUMP_INTF) {
    }
}

// Invoked when received send break
//void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms) {}


//--------------------------------------------------------------------+
// stdio drivers
//--------------------------------------------------------------------+

#define STDIO_USB_STDOUT_TIMEOUT_US 500000

static void stdio_usb_out_chars(uint8 itf, const char *buf, int length) {
    static uint64_t last_avail_time;
    if (tud_cdc_n_connected(itf)) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = tud_cdc_n_write_available(itf);
            if (n > avail) n = avail;
            if (n) {
                int n2 = tud_cdc_n_write(itf, buf + i, n);
                tud_task();
                tud_cdc_n_write_flush(itf);
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_n_write_flush(itf);
                if (!tud_cdc_n_connected(itf) ||
                    (!tud_cdc_n_write_available(itf) && time_us_64() > last_avail_time + STDIO_USB_STDOUT_TIMEOUT_US)) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
}
static int stdio_usb_in_chars(uint8 itf, char *buf, int length) {
    int rc = PICO_ERROR_NO_DATA;
    if (tud_cdc_n_connected(itf) && tud_cdc_n_available(itf)) {
        int count = tud_cdc_n_read(itf, buf, length);
        rc =  count ? count : PICO_ERROR_NO_DATA;
    }
    return rc;
}
static void stdio_usb_cdc0_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(0, buf, len);
}
static int stdio_usb_cdc0_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(0, buf, len);
}
static void stdio_usb_cdc1_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(1, buf, len);
}
static int stdio_usb_cdc1_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(1, buf, len);
}
static void stdio_usb_cdc2_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(2, buf, len);
}
static int stdio_usb_cdc2_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(2, buf, len);
}
static void stdio_usb_cdc3_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(3, buf, len);
}
static int stdio_usb_cdc3_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(3, buf, len);
}
static void stdio_usb_cdc4_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(4, buf, len);
}
static int stdio_usb_cdc4_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(4, buf, len);
}
static void stdio_usb_cdc5_out_chars(const char *buf, int len) {
  stdio_usb_out_chars(5, buf, len);
}
static int stdio_usb_cdc5_in_chars(char *buf, int len) {
  return stdio_usb_in_chars(5, buf, len);
}
static stdio_driver_t stdio_usb_cdc_driver[6] = {
  {
  .out_chars = stdio_usb_cdc0_out_chars,
  .in_chars = stdio_usb_cdc0_in_chars,
  .crlf_enabled = true
  },
  {
  .out_chars = stdio_usb_cdc1_out_chars,
  .in_chars = stdio_usb_cdc1_in_chars,
  .crlf_enabled = true
  },
  {
  .out_chars = stdio_usb_cdc2_out_chars,
  .in_chars = stdio_usb_cdc2_in_chars,
  .crlf_enabled = true
  },
  {
  .out_chars = stdio_usb_cdc3_out_chars,
  .in_chars = stdio_usb_cdc3_in_chars,
  .crlf_enabled = true
  },
  {
  .out_chars = stdio_usb_cdc4_out_chars,
  .in_chars = stdio_usb_cdc4_in_chars,
  .crlf_enabled = true
  },
  {
  .out_chars = stdio_usb_cdc5_out_chars,
  .in_chars = stdio_usb_cdc5_in_chars,
  .crlf_enabled = true
  }
};

void usb_init(consumer_meta_t const* user, uint8 stdio) {
  tusb_init();

  if (stdio&&(stdio<6)) {
    usb_cdc_stdio(stdio, user, true);
  } else if (stdio) {
    LOG_WAR("%d out of bounduaries, USB CDC number must be between 1 and 5", stdio);
    return;
  }
}

void usb_cdc_stdio(uint8 id, consumer_meta_t const* user, bool stdio) {
  if (usb_cdc_alloc(id, user)>0) {
    stdio_set_driver_enabled(&stdio_usb_cdc_driver[id], true);
    LOG_INF("Standard I/O on USB CDC %d", id);
  } else {
    LOG_WAR("Coudn't bind stdio to USB CDC %d", id);
    return;
  }
  LOG_INF("Standard I/O on CDC%d", id);
  LOG_EME("emergency log entry");
  LOG_ALE("alert log entry");
  LOG_CRI("critical log entry");
  LOG_ERR("error log entry");
  LOG_WAR("warning log entry");
  LOG_NOT("notice log entry");
  LOG_INF("info log entry");
  LOG_DEB("debug log entry");
  unsigned char data[20] = {32, 1, 24, 56, 102, 5, 78, 92, 200, 0, 32, 1, 24, 56, 102, 5, 78, 92, 200, 0};
  LOG_HEX(data, 20, "hex %s", "log entry");
}
