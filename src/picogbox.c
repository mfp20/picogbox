// USB CDC0 (user text console)
// USB CDC1 (user binary console)
// USB CDC2 (defaults to UART0 bridge)
// USB CDC3 (defaults to UART1 bridge)
// USB CDC4 (defaults to SUMP logic analyser)
// USB CDC5 (defaults to local log)
// USB VENDOR (raw data application: defaults to picoprobe)
// UART0 is stdio, available for target device on request from console
// UART1 available for target device

#if TURBO_200MHZ
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#endif

#include "log.h"
#include "manager.h"
#include "managed_uart.h"

#include "bin_cdc_microshell.h"
#include "bin_cdc_bridge_uart.h"
//#include "bin_cdc_sump.h"
//#include "bin_cdc_sigrock.h"
//#include "bin_vendor_swd.h"

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

#if TURBO_200MHZ
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(200000, true);
#endif

    // resources manager
    manager_init();

    // uart0 for stdio
    managed_uart_init(0, &user, true);

    // usb, stdio on CDC5
    usb_init(&user, 5);

    // microshell on CDC0
    bin_cdc_microshell_init(0);

    // default apps
    //bin_cdc_binary_init(1);
    //bin_cdc_bridge_uart_init(0, 2);
    bin_cdc_bridge_uart_init(1, 3);
    //bin_app_cdc_sump_init(4);
    //bin_app_vendor_swd_init(0);

    repeating_timer_t loop_rp;
    add_repeating_timer_ms (10000, loop_rp_cb, NULL, &loop_rp);

    // main loop start
    while (1) {
        // permanent tasks
        led_task();
        tud_task();

        // application tasks
        //bin_cdc_bridge_uart0_task();
        //bin_cdc_bridge_uart1_task();
        //bin_cdc_sump_task();
        //bin_vendor_swd_task();
        for (uint8 i=0;i<alloc_no;i++) {
            if (alloc[i].user->task)
                alloc[i].user->task(NULL);
        }
    }

    return 0;
}