/** 
 * @file usb.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-04
 * @brief Implementation for the USB CDC virtual com port interface
 */


#include <stdint.h>
#include <stdbool.h>

#include "stm32f103x6.h"

void USB_LP_IRQHandler() {
    GPIOC->ODR ^= GPIO_ODR_ODR13;  // Toggle PC13

    USB->ISTR = ~USB_ISTR_RESET;
}

void USB_init() {
    // Initialize the NVIC
    NVIC_SetPriority(USB_LP_IRQn, 8);
    NVIC_EnableIRQ(USB_LP_IRQn);

    // Enable USB macrocell
    USB->CNTR &= ~USB_CNTR_PDWN;

    // Wait 1μs until clock is stable
    SysTick->LOAD = 100;
    SysTick->VAL = 0;
    SysTick->CTRL = 1;
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
    }
    SysTick->CTRL = 0;

    // Enable all interrupts & the internal pullup to put 1.5K on D+ for FullSpeed USB
    USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM;
    // Disable 1.5k pull up here

    // Clear the USB Reset (D+ & D- low) to start enumeration
    USB->CNTR &= ~USB_CNTR_FRES;
}
