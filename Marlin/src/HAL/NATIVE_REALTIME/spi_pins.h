/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include <src/core/macros.h>
#include <src/inc/MarlinConfigPre.h>

/** onboard SD card */
//#define SCK_PIN           P0_07
//#define MISO_PIN          P0_08
//#define MOSI_PIN          P0_09
//#define SS_PIN            P0_06
/** external */
#ifndef SCK_PIN
  #define SCK_PIN           50
#endif
#ifndef MISO_PIN
  #define MISO_PIN          51
#endif
#ifndef MOSI_PIN
  #define MOSI_PIN          52
#endif
#ifndef SS_PIN
  #define SS_PIN            53
#endif
#ifndef SDSS
  #define SDSS              SS_PIN
#endif
