// SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>

void audio_init(void);
bool audio_task(void);

void audio_callback(int16_t *buf, uint16_t buf_len);
