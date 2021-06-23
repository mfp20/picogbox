#ifndef BIN_CDC_MICROSHELL_H
#define BIN_CDC_MICROSHELL_H

#include <microshell.h>

#include <stdarg.h>

void bin_cdc_microshell_init(uint8 cdc);
void ush_printf(const char* fmt, ...);

#endif
