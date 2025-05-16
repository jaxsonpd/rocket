/** 
 * @file dev.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-17
 * @brief Implementation of the dev (device) folder
 */


#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include "microshell.h"

#include "fs/fs.h"

// led file get data callback
size_t led_get_data_callback(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data)
{
    // read current led state
    bool state = gpio_get(GPIOC, GPIO13);
    // return pointer to data
    *data = (uint8_t*)((state) ? "1\r\n" : "0\r\n");
    // return data size
    return strlen((char*)(*data));
}

// led file set data callback
void led_set_data_callback(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t *data, size_t size)
{
    // data size validation
    if (size < 1)
        return;

    // arguments validation
    if (data[0] == '1') {
        // turn led on
        gpio_set(GPIOC, GPIO13);
    } else if (data[0] == '0') {
        // turn led off
        gpio_clear(GPIOC, GPIO13);
    }
}

// time file get data callback
size_t time_get_data_callback(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data)
{
    static char time_buf[16];
    // read current time
    long current_time = 11011;
    // convert
    snprintf(time_buf, sizeof(time_buf), "%ld\r\n", current_time);
    time_buf[sizeof(time_buf) - 1] = 0;
    // return pointer to data
    *data = (uint8_t*)time_buf;
    // return data size
    return strlen((char*)(*data));
}

// dev directory files descriptor
static const struct ush_file_descriptor dev_files[] = {
    {
        .name = "led",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = led_get_data_callback,      // optional data getter callback
        .set_data = led_set_data_callback,      // optional data setter callback
    },
    {
        .name = "time",
        .description = NULL,
        .help = NULL,
        .exec = NULL,
        .get_data = time_get_data_callback,
    },
};

static struct ush_node_object dev;

void fs_mnt_dev(struct ush_object *ush) {
    ush_node_mount(ush, "/dev", &dev, dev_files, sizeof(dev_files) / sizeof(dev_files[0]));
}
