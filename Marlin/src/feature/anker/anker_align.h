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
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime :
 * @Description  :
 */
#if ENABLED(ANKER_ALIGN)

class Anker_Align {
public:
  static xy_pos_t xy[NUM_Z_STEPPER_DRIVERS];
  static float z1_value, z2_value;
  // static uint8_t anker_is_leveling;
  void init();
  void reset();
  void run_align();      // Run after double Z reset
  void auto_align();     // Automatically detect two points on the heated bed to calculate the distance
  void add_z_value(float value, uint8_t choose);
  void add_z1_value(float value);
  void add_z2_value(float value);
  void add_z1_value_no_save(float value);
  void add_z2_value_no_save(float value);
};

extern Anker_Align anker_align;

#endif // ANKER_ALIGN

#if ENABLED(ANKER_ALIGN_ONLY_Z)

class Anker_Align_Only_Z {
public:
  static xy_pos_t xy;
  void init();
  void auto_run();
};

extern Anker_Align_Only_Z anker_align_only_z;

#endif // ANKER_ALIGN_ONLY_Z
