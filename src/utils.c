
#include "picogbox.h"

#include <hardware/timer.h>

uint32 monotonic32(void) {
    return time_us_32();
}

uint64 monotonic64(void) {
    return time_us_64();
}

