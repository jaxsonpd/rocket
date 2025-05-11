/** 
 * @file cli.h
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-10
 * @brief Declarations for the cli interface
 */


#ifndef CLI_H
#define CLI_H


#include <stdint.h>
#include <stdbool.h>

/** 
 * @brief Initialise the cli interface
 * @param write_function a function to write to the consol
 * 
 * @return 0 if successful
 */
int cli_init(void);

/** 
 * @brief Update the cli interface (process the inputs etc.)
 * 
 */
void cli_update(void);

void cli_printf(const char *format, ...);

void cli_add_input(char c);

#endif // CLI_H