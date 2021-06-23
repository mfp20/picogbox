#include "log.h"
#include <stdio.h>

uint8 log_level = LOG_LEVEL;

void printk_hex(unsigned char *in, unsigned int count, char *out) {
	unsigned int size = 0;
	char ascii[16] = {};
	int index = 0;
	while(size<count) {
		if (!(size%0x10)) {
			index += snprintf(out+index, count*5, "\n\t\t%08x: ", size);
		} else if (!(size%8)) {
			//	After per 8 bytes insert two space for split
			index += snprintf(out+index, count*5, " ");
		}

		// handle Ascii detail area, store current byte
		ascii[size%16] = ((in[size] >= '!') && (in[size] <= '~')) ? in[size] : '.';

		// print current byte
		index += snprintf(out+index, count*5, "%02x ", in[size]);
		size++;

		//
		if (!(size%16) || (size == count)) {
			//	Empty bytes
			unsigned char len = size%16;
			if (len) {
				len = 16 - len;
				while(len--) {
					index += snprintf(out+index, count*5, "   ");
					if (len==8) {
						index += snprintf(out+index, count*5, " ");
					}
				}
			}
			index += snprintf(out+index, count*5, "    %s", ascii);
			if (size == count)
				break;
		}
	}
}

void log_set_level(uint8 level) {
	if (log_level > LOG_LEVEL) level = LOG_LEVEL;
	log_level = level;
}
