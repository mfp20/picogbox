// USB CDC0 (user console)
// USB CDC1 (bridge1: defaults to UART0)
// USB CDC2 (bridge2: defaults to UART1)
// USB CDC3 (bridge3: defaults to I2C master)
// USB CDC4 (bridge4: defaults to SPI master)
// USB CDC5 (serial data application: defaults to SUMP)
// USB VENDOR (raw data application: defaults to picoprobe)
// UART0 is stdio, available for target device on request from console
// UART1 available for target device

#if TURBO_200MHZ
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#endif
#include <bsp/board.h>
#include <tusb.h>
#include <pico/stdio_uart.h>    // stdio_uart_init_full()
#include <RP2040.h>             // NVIC_SystemReset()

#include "picogbox.h"
#include "log.h"
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

// global microshell object ptr
ush_object_ptr_t ush;

static void ush_handler_exec_nothing(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_process_nothing(struct ush_object *self, struct ush_file_descriptor const *file) {
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_info(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // TODO
    ush_print(self, "ush_handler_exec_info\r\n");
}
static void ush_handler_process_info(struct ush_object *self, struct ush_file_descriptor const *file) {
    // TODO
    ush_print(self, "ush_handler_process_info\r\n");
}
static void ush_handler_exec_reboot(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    ush_print(self, "Rebooting...\r\n");
    NVIC_SystemReset();
}
// base commands descriptor and node
static const struct ush_file_descriptor ush_cmds_base[] = {
    {
        .name = "info",
        .description = "print some general info about the device",
        .help = "usage: info\n\r",
        .exec = ush_handler_exec_info,
        .process = ush_handler_process_info,
    },
    {
        .name = "log_level",
        .description = "change log level",
        .help = "usage: log_level [ASSERT, ERROR, WARNING, INFO, DEBUG]\n\r",
        .exec = ush_handler_exec_nothing,
        .process = ush_handler_process_nothing,
    },
    {
        .name = "log_device",
        .description = "change device to send log output",
        .help = "usage: log_device [uart0, uart1, cdc1, cdc2, ..., cdcN]\n\r",
        .exec = ush_handler_exec_nothing,
        .process = ush_handler_process_nothing,
    },
    {
        .name = "reboot",
        .description = "instantly reboots the device",
        .help = "usage: reboot\n\r",
        .exec = ush_handler_exec_reboot
    },
};
static struct ush_node_object ush_node_base;


int main(void) {
#if TURBO_200MHZ
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(200000, true);
#endif

    // early init stdio on uart0
    stdio_uart_init_full(uart0, 115200, 0, 1);
    //stdio_set_driver_enabled(&stdio_uart0_driver, true);

    // hardware
    board_init();
    tusb_init();

    // microshell
    ush = app_cdc_microshell_init();
    ush_commands_add(ush, &ush_node_base, ush_cmds_base, sizeof(ush_cmds_base) / sizeof(ush_cmds_base[0]));

    // misc
    serialid_init();
    led_init();

    // default apps
    cdc_uart_init();
    //cdc_i2c_init();
    //cdc_spi_init();
    cdc_sump_init();
    cdc_sigrock_init();
    vendor_swd_init();

    unsigned char data[10] = {32, 1, 24, 56, 102, 5, 78, 92, 200, 158};
    LOG_HEX(data, 10, "Test %s", data);

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