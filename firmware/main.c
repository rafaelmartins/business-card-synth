// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stm32g4xx.h>
#include "battery.h"
#include "clock.h"
#include "keyboard.h"


int
main(void)
{
    clock_init();
    battery_init();
    keyboard_init();

    while (true) {
        if (keyboard_task())
            continue;

        if (battery_task())
            continue;
    }
    return 0;
}
