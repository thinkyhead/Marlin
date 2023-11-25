/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2023 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Macros to make a string from a macro
#define STRINGIFY_(M) #M
#define STRINGIFY(M) STRINGIFY_(M)

#define NOBEYOND(min, v, max) _MAX(min, _MIN(v, max))

uint8_t inRange(int cur, int tag , int range);

uint8_t string_2_uint8_t(const uint8_t *string);
uint8_t *uint8_2_string(uint8_t num, uint8_t *string);
uint32_t string_2_uint32(const uint8_t *string, const uint8_t bytes_num);
uint8_t *uint32_2_string(uint32_t num, uint8_t bytes_num, uint8_t *string);

#ifdef __cplusplus
}
#endif
