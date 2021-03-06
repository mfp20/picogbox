// USB CDC0 (user text console)
// USB CDC1 (user binary console)
// USB CDC2 (defaults to local log)
// USB CDC3 (defaults to UART1 bridge)
// USB CDC4 (defaults to sigrock logic analyser)
// USB CDC5 (unused)
// USB VENDOR (raw data application: defaults to picoprobe)
// UART0 is stdio, available for target device on request from console
// UART1 available for target device

#include "log.h"
#include "manager.h"
#include "managed_uart.h"

#include "bin_cdc_microshell.h"
#include "bin_cdc_binary.h"
#include "bin_cdc_bridge_uart.h"
#include "bin_cdc_capture.h"
#include "bin_vendor_swd.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool loop_rp_cb(repeating_timer_t *rt) {
    LOG_DEB("%lu", monotonic32());
    return true;
}

static const consumer_meta_t user = {
    .name = "MAIN",
    .task = NULL
};

int main(void) {
    // set default power level
    power_set_mode(POWER_DEFAULT);
    // resources manager
    manager_init();
    // uart0 for stdio
    managed_uart_init(0, &user, true);
    // usb, stdio on CDC2
    usb_init(&user, 2);

    // default apps
    bin_cdc_microshell_init(0);
    bin_cdc_binary_init(1);
    bin_cdc_bridge_uart_init(1, 3);
    bin_cdc_capture_init(4);
    // 5 unused
    bin_vendor_swd_init(0);

    repeating_timer_t loop_rp;
    add_repeating_timer_ms (10000, loop_rp_cb, NULL, &loop_rp);

    for (uint8 i=0;i<task_no;i++) {
        char *name = "not found";
        for (int k=0;k<alloc_no;k++) {
            LOG_INF("\t0x%08x %s", (int)alloc[k].user->task, alloc[k].user->name);
            if (alloc[k].user->task == *task[i])
                name = (char *) alloc[k].user->name;
        }
        LOG_INF("0x%08x -> 0x%08x %s", (int)task[i], (int)*task[i], name);
    }

    // main loop start
    while (1) {
        // permanent tasks
        led_task();
        tud_task();

        // variable tasks
        for (uint8 i=0;i<task_no;i++) {
            if (*task[i]) {
                (*(*task[i]))(NULL);
            }
        }
        tight_loop_contents();
    }

    return 0;
}