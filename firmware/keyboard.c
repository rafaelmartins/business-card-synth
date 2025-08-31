// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stm32g4xx.h>
#include "keyboard.h"

// Keys
//
// KBD01    - PA15
// KBD02    - PB4
// KBD03    - PB5
// KBD04    - PB6
// KBD05    - PB7
// KBD06    - PB9
// KBD07    - PB2
// KBD08    - PB1
// KBD09    - PB0
// KBD10    - PA7
// KBD10    - PA6
// KBD10    - PA5
// OCTAVE+  - PA8
// OCTAVE-  - PB11
//
// Leds
//
// OCTAVE - PA9

#ifndef KBD_DEBOUNCE_MS
#define KBD_DEBOUNCE_MS 10
#endif

static volatile uint16_t state = 0;

static volatile uint16_t debounce_state = 0;
static volatile uint8_t debounce_counter = 0;
static volatile bool debounce_done = false;

static volatile uint8_t octave = 4;


static uint16_t
keyboard_read(void)
{
    return
        (((GPIOA->IDR & GPIO_IDR_IDR_15) >> GPIO_IDR_ID15_Pos) << 0) |
        (((GPIOB->IDR & (GPIO_IDR_IDR_4 | GPIO_IDR_IDR_5 | GPIO_IDR_IDR_6 | GPIO_IDR_IDR_7)) >> GPIO_IDR_ID4_Pos) << 1) |
        (((GPIOB->IDR & GPIO_IDR_IDR_9) >> GPIO_IDR_ID9_Pos) << 5) |
        (((GPIOB->IDR & GPIO_IDR_IDR_2) >> GPIO_IDR_ID2_Pos) << 6) |
        (((GPIOB->IDR & GPIO_IDR_IDR_1) >> GPIO_IDR_ID1_Pos) << 7) |
        (((GPIOB->IDR & GPIO_IDR_IDR_0) >> GPIO_IDR_ID0_Pos) << 8) |
        (((GPIOA->IDR & GPIO_IDR_IDR_7) >> GPIO_IDR_ID7_Pos) << 9) |
        (((GPIOA->IDR & GPIO_IDR_IDR_6) >> GPIO_IDR_ID6_Pos) << 10) |
        (((GPIOA->IDR & GPIO_IDR_IDR_5) >> GPIO_IDR_ID5_Pos) << 11) |
        (((GPIOA->IDR & GPIO_IDR_IDR_8) >> GPIO_IDR_ID8_Pos) << 12) |
        (((GPIOB->IDR & GPIO_IDR_IDR_11) >> GPIO_IDR_ID11_Pos) << 13);
}


static uint32_t
led_duty_cycle(void)
{
    if (octave == 4)
        return 0;
    return octave * 1000;
}


void
keyboard_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM7EN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7 |
        GPIO_MODER_MODE8 | GPIO_MODER_MODE9 | GPIO_MODER_MODE15);
    GPIOA->MODER |= GPIO_MODER_MODER9_1;
    GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 |
        GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7 |
        GPIO_MODER_MODE9 | GPIO_MODER_MODE11);

    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5 | GPIO_OSPEEDR_OSPEED6 |
        GPIO_OSPEEDR_OSPEED7 | GPIO_OSPEEDR_OSPEED8 | GPIO_OSPEEDR_OSPEED8 |
        GPIO_OSPEEDR_OSPEED15;
    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1 |
        GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED5 |
        GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7 | GPIO_OSPEEDR_OSPEED9 |
        GPIO_OSPEEDR_OSPEED11;

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7 |
        GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD15);
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD5_0 | GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0 |
        GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD15_0;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 |
        GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7 |
        GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD11);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD0_0 | GPIO_PUPDR_PUPD1_0 | GPIO_PUPDR_PUPD2_0 |
        GPIO_PUPDR_PUPD4_0 | GPIO_PUPDR_PUPD5_0 | GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0 |
        GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD11_0;

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL9);
    GPIOA->AFR[1] |= GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_3;

    TIM2->PSC = SystemCoreClock / 10000 - 1;
    TIM2->ARR = 7999;
    TIM2->CCR3 = led_duty_cycle();
    TIM2->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
    TIM2->CCER = TIM_CCER_CC3E;
    TIM2->CR1 = TIM_CR1_CEN;

    TIM7->PSC = SystemCoreClock / 10000 - 1;
    TIM7->ARR = 9;
    TIM7->DIER = TIM_DIER_UIE;
    TIM7->CR1 = TIM_CR1_CEN | TIM_CR1_URS;

    for (__IO uint16_t i = 0xffff; i; i--);

    state = keyboard_read();
}


static void
keyboard_note_cb(uint8_t note, bool on)
{
    (void) note;
    (void) on;
}


bool
keyboard_task(void)
{
    if ((TIM7->SR & TIM_SR_UIF) != TIM_SR_UIF)
        return false;
    TIM7->SR &= ~TIM_SR_UIF;

    uint16_t tstate = keyboard_read();
    if (tstate != debounce_state) {
        debounce_state = tstate;
        debounce_counter = 0;
        debounce_done = false;
        return false;
    }

    if (debounce_done || (++debounce_counter > KBD_DEBOUNCE_MS))
        return false;

    for (uint8_t i = 0; i < 14; i++) {
        if ((debounce_state & (1 << i)) != (state & (1 << i))) {
            bool on = (debounce_state & (1 << i)) == 0;

            if (i == 12) {
                if (on && octave < 7) {
                    octave++;
                    TIM2->CCR3 = led_duty_cycle();
                }
                continue;
            }
            if (i == 13) {
                if (on && octave > 1) {
                    octave--;
                    TIM2->CCR3 = led_duty_cycle();
                }
                continue;
            }

            keyboard_note_cb((octave + 1) * 12 + i, on);
        }
    }

    debounce_done = true;
    state = debounce_state;
    return true;
}
