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

// Expand cli implementation here (must be in one file only)
#define EMBEDDED_CLI_IMPL

#include "embedded_cli.h"

#include "cli.h"

#define CLI_BUFFER_SIZE 166
#define CLI_RX_BUFFER_SIZE 16
#define CLI_CMD_BUFFER_SIZE 32
#define CLI_HISTORY_SIZE 32
#define CLI_BINDING_COUNT 3
#define CLI_PRINT_BUFFER_SIZE 512

// CLI buffer
static EmbeddedCli *cli;
static CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

// Bool to disable the interrupts, if CLI is not yet ready.
static bool cliIsReady = false;

void writeChar(EmbeddedCli *embeddedCli, char c) {
    (void)embeddedCli;

    usb_cdc_write(&c, 1);
}

void onClearCLI(EmbeddedCli *cli, char *args, void *context) {
    cli_printf("\33[2J");
}

void onLed(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);
    if (arg1 == NULL || arg2 == NULL) {
        cli_printf("usage: get-led [arg1] [arg2]");
        return;
    }
    // Make sure to check if 'args' != NULL, printf's '%s' formatting does not like a null pointer.
    cli_printf("LED with args: %s and %s", arg1, arg2);
}

void initCliBinding() {
    // Define bindings as local variables, so we don't waste static memory

    // Command binding for the clear command
    CliCommandBinding clear_binding = {
            .name = "clear",
            .help = "Clears the console",
            .tokenizeArgs = true,
            .context = NULL,
            .binding = onClearCLI
    };

    // Command binding for the led command
    CliCommandBinding led_binding = {
            .name = "get-led",
            .help = "Get led status",
            .tokenizeArgs = true,
            .context = NULL,
            .binding = onLed
    };

    embeddedCliAddBinding(cli, clear_binding);
    embeddedCliAddBinding(cli, led_binding);
}

int cli_init(void) {

    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBuffer = cliBuffer;
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_BINDING_COUNT;

    cli = embeddedCliNew(config);
    cli->writeChar = writeChar;

    // initCliBinding();

    // // Init the CLI with blank screen
    // onClearCLI(cli, NULL, NULL);

    // // CLI has now been initialized, set bool to true to enable interrupts.
    // cliIsReady = true;

    // return 0;
}

void cli_update(void) {
    usb_cdc_poll();
    embeddedCliProcess(cli);

}

// Function to encapsulate the 'embeddedCliPrint()' call with print formatting arguments (act like printf(), but keeps cursor at correct location).
// The 'embeddedCliPrint()' function does already add a linebreak ('\r\n') to the end of the print statement, so no need to add it yourself.
void cli_printf(const char *format, ...) {
    // Create a buffer to store the formatted string
    char buffer[CLI_PRINT_BUFFER_SIZE];

    // Format the string using snprintf
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Check if string fitted in buffer else print error to stderr
    if (length < 0) {
        fprintf(stderr, "Error formatting the string\r\n");
        return;
    }

    // Call embeddedCliPrint with the formatted string
    embeddedCliPrint(cli, buffer);
}

void cli_add_input(char c) {
    embeddedCliReceiveChar(cli, c);
}