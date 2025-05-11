/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/common.h>


#include "usb_cdc.h"
#include "cli.h"

void rx_callback(char *buf, uint16_t len);

void rx_callback(char *buf, uint16_t len) {
    if (len && buf[0] == 'h') {
        gpio_toggle(GPIOC, GPIO13);
    }

    for (uint16_t i = 0; i < len; i++) {
        cli_add_input(buf[i]);
    }
}

int main(void)
{
	int64_t i;

	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_USB);

	gpio_set(GPIOC, GPIO13);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_set(GPIOC, GPIO13);
    
    usb_cdc_add_rx_cb(rx_callback);
    usb_cdc_init();

	while (!usb_cdc_ready()) {
		__asm__("nop");
        usb_cdc_poll();
    }
	gpio_clear(GPIOC, GPIO13);

    while (usb_cdc_write("hello World7\r\n", 14) == 0) {
        usb_cdc_poll();
    };

    // cli_init();



	while (1) {
        // cli_update();
        usb_cdc_poll();
        
        if (i > 800000) {
            gpio_toggle(GPIOC, GPIO13);
            // usb_cdc_write("hello World8\r\n", 14);
            // usb_cdc_write(buf, 12);
            i = 0;
        }
        i++;
    }
}