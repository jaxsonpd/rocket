/** 
 * @file dev.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Implementation of the etc (config) folder
 */


#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include "microshell.h"

#include "fs/fs.h"

// dev directory files descriptor
static const struct ush_file_descriptor etc_files[] = {
    {
        .name = "dev_info",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,      // optional data getter callback
        .set_data = NULL,      // optional data setter callback
    },
    {
        .name = "users",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,
    },
    {
        .name = "rocket_cfg",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,
    },
};

static struct ush_node_object etc;

void fs_mnt_etc(struct ush_object *ush) {
    ush_node_mount(ush, "/etc", &etc, etc_files, sizeof(etc_files) / sizeof(etc_files[0]));
}