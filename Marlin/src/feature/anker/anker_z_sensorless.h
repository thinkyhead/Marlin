/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2022 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

/*
 * @Author       : Harley
 * @Date         : 2022-05-28 10:50:23
 * @LastEditors  : Harley
 * @LastEditTime : 2022-05-28 14:03:20
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
#include "../../gcode/queue.h"

#define Z_SENSORLESS_KILL_STA  0
#define Z_SENSORLESS_KILL_MSG  "echo:Z homing foreign object detected!"

class Use_Z_Sensorless {
public:
  static uint16_t z1_stall_value;
  static uint16_t z2_stall_value;
  void init();
  void reset();
  void set_z1_value(uint16_t z1_value);
  #ifdef ANKER_Z2_STALL_SENSITIVITY
    void set_z2_value(uint16_t z2_value);
  #endif
  void report();
  void z_sensorless_kill(uint8_t sta);
};

extern Use_Z_Sensorless use_z_sensorless;
