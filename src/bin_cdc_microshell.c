
#include "config.h"
#include "tusb.h"
#include <microshell.h>

#include "types.h"

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    32
#define BUF_OUT_SIZE   32
#define PATH_MAX_SIZE  32
static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// microshell main object
static struct ush_object ush;

// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch)
{
    if (tud_cdc_n_connected(APP_CDC_MICROSHELL_INTF)) {
        if (tud_cdc_n_available(APP_CDC_MICROSHELL_INTF)) {
            return (int)tud_cdc_n_read(APP_CDC_MICROSHELL_INTF, ch, 1);
        }
    }
    return 0;
}
// non-blocking write interface
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
// I/O USB CDCdescriptor
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

//
static size_t ush_handler_get_root_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info =   "This is the root folder.\n\rTo list folder contents you can use \"ls\" command. Choose one sub folder by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

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

static size_t ush_handler_get_sys_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info =   "In this folder there are all available system peripherals, their registers and generic system information.\n\rTo list folder contents you can use \"ls\" command. Choose one by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_sys[] = {
    {
        .name = "README",
        .description = "use \"cat README\" to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_sys_readme,
    },
};
static struct ush_node_object ush_node_sys;

static size_t ush_handler_get_dev_readme(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    static const char *info =   "In this folder there are all available devices to be used with apps.\n\rTo list folder contents you can use \"ls\" command. Choose one by typing \"cd <name>\".\n\rAfter you \"cd\" into the folder, a new README together with files and commands will be available using \"ls\" again.\n\r";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}
static const struct ush_file_descriptor ush_files_dev[] = {
    {
        .name = "README",
        .description = "use \"cat README\" to get some help about this folder",
        .help = NULL,
        .get_data = ush_handler_get_dev_readme,
    },
};
static struct ush_node_object ush_node_dev;

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

ush_object_ptr_t app_cdc_microshell_init(void) {
    ush_init(&ush, &ush_desc);
    ush_node_mount(&ush, "/", &ush_node_root, ush_files_root, sizeof(ush_files_root) / sizeof(ush_files_root[0]));
    ush_node_mount(&ush, "/sys", &ush_node_sys, ush_files_sys, sizeof(ush_files_sys) / sizeof(ush_files_sys[0]));
    ush_node_mount(&ush, "/dev", &ush_node_dev, ush_files_dev, sizeof(ush_files_dev) / sizeof(ush_files_dev[0]));
    ush_node_mount(&ush, "/bin", &ush_node_bin, ush_files_bin, sizeof(ush_files_bin) / sizeof(ush_files_bin[0]));
    return &ush;
}
