#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
  #error CFG_TUSB_MCU must be defined
#endif
#define CFG_TUSB_RHPORT0_MODE     OPT_MODE_DEVICE
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_PICO
#endif
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif
#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//------------- CLASS -------------//
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
#define CFG_TUD_WEB     0

#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64

#define CFG_TUD_VENDOR_RX_BUFSIZE 8192
#define CFG_TUD_VENDOR_TX_BUFSIZE 8192

#include "picogbox.h"
#include "manager.h"

void usb_init(consumer_meta_t const* user, uint8 stdio);
void usb_cdc_stdio(uint8 id, consumer_meta_t const* user, bool stdio);

#ifdef __cplusplus
 }
#endif

#endif
