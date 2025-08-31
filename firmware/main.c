// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stm32g4xx.h>
#include "audio.h"
#include "battery.h"
#include "clock.h"
#include "keyboard.h"


void
audio_callback(int16_t *buf, uint16_t buf_len)
{
    (void) buf;
    (void) buf_len;
}


void
keyboard_note_callback(uint8_t note, bool on)
{
    (void) note;
    (void) on;
}


int
main(void)
{
    clock_init();
    battery_init();
    keyboard_init();
    audio_init();

    while (true) {
        if (audio_task())
            continue;

        if (keyboard_task())
            continue;

        if (battery_task())
            continue;
    }
    return 0;
}
