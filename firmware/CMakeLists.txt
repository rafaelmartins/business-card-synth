# SPDX-FileCopyrightText: 2025 Rafael G. Martins <rafael@rafaelmartins.eng.br>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.17)

include(FetchContent)

FetchContent_Declare(cmake_cmsis_stm32
    GIT_REPOSITORY https://github.com/rafaelmartins/cmake-cmsis-stm32.git
    GIT_TAG 5042adce80f41f937d09ea7db59f595f6718a7e8
)
FetchContent_MakeAvailable(
    cmake_cmsis_stm32
)

project(business-card-synth C ASM)

add_executable(business-card-synth
    main.c

    audio.c
    audio.h
    battery.c
    battery.h
    clock.c
    clock.h
    keyboard.c
    keyboard.h
)

target_compile_options(business-card-synth PRIVATE
    -Wall
    -Wextra
    -Werror
)

set_property(TARGET business-card-synth
    PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE
)

cmsis_stm32_target(business-card-synth
    DEVICE STM32G431xx
    VERSION 20241122015203
    LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32G431CBTx_FLASH.ld
    ADDITIONAL_OUTPUTS BIN MAP IHEX DFU
    SHOW_SIZE ON
    STLINK ON
)
