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

#include <stdbool.h>

enum
{
  FILAMENT_RUNOUT_OFF = 0,
  FILAMENT_RUNOUT_ON,
  FILAMENT_SMART_RUNOUT_ON,
};

void PS_ON_Init(void);
void PS_ON_On(void);
void PS_ON_Off(void);
void positionSetUpdateWaiting(bool isWaiting);
void FIL_Runout_Init(void);
void loopBackEndFILRunoutDetect(void);
void loopFrontEndFILRunoutDetect(void);

#ifdef __cplusplus
}
#endif
