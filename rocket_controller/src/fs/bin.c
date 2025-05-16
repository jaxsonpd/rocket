/** 
 * @file bin.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Implementation of the bin file system
 */


#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include "microshell.h"

#include "fs/fs.h"

static struct ush_node_object bin;

// toggle file execute callback
static void toggle_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[]) {
    // simple toggle led, without any arguments validation
    gpio_toggle(GPIOC, GPIO13);
}

// set file execute callback
static void set_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    // arguments count validation
    if (argc != 2) {
        // return predefined error message
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    // arguments validation
    if (strcmp(argv[1], "1") == 0) {
        // turn led on
        gpio_set(GPIOC, GPIO13);
    } else if (strcmp(argv[1], "0") == 0) {
        // turn led off
        gpio_clear(GPIOC, GPIO13);
    } else {
        // return predefined error message
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }
}

// bin directory files descriptor
static const struct ush_file_descriptor bin_files[] = {
    {
        .name = "toggle",                       // toggle file name
        .description = "toggle led",            // optional file description
        .help = "usage: toggle\r\n",            // optional help manual
        .exec = toggle_exec_callback,           // optional execute callback
    },
    {
        .name = "set",                          // set file name
        .description = "set led",
        .help = "usage: set {0,1}\r\n",
        .exec = set_exec_callback
    },
};

void fs_mnt_bin(struct ush_object *ush) {
    ush_node_mount(ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
}
