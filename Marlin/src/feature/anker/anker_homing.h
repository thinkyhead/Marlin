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

#include "../../inc/MarlinConfig.h"

#if ENABLED(USE_Z_SENSORLESS)
  typedef enum {
    Z_AXIS_IDLE = 0,
    Z_AXIS_IS_Z1,
    Z_AXIS_IS_Z2,
  } ANKER_Z_AXIS;
#endif

class Anker_Homing {
public:
  #if ENABLED(NO_MOTION_BEFORE_HOMING)
    static bool no_check_all_axis;
    static void anker_disable_motion_before_init();
    static void anker_disable_motion_before_check();
  #endif
  static bool is_home_z;
  static bool is_clean;
  static bool anker_z_homing_options;    // Actions that need to be performed only when zeroing Z
  #if ENABLED(USE_Z_SENSORLESS)
    static int is_again_homing_z_num;
    static bool is_again_probe_homing;
    static ANKER_Z_AXIS first_end_z_axis;    // Record the Z axis that reaches the end point first
    static millis_t trigger_per_ms;
    static millis_t trigger_ms;
    static void set_first_end_z_axis(ANKER_Z_AXIS);
    static ANKER_Z_AXIS get_first_end_z_axis();
    static void anker_dual_z_run_align();      // Roughly level the two Z axes
    static void set_trigger_per_ms();
    static void set_trigger_ms();
    static void set_probe_trigger_ms();
    static bool is_z_top_trigger();
    static bool is_other_z_no_trigger();      // The other axis is not firing
    static bool is_z_probe_no_trigger();
  #endif
  static bool is_center_home();        // Determine whether the center point needs to be zeroed
};

extern Anker_Homing anker_homing;
