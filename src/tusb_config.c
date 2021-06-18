/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "pico_serialid.h"

#include <pico/stdio/driver.h>


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110, // // USB Specification version 1.1
    .bDeviceClass       = 0x00, // Each interface specifies its own
    .bDeviceSubClass    = 0x00, // Each interface specifies its own
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0x2E8A, // Pi
#if PICO_NO_FLASH
    .idProduct          = 0xF005, // Picoprobe, quad bridge, APP - debug
#else
    .idProduct          = 0x0005, // Picoprobe, quad bridge, APP
#endif
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

enum
{
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

uint8_t const desc_configuration[] =
{
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
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "Raspberry Pi", // 1: Manufacturer
  "Picoprobe",    // 2: Product
  usb_serial,     // 3: Serial, uses flash unique ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
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

static void stdio_usb_cdc0_out_chars(const char *buf, int len) {
}
static void stdio_usb_cdc0_out_flush(void) {
}
static int stdio_usb_cdc0_in_chars(char *buf, int len) {
}
stdio_driver_t stdio_usb_cdc0_driver = { 
    .out_chars = stdio_usb_cdc0_out_chars,
    .out_flush = stdio_usb_cdc0_out_flush,
    .in_chars = stdio_usb_cdc0_in_chars,
};
