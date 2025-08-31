// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stm32g4xx.h>
#include "audio.h"

#ifndef DAC_BUFFER_HALF_SIZE
#define DAC_BUFFER_HALF_SIZE 128
#endif

static int16_t buffer[DAC_BUFFER_HALF_SIZE + DAC_BUFFER_HALF_SIZE];
static int16_t *buffer_half = &buffer[DAC_BUFFER_HALF_SIZE];


void
audio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_DAC1EN | RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIOA->MODER |= GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4;
    OPAMP1->CSR = OPAMP_CSR_HIGHSPEEDEN | OPAMP_CSR_VMSEL | OPAMP_CSR_VPSEL_0 |
        OPAMP_CSR_OPAMPxEN;

    audio_callback(buffer, DAC_BUFFER_HALF_SIZE);

    DMAMUX1_Channel0->CCR = 6;

    DMA1_Channel1->CPAR = (uint32_t) &(DAC1->DHR12R1);
    DMA1_Channel1->CMAR = (uint32_t) &(buffer);
    DMA1_Channel1->CNDTR = DAC_BUFFER_HALF_SIZE + DAC_BUFFER_HALF_SIZE;
    DMA1_Channel1->CCR = DMA_CCR_DIR | DMA_CCR_PL | DMA_CCR_CIRC | DMA_CCR_PSIZE_1 |
        DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_EN;

    DAC1->MCR = DAC_MCR_HFSEL_1 | DAC_MCR_MODE1_1 | DAC_MCR_SINFORMAT1;
    DAC1->CR = DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0 | DAC_CR_DMAEN1 |
        DAC_CR_EN1;
    while ((DAC1->SR & DAC_SR_DAC1RDY) != DAC_SR_DAC1RDY);
    DAC1->CR |= DAC_CR_TEN1;

    TIM6->ARR = SystemCoreClock / 48000;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->CR2 = TIM_CR2_MMS_1;
    TIM6->CR1 = TIM_CR1_CEN;
}


bool
audio_task(void)
{
    if ((DMA1->ISR & DMA_ISR_HTIF1) == DMA_ISR_HTIF1) {
        DMA1->IFCR = DMA_IFCR_CHTIF1;
        audio_callback(buffer, DAC_BUFFER_HALF_SIZE);
        return true;
    }

    if ((DMA1->ISR & DMA_ISR_TCIF1) == DMA_ISR_TCIF1) {
        DMA1->IFCR = DMA_IFCR_CTCIF1;
        audio_callback(buffer_half, DAC_BUFFER_HALF_SIZE);
        return true;
    }
    return false;
}
