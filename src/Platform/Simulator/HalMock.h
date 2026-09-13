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

/* C header file required by hal.ll to pass on mock values to the Simulator.
 * See AGENTS.md section 9. */

#ifndef HAL_MOCK_H
#define HAL_MOCK_H

#include "HALConfig.h"

/* GPS::UartGetLine only closes a line on '\n' or '\0'. */
#define MOCK_UART_READ { \
    { .Channel = GPS_UART, .Type = HAL_MOCK_TEXT, \
      .Text = "$GPGGA,120000.00,2227.7653,S,04259.2148,W,1,08,0.9,345.2,M,0.0,M,,*5E\n" } }

#endif /* HAL_MOCK_H */
