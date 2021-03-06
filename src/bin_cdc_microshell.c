#include <RP2040.h> // NVIC_SystemReset()

#include "log.h"
#include "manager.h"
#include "bin_cdc_microshell.h"

#include <pico.h>
#include <hardware/timer.h>
#include <hardware/rtc.h>
#include <tusb.h>
#include <inc/ush.h>
#include <inc/ush_internal.h>

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    128
#define BUF_OUT_SIZE   128
#define PATH_MAX_SIZE  128
static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// CDC IO
static uint8 cdc_no = 0;
// microshell main object
static struct ush_object _ush;


// microshell non-blocking read interface
static int ush_read(struct ush_object *self, char *ch)
{
    if (tud_cdc_n_connected(cdc_no)) {
        if (tud_cdc_n_available(cdc_no)) {
            return (int)tud_cdc_n_read(cdc_no, ch, 1);
        }
    }
    return 0;
}
// microshell non-blocking write interface
static int ush_write(struct ush_object *self, char ch)
{
    if (tud_cdc_n_connected(cdc_no)) {
        if (tud_cdc_n_write_available(cdc_no) > 0) {
            int res = tud_cdc_n_write_char(cdc_no, ch);
            tud_cdc_n_write_flush(cdc_no);
            return res;
        }
    }
    return 0;
}
// microshell I/O interface
static const struct ush_io_interface ush_iface = {
    .read = ush_read,
    .write = ush_write,
};
// microshell descriptor
const struct ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O USB CDCpointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "picobox",                      // hostname (in prompt)
    // ush_file_execute_callback exec;                 /**< General command execute callback (optional) */
};

// base commands
static void ush_handler_exec_nothing(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    ush_print(self, "NOT IMPLEMENTED.\r\n");
}
static void ush_handler_exec_info(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    LOG_INF("ush_handler_exec_info, state=%d", self->state);
    // TODO
    /*
    alloc_meta_t *list[30];
    uint8 count = 0;
    count = alloc_list_by_type(RSRC_CDC, list);
    ush_printf("- USB CDCs (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_VEN, list);
    ush_printf("- USB VENDORs (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_DMA, list);
    ush_printf("- DMA channels (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_PIN, list);
    ush_printf("- Pins (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_PIO, list);
    ush_printf("- PIO channels (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_UART, list);
    ush_printf("- UART (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_I2C, list);
    ush_printf("- I2C (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    count = alloc_list_by_type(RSRC_SPI, list);
    ush_printf("- SPI (total: %d)\n\r", count);
    for (uint8 i=0;i<count;i++) {
        ush_printf("\t%s - %s\n\r", list[i]->rsrc.name, list[i]->user->name);
    }
    */
    ush_print(ush, "0:23456789");
    ush_print(ush, "01:3456789");
    ush_print(ush, "012:456789");
    ush_print(ush, "0123:56789");
    ush_print(ush, "01234:6789");
    ush_print(ush, "012345:789");
    ush_print(ush, "0123456:89");
    ush_print(ush, "01234567:9");
    ush_process_start(self, file);
}
void ush_handler_process_info(struct ush_object *self, const struct ush_file_descriptor *file) {
    LOG_INF("ush_handler_process_info, state=%d", self->state);
    (void)file;

    USH_ASSERT(self != NULL);
    USH_ASSERT(file != NULL);

    switch (self->state) {
        case USH_STATE_PROCESS_START:
            ush_print(self, "USH_STATE_PROCESS_START");
            self->state = USH_STATE_PROCESS_SERVICE;
        break;
        case USH_STATE_PROCESS_SERVICE:
            ush_print(self, "USH_STATE_PROCESS_SERVICE");
            self->state = USH_STATE_PROCESS_FINISH;
        break;
        case USH_STATE_PROCESS_FINISH:
            ush_print(self, "USH_STATE_PROCESS_FINISH");
            self->state = USH_STATE_RESET_PROMPT;
        break;
    }
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
        .process = ush_handler_process_info
    },
    {
        .name = "log_level",
        .description = "change log level",
        .help = "usage: log_level [ASSERT, ERROR, WARNING, INFO, DEBUG]\n\r",
        .exec = ush_handler_exec_nothing
    },
    {
        .name = "log_device",
        .description = "change device to send log output",
        .help = "usage: log_device [uart0, uart1, cdc1, cdc2, ..., cdcN]\n\r",
        .exec = ush_handler_exec_nothing
    },
    {
        .name = "reboot",
        .description = "instantly reboots the device",
        .help = "usage: reboot\n\r",
        .exec = ush_handler_exec_reboot
    },
};
static struct ush_node_object ush_node_base;


