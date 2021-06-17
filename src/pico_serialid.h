#ifndef PICO_SERIALID_H_
#define PICO_SERIALID_H_

// Contains unique serial number string (NUL terminated) after call to init_usb_serial
extern char usb_serial[];

// Fills unique_serial with the flash unique id
extern void serialid_init(void);

#endif
