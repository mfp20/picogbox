
#include "picogbox.h"
#include "log.h"
#include "tusb.h"
#include "bin_cdc_microshell.h"

#include <pico.h>
#include <hardware/timer.h>
#include <hardware/rtc.h>

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    32
#define BUF_OUT_SIZE   32
#define PATH_MAX_SIZE  32
static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// microshell main object
static struct ush_object ush;
// microshell non-blocking read interface
static int ush_read(struct ush_object *self, char *ch)
{
    if (tud_cdc_n_connected(APP_CDC_MICROSHELL_INTF)) {
        if (tud_cdc_n_available(APP_CDC_MICROSHELL_INTF)) {
            return (int)tud_cdc_n_read(APP_CDC_MICROSHELL_INTF, ch, 1);
        }
    }
    return 0;
}
// microshell non-blocking write interface
static int ush_write(struct ush_object *self, char ch)
{
    //return (Serial.write(ch) == 1);
    if (tud_cdc_n_connected(APP_CDC_MICROSHELL_INTF)) {
        if (tud_cdc_n_write_available(APP_CDC_MICROSHELL_INTF) > 0) {
            int res = tud_cdc_n_write_char(APP_CDC_MICROSHELL_INTF, ch);
            tud_cdc_n_write_flush(APP_CDC_MICROSHELL_INTF);
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

// microshell init
ush_object_ptr_t app_cdc_microshell_init(void) {
    LOG_INF("uShell init");
    ush_init(&ush, &ush_desc);
    ush_node_mount(&ush, "/", &ush_node_root, ush_files_root, sizeof(ush_files_root) / sizeof(ush_files_root[0]));
    ush_node_mount(&ush, "/sys", &ush_node_sys, ush_files_sys, sizeof(ush_files_sys) / sizeof(ush_files_sys[0]));
    ush_node_mount(&ush, "/dev", &ush_node_dev, ush_files_dev, sizeof(ush_files_dev) / sizeof(ush_files_dev[0]));
    ush_node_mount(&ush, "/bin", &ush_node_bin, ush_files_bin, sizeof(ush_files_bin) / sizeof(ush_files_bin[0]));
    return &ush;
}
