/** 
 * @file dev.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Implementation of the mnt (external device) folder
 */


#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include "microshell.h"

#include "fs/fs.h"

// dev directory files descriptor
static const struct ush_file_descriptor mnt_files[] = {
    {
        .name = "sdcard",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,      // optional data getter callback
        .set_data = NULL,      // optional data setter callback
    },
};

static struct ush_node_object mnt;

void fs_mnt_mnt(struct ush_object *ush) {
    ush_node_mount(ush, "/mnt", &mnt, mnt_files, sizeof(mnt_files) / sizeof(mnt_files[0]));
}