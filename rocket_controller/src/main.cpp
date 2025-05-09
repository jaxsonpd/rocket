#include "stm32f103x6.h"  // CMSIS device header for STM32F103

#include "microsh.h"

void delay_ms(uint32_t ms);

static void init_clock() {
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0);
    

    // Disable PLL before configuration
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);


    // Configure PLL:
    // PLLM = 9, Source = HSE (External 8MHz) so CLK = 72MHz
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC);
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    
    RCC->CFGR &= ~RCC_CFGR_PLLMULL;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;

    // Divide PLL by 1.5 to get 48MHz
    RCC->CFGR &= ~RCC_CFGR_USBPRE;

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));  // Wait until PLL is ready

    // Set PLL as system clock
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;  // Select PLL as system clock

    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2; // Use 2 flash wait states due to higher clock

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait for switch

    // // Step 6: Enable USB clock
    // RCC->APB1ENR |= RCC_APB1ENR_USBEN;
}

int main(void) {
    // static microsh_t* psh = &sh;
    // microsh_init(psh, microrl_print);

    // microsh_session_init(psh, credentials, 1, )

    init_clock();

    // Enable USB clock
    RCC->APB1ENR = RCC_APB1ENR_USBEN; 

    // Enable clock for GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // Configure PC13 as push-pull output (max speed 2 MHz)
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13); // Clear mode and config
    GPIOC->CRH |= GPIO_CRH_MODE13_1;                   // Output mode, max 2 MHz
    delay_ms(500);

    __enable_irq();

    while (1) {
        GPIOC->ODR ^= GPIO_ODR_ODR13;  // Toggle PC13
        delay_ms(500);
    }
}

void delay_ms(uint32_t ms) {
    // Simple delay assuming 72 MHz system clock
    // Approximate delay loop (~7200 iterations per ms)
    for (uint32_t i = 0; i < ms * 7200; i++) {
        __NOP();
    }
}
