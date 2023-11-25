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

#define SPEED_NUM 2
#define SPEED_MIN 10
#define SPEED_MAX 999

#include <stdint.h>
#include <stdbool.h>

void speedSetSendWaiting(uint8_t tool, bool isWaiting);
void speedQuerySetWait(bool wait);
void speedSetPercent(uint8_t tool, uint16_t per);
uint16_t speedGetPercent(uint8_t tool);
bool SpeedChanged(uint8_t i);
void loopSpeed(void);
void speedQuery(void);

#ifdef __cplusplus
}
#endif