// node / (root)
static size_t ush_handler_get_root_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info = "This is the root folder.\n\rTo list folder contents you can use 'ls' command. Choose one sub folder by typing 'cd <name>'.\n\rAfter you 'cd' into the folder, a new README together with files and commands will be available using 'ls' again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_root[] = {
    {
        .name = "README",
        .description = "use \"cat README\" to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_root_readme,
    },
};
static struct ush_node_object ush_node_root;


// node /etc
static size_t ush_handler_get_etc_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info =   "In this folder there are all configuration values stored in flash.\n\rTo list folder contents you can use \"ls\" command. Choose one by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_etc[] = {
    {
        .name = "README",
        .description = "use \"cat README\" to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_etc_readme,
    },
};
static struct ush_node_object ush_node_etc;


// node /dev
static size_t ush_handler_get_dev_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info = "In this folder there are all available devices to be used with apps.\n\rTo list folder contents you can use \"ls\" command. Choose one by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_dev[] = {
    {
        .name = "README",
        .description = "use 'cat README' to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_dev_readme,
    },
};
static struct ush_node_object ush_node_dev;


// node /sys
static size_t ush_handler_get_sys_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info = "In this folder there are all available system peripherals, their registers and generic system information.\n\rTo list folder contents you can use 'ls' command. Choose one by typing 'cd <name>'.\n\rAfter you 'cd' into the folder, a new README together with files and commands will be available using 'ls' again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static void ush_handler_set_dev_stdout(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t *data, size_t size) {
    if (size < 1) return;

    printf("%.*s", size, data);
}
static size_t ush_handler_get_sys_time(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static char time_buf[32];
    // read current time
    datetime_t current_time;
    rtc_get_datetime(&current_time);
    // convert
    snprintf(time_buf, sizeof(time_buf), "%d-%d-%d %d-%d-%d\r\n", current_time.year, current_time.month, current_time.day, current_time.hour, current_time.min, current_time.sec);
    time_buf[sizeof(time_buf) - 1] = 0;
    // return pointer to data
    *data = (uint8_t*)time_buf;
    // return data size
    return strlen((char*)(*data));
}
static void ush_handler_set_sys_time(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t *data, size_t size) {
    ush_print(self, "NOT IMPLEMENTED\n\r");
}
static size_t ush_handler_get_sys_time_us_32(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static char time_buf[16];
    // read current time
    uint32_t current_time = time_us_32();
    // convert
    snprintf(time_buf, sizeof(time_buf), "%ld\r\n", current_time);
    time_buf[sizeof(time_buf) - 1] = 0;
    // return pointer to data
    *data = (uint8_t*)time_buf;
    // return data size
    return strlen((char*)(*data));
}
static size_t ush_handler_get_sys_time_us_64(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static char time_buf[16];
    // read current time
    uint64_t current_time = time_us_64();
    // convert
    snprintf(time_buf, sizeof(time_buf), "%lld\r\n", current_time);
    time_buf[sizeof(time_buf) - 1] = 0;
    // return pointer to data
    *data = (uint8_t*)time_buf;
    // return data size
    return strlen((char*)(*data));
}
static size_t ush_handler_get_sys_timestamp(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static char time_buf[16];
    // read current time
    uint64_t current_time = (time_us_64()/1000000);
    // convert
    snprintf(time_buf, sizeof(time_buf), "%lld\r\n", current_time);
    time_buf[sizeof(time_buf) - 1] = 0;
    // return pointer to data
    *data = (uint8_t*)time_buf;
    // return data size
    return strlen((char*)(*data));
}
static const struct ush_file_descriptor ush_files_sys[] = {
    {
        .name = "README",
        .description = "use 'cat README' to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_sys_readme,
    },
    {
        .name = "stdout",
        .description = "send data to stdout (default: UART0)",
        .help = "usage: echo <string> /sys/stdout",
        .set_data = ush_handler_set_dev_stdout,
    },
    {
        .name = "time",
        .description = "use 'cat time' to get current date and time or 'echo time' to set current date and time",
        .help = NULL,
        .get_data = ush_handler_get_sys_time,
        .set_data = ush_handler_set_sys_time,
    },
    {
        .name = "time_us_32",
        .description = "use 'cat time_us_32' to get the amount of microseconds since boot",
        .help = NULL,
        .get_data = ush_handler_get_sys_time_us_32,
    },
    {
        .name = "time_us_64",
        .description = "use 'cat time_us_64' to get the amount of microseconds since boot",
        .help = NULL,
        .get_data = ush_handler_get_sys_time_us_64,
    },
    {
        .name = "timestamp",
        .description = "use 'cat timestamp' to get the amount of seconds since boot",
        .help = NULL,
        .get_data = ush_handler_get_sys_timestamp,
    },
};
static struct ush_node_object ush_node_sys;


