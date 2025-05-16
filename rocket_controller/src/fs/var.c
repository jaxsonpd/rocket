/** 
 * @file dev.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Implementation of the var (variable/log) folder
 */


#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include "microshell.h"

#include "fs/fs.h"

// dev directory files descriptor
static const struct ush_file_descriptor var_files[] = {
    // {
    //     .name = "led",
    //     .description = NULL,
    //     .help = NULL,
    //     .exec = NULL,
    //     .get_data = led_get_data_callback,      // optional data getter callback
    //     .set_data = led_set_data_callback,      // optional data setter callback
    // },
    // {
    //     .name = "time",
    //     .description = NULL,
    //     .help = NULL,
    //     .exec = NULL,
    //     .get_data = time_get_data_callback,
    // },
};

static const struct ush_file_descriptor var_log_files[] = {
    {
        .name = "flight_1.csv",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,      // optional data getter callback
        .set_data = NULL,      // optional data setter callback
    },
    {
        .name = "syslog",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = NULL,
    },
};



static struct ush_node_object var;

static struct ush_node_object var_log;

void fs_mnt_log(struct ush_object *ush) {
    ush_node_mount(ush, "/var", &var, var_files, sizeof(var_files) / sizeof(var_files[0]));
    ush_node_mount(ush, "/var/log", &var_log, var_log_files, sizeof(var_log_files) / sizeof(var_log_files[0]));
}