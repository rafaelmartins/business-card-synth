// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <assert.h>
#include <stm32g4xx.h>
#include "clock.h"


void
clock_init(void)
{
    // 4 flash wait cycles required to operate @ 170MHz (RM0440 section 5.3.3 table 29)
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_4WS;  // RM0440 5.3.3
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS);

    // regulator boost mode required to operate @ 170MHz (RM0440 section 6.1.5 table 38)
    PWR->CR5 &= ~PWR_CR5_R1MODE;

#ifndef USE_HSI
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY);
#else
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY);
#endif

    RCC->CRRCR |= RCC_CRRCR_HSI48ON;
    while ((RCC->CRRCR & RCC_CRRCR_HSI48RDY) != RCC_CRRCR_HSI48RDY);

    // pll configuration (RM0440 section 7.4.4)
    //      f(PLL_R) = F(HSE/HSI) * PLLN / (PLLM * PLLR)
    //
    // HSE:
    //      24e6 * 85 / (2 * 6) = 170e6
    // HSI:
    //      16e6 * 85 / (2 * 4) = 170e6
    RCC->PLLCFGR = (85 << RCC_PLLCFGR_PLLN_Pos) |  // 0b00: PLLR = 2 (RM0440 section 7.4.4)
#ifndef USE_HSI
        RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLM_2 | RCC_PLLCFGR_PLLM_0;  // 0b0101: PLLM = 6
#else
        RCC_PLLCFGR_PLLSRC_HSI | RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0;  // 0b0011: PLLM = 4
#endif

    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);

    // to switch to the high speed 170MHz clock, it is recommended to put
    // system clock into an intermediate frequency, for at least 1us.
    // run @ 85MHz by setting AHB prescaler to 2.
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL | RCC_CFGR_HPRE_DIV2;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // by iterating 85 times @ 170MHz and assuming that each iteration takes
    // at least two cycles (in practice it takes more), this loop will take at
    // least 1us
    for (__IO uint32_t i = 85; i; i--);

    // disable AHB prescaler.
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

#ifndef USE_HSI
    // disable HSI clock
    RCC->CR &= ~RCC_CR_HSION;
#endif

    // SysTick_Config(170000000 / 1000);
    SystemCoreClock = 170000000;
}
