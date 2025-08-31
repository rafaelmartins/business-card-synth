// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stm32g4xx.h>
#include "battery.h"

static volatile bool on_bat = false;

void
battery_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

    PWR->CR3 |= PWR_CR3_UCPD_DBDIS;

    GPIOA->MODER &= ~GPIO_MODER_MODE10;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10_1;
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0;

    GPIOB->MODER &= ~GPIO_MODER_MODE10;
    GPIOB->MODER |= GPIO_MODER_MODE10_0;
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10_1;
    GPIOB->BSRR = GPIO_BSRR_BR10;

    on_bat = (GPIOA->IDR & GPIO_IDR_IDR_10) != GPIO_IDR_IDR_10;
    if (on_bat)
        GPIOB->BSRR = GPIO_BSRR_BS10;
}


bool
battery_task(void)
{
    bool on_bat_now = (GPIOA->IDR & GPIO_IDR_IDR_10) != GPIO_IDR_IDR_10;
    if (on_bat_now != on_bat) {
        on_bat = on_bat_now;
        GPIOB->BSRR = on_bat ? GPIO_BSRR_BS10 : GPIO_BSRR_BR10;
        return true;
    }
    return false;
}
