// USB CDC0 (user console)
// USB CDC1 (bridge1: defaults to UART0)
// USB CDC2 (bridge2: defaults to UART1)
// USB CDC3 (bridge3: defaults to I2C master)
// USB CDC4 (bridge4: defaults to SPI master)
// USB CDC5 (serial data application: defaults to SUMP)
// USB VENDOR (raw data application: defaults to picoprobe)
// UART0 for Picobox itself (ex: debug)
// UART1 for target device


#if TURBO_200MHZ
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#endif
#include <bsp/board.h>
#include <tusb.h>
#include <microshell.h>

#include "config.h"
#include "pico_led.h"
#include "pico_serialid.h"
#include "bin_cdc_microshell.h"
#include "bin_cdc_bridge_uart.h"
#include "bin_cdc_sump.h"
#include "bin_cdc_sigrock.h"
#include "bin_vendor_swd.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ush_object_ptr_t ush;

// base commands handlers
#include <inc/ush_internal.h>

static void ush_handler_exec_info(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    ush_write_pointer(ush, "ush_handler_exec_info\r\n", USH_STATE_RESET_PROMPT);
    ush_process_start(self, file);
}
static void ush_handler_process_info(struct ush_object *self, struct ush_file_descriptor const *file) {
    // TODO
    ush_write_pointer(ush, "ush_handler_process_info\r\n", USH_STATE_RESET_PROMPT);
}

static void ush_handler_exec_reboot(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
}
static void ush_handler_process_reboot(struct ush_object *self, struct ush_file_descriptor const *file) {
    // TODO
}

// base commands descriptor and node
static const struct ush_file_descriptor ush_cmds_base[] = {
    {
        .name = "info",
        .description = "print some general info about the device",
        .help = "usage: info",
        .exec = ush_handler_exec_info,
        .process = ush_handler_process_info,
    },
    {
        .name = "reboot",
        .description = "instantly reboots the device",
        .help = "usage: reboot",
        .exec = ush_handler_exec_reboot,
        .process = ush_handler_process_reboot,
    },
};
static struct ush_node_object ush_node_base;

int main(void) {
#if TURBO_200MHZ
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(200000, true);
#endif

    // hardware
    board_init();
    tusb_init();
    pico_serialid();
    led_init();

    // microshell
    ush = app_cdc_microshell_init();
    ush_commands_add(ush, &ush_node_base, ush_cmds_base, sizeof(ush_cmds_base) / sizeof(ush_cmds_base[0]));

    // default apps
    cdc_uart_init();
    //cdc_i2c_init();
    //cdc_spi_init();
    cdc_sump_init();
    cdc_sigrock_init();
    vendor_swd_init();

    // main loop start
    while (1) {
        // permanent tasks
        tud_task();
        ush_service(ush);
        led_task();

        // application tasks
        cdc_uart_task();
        cdc_sump_task();
        vendor_swd_task();
        //cdc_sigrock_task();
    }

    return 0;
}