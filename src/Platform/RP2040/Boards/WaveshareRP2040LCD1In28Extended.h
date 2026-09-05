/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (MCUs like RP2040 and ESP32-S3).
    Copyright (C) 2022, Julianno F. C. Silva (@juliannojungle)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.
*/

/* pico-sdk board definition for the Waveshare RP2040-LCD-1.28 carrying pedal.guru's
 * expansion board. Selected with PICO_BOARD, and found through PICO_BOARD_HEADER_DIRS.
 *
 * This file holds ONLY what the pico-sdk and the cyw43 driver read directly: whether the
 * board has a radio, and where that radio is wired. Every other pin on the device --
 * SD card, LCD, GPS, navigation switches -- is read by our own code and therefore lives
 * in hal.ll's HALConfig.h instead. The radio is the one case where a third party looks up
 * the pin numbers, so they have to be declared where it looks.
 *
 * The base board has no radio; the module sits on the expansion board. It is RP2040-only
 * by nature (the ESP32-S3 has its radio built in), so it is allowed pins that are not
 * shared between the two MCUs -- a board built for the ESP32-S3 leaves the footprint
 * unpopulated. See pedal.guru's AGENTS.md §3.
 */

#ifndef _BOARDS_WAVESHARE_RP2040_LCD_1_28_EXTENDED_H
#define _BOARDS_WAVESHARE_RP2040_LCD_1_28_EXTENDED_H

/* Read by CMake, not by the compiler: the pico-sdk scrapes this out of the header and
 * only creates the pico_cyw43_arch* targets when it is set. Without it, net.ll's radio
 * code cannot even find pico/cyw43_arch.h. */
pico_board_cmake_set(PICO_CYW43_SUPPORTED, 1)

/* For board detection from application code, mirroring the pico-sdk's convention. */
#define WAVESHARE_RP2040_LCD_1_28_EXTENDED

/* Radio wiring: GPIO 19-22, which are header H2 positions 8, 10, 12 and 14 -- four
 * contiguous positions in one column. None of these pins needs a special peripheral
 * function: the gSPI bus is a PIO program, and PIO reaches any GPIO. CLOCK and DATA are
 * the only timing-critical pair, hence kept adjacent.
 *
 * DATA_OUT, DATA_IN and HOST_WAKE are deliberately the same pin: the CYW43's gSPI is
 * half-duplex, so one wire carries the outgoing command, the reply, and the module's
 * interrupt. That is also why the hardware SPI peripheral cannot drive it. */
#ifndef CYW43_DEFAULT_PIN_WL_REG_ON
#define CYW43_DEFAULT_PIN_WL_REG_ON 19
#endif
#ifndef CYW43_DEFAULT_PIN_WL_DATA_OUT
#define CYW43_DEFAULT_PIN_WL_DATA_OUT 20
#endif
#ifndef CYW43_DEFAULT_PIN_WL_DATA_IN
#define CYW43_DEFAULT_PIN_WL_DATA_IN 20
#endif
#ifndef CYW43_DEFAULT_PIN_WL_HOST_WAKE
#define CYW43_DEFAULT_PIN_WL_HOST_WAKE 20
#endif
#ifndef CYW43_DEFAULT_PIN_WL_CLOCK
#define CYW43_DEFAULT_PIN_WL_CLOCK 21
#endif
#ifndef CYW43_DEFAULT_PIN_WL_CS
#define CYW43_DEFAULT_PIN_WL_CS 22
#endif

/* The base board's own definition. Included last so every #ifndef-guarded value above
 * wins over it -- and the pico-sdk follows this include to scrape its cmake directives
 * too, so PICO_PLATFORM and PICO_FLASH_SIZE_BYTES come from there. */
#include "boards/waveshare_rp2040_lcd_1.28.h"

#endif
