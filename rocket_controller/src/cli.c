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


#include <libopencm3/stm32/gpio.h>

#include "usb_cdc.h"

// #define USH_CONFIG_CUSTOM_FILE "ush_config_platform.h"
#define USH_CONFIG_PLATFORM_POSIX
#include "microshell.h"

#include "cli.h"

// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch)
{
    // should be implemented as a FIFO
    if (usb_cdc_avail() > 0) {
        *ch = usb_cdc_recv_byte();
        return 1;
    }
    return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch)
{
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

size_t info_get_data_callback(struct ush_object *self, struct ush_file_descriptor const *file, uint8_t **data)
{
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

// toggle file execute callback
static void toggle_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    // simple toggle led, without any arguments validation
    gpio_toggle(GPIOC, GPIO13);
}

// reboot cmd file execute callback
static void reboot_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
#if defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#elif defined(ARDUINO_ARCH_AVR)
    void (*reset)(void) = 0;
    reset();
#else
    ush_print(self, "error: reboot not supported...");
#endif
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

// root directory handler
static struct ush_node_object root;

static struct ush_node_object bin;

static struct ush_node_object dev;

int cli_init(void) {

    // initialize microshell instance
    ush_init(&ush, &ush_desc);

    // mount root directory (root must be first)
    ush_node_mount(&ush, "/", &root, root_files, sizeof(root_files) / sizeof(root_files[0]));

    ush_node_mount(&ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
    
    ush_node_mount(&ush, "/dev", &dev, dev_files, sizeof(dev_files) / sizeof(dev_files[0]));
    return 0;
}

void cli_update(void) {
    ush_service(&ush);
}