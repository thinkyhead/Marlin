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
#include <stdbool.h>

#define FAN_TYPE_F        0  // Default cooling fan
#define FAN_TYPE_CTRL_S   1  // Controller FAN on Stepper/Bed On  (Check - Marlin M710)
#define FAN_TYPE_CTRL_I   2  // Controller FAN on Idle            (Check - Marlin M710)
#define FAN_TYPE_UNKNOWN  8  // Unknown / Not defined!

extern char*   fanID[];
extern char*   fanCmd[];
extern uint8_t fanType[];

bool    fanIsType(uint8_t i, uint8_t type);
uint8_t fanGetTypID(uint8_t startIndex, uint8_t type);
void    fanSetSpeed(uint8_t i, uint8_t speed);
uint8_t fanGetSpeed(uint8_t i);
void    fanSetSpeedPercent(uint8_t i, uint8_t percent);
uint8_t fanGetSpeedPercent(uint8_t i);
void    fanSetSendWaiting(uint8_t i, bool isWaiting);
void    fanBuildList(void);
void    fanControlInit(void);
void    loopFan(void);

void    fanSpeedQuery(void);
void    fanSpeedQuerySetWait(bool wait);

#ifdef __cplusplus
}
#endif