// node /bin
static size_t ush_handler_get_bin_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info =   "In this folder there are all available binaries (ie: applications).\n\rTo list folder contents you can use \"ls\" command. Choose one by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_bin[] = {
    {
        .name = "README",
        .description = "use \"cat README\" to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_bin_readme,
    },
};
static struct ush_node_object ush_node_bin;


static void microshell_task(void *data) {
    ush_service(&_ush);
}

// microshell init
static const consumer_meta_t user = {
    .name = "USHELL CONSOLE",
    .task = microshell_task
};

void bin_cdc_microshell_init(uint8 cdc) {
    if (usb_cdc_alloc(cdc, &user)>=0) {
        LOG_INF("uShell init on USB CDC %d", cdc);
    } else {
        LOG_WAR("Coudn't bind microshell to USB CDC %d", cdc);
    }
    cdc_no = cdc;
    // set global pointer
    ush = &_ush;
    //
    ush_init(ush, &ush_desc);
    ush_commands_add(ush, &ush_node_base, ush_cmds_base, sizeof(ush_cmds_base) / sizeof(ush_cmds_base[0]));
    ush_node_mount(ush, "/", &ush_node_root, ush_files_root, sizeof(ush_files_root) / sizeof(ush_files_root[0]));
    ush_node_mount(ush, "/etc", &ush_node_etc, ush_files_etc, sizeof(ush_files_etc) / sizeof(ush_files_etc[0]));
    ush_node_mount(ush, "/sys", &ush_node_sys, ush_files_sys, sizeof(ush_files_sys) / sizeof(ush_files_sys[0]));
    ush_node_mount(ush, "/dev", &ush_node_dev, ush_files_dev, sizeof(ush_files_dev) / sizeof(ush_files_dev[0]));
    ush_node_mount(ush, "/bin", &ush_node_bin, ush_files_bin, sizeof(ush_files_bin) / sizeof(ush_files_bin[0]));
}

void ush_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buff[128];
    vsnprintf(buff, 128, fmt, args);

    //ush_print(ush, buff);

    uint8 index = 0;
    uint8 c = buff[index];
    while ((c != '\0')&&(index<80)) {
        ush->desc->io->write(ush, c);
        c = buff[++index];
    }

    va_end(args);
}
