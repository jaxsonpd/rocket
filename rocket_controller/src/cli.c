/** 
 * @file cli.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-10
 * @brief Implementation for the command line interface
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "fs/fs.h"

#include <libopencm3/stm32/gpio.h>

#include "usb_cdc.h"

// #define USH_CONFIG_CUSTOM_FILE "ush_config_platform.h"
#define USH_CONFIG_PLATFORM_POSIX
#include "microshell.h"

#include "cli.h"

// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch) {
    (void)self;
    // should be implemented as a FIFO
    if (usb_cdc_avail() > 0) {
        *ch = usb_cdc_recv_byte();
        return 1;
    }
    return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch) {
    (void)self;
    // should be implemented as a FIFO
    usb_cdc_send_byte(ch);
    return 1;
}

// I/O interface descriptor
static const struct ush_io_interface ush_iface = {
    .read = ush_read,
    .write = ush_write,
};

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    32
#define BUF_OUT_SIZE   32
#define PATH_MAX_SIZE  32

static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// microshell instance handler
static struct ush_object ush;

// microshell descriptor
static const struct ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O interface pointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "HERMES01",                      // hostname (in prompt)
};

size_t info_get_data_callback(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data) {
    (void)self;
    static const char *info = "Use MicroShell and make fun!\r\n";

    // return pointer to data
    *data = (uint8_t*)info;
    // return data size
    return strlen(info);
}

// root directory files descriptor
static const struct ush_file_descriptor root_files[] = {
    {
        .name = "info.txt",                     // info.txt file name
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = info_get_data_callback,
    }
};


// root directory handler
static struct ush_node_object root;



int cli_init(void) {

    // initialize microshell instance
    ush_init(&ush, &ush_desc);

    // mount root directory (root must be first)
    ush_node_mount(&ush, "/", &root, root_files, sizeof(root_files) / sizeof(root_files[0]));

    fs_mnt_bin(&ush);
    
    fs_mnt_dev(&ush);

    fs_mnt_etc(&ush);

    fs_mnt_mnt(&ush);

    fs_mnt_log(&ush);
    return 0;
}

void cli_update(void) {
    ush_service(&ush);
}