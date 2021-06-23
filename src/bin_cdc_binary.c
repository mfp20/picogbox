#include "log.h"
#include "manager.h"
#include "bin_cdc_binary.h"

static void cdc_binary_task(void *data) {
}

static const consumer_meta_t user = {
    .name = "BIN CONSOLE",
    .task = cdc_binary_task
};

void bin_cdc_binary_init(uint8 cdc) {
    usb_cdc_alloc(cdc, &user);

    LOG_INF("Binary console on USB CDC%d", cdc);
}
