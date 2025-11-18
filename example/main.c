#include <stdio.h>
#include <stdint.h>

#include "system_stm32f4xx.h"
#include "stm32f4xx.h"
#include "i2c.h"

void SystemClock_Init(void) {
    RCC->CR |= RCC_CR_HSION;                                                // Enable HSI
    while (!(RCC->CR & RCC_CR_HSIRDY));                                     // Wait for HSI to be ready

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;                                      // Enable power interface clock
    PWR->CR &= ~PWR_CR_VOS;                                                 // Clear bits
    PWR->CR |= (2 << PWR_CR_VOS_Pos);                                       // Set the voltage regulator to Scale 2

    FLASH->ACR |= FLASH_ACR_DCEN;                                           // Enable data cache
    FLASH->ACR |= FLASH_ACR_ICEN;                                           // Enable instruction cache
    FLASH->ACR |= FLASH_ACR_PRFTEN;                                         // Enable prefetch buffer
    FLASH->ACR &= ~FLASH_ACR_LATENCY;                                       // Clear bits
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;                                    // Set the flash latency to 2 wait states

    RCC->CFGR &= ~RCC_CFGR_HPRE;                                            // Clear bits
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                                        // Don't divide AHB clock frequency
    RCC->CFGR &= ~RCC_CFGR_PPRE1;                                           // Clear bits
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;                                       // Divide APB1 clock frequency by 2
    RCC->CFGR &= ~RCC_CFGR_PPRE2;                                           // Don't divide APB2 clock frequency

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;                                    // Set HSI as PLL clock source
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;                                      // Clear bits
    RCC->PLLCFGR |= (16 << RCC_PLLCFGR_PLLM_Pos);                           // Set main PLL division factor to 16
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;                                      // Clear bits
    RCC->PLLCFGR |= (336 << RCC_PLLCFGR_PLLN_Pos);                          // Set main PLL multiplication factor to 336
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;                                      // Clear bits
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLP_Pos);                            // Set main PLL division factor to 4

    RCC->CR |= RCC_CR_PLLON;                                                // Enable PLL
    while(!(RCC->CR & RCC_CR_PLLRDY));                                      // Wait for PLL to be ready

    RCC->CFGR &= ~RCC_CFGR_SW;                                              // Clear bits
    RCC->CFGR |= RCC_CFGR_SW_PLL;                                           // Set PLL as system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);                 // Wait for PLL to be set as system clock

    SystemCoreClockUpdate();                                                // Update SystemCoreClock variable
}

I2C_Config I2C1_Config = {
    .instance = I2C1,
    .clockFrequency = 42000000,
    .mode = I2C_MODE_STANDARD,
    .enableAck = false
};

void I2C_Init(void) {
    I2C_EnableClock(I2C1);
    I2C_Configure(I2C1, &I2C1_Config);
}

int main(void) {
    SystemClock_Init();
    I2C_Init();
    
    while(1) {

    }
}
