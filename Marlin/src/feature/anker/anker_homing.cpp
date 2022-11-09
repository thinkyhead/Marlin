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

/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime :
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(EVT_HOMING_5X)

#include "anker_homing.h"

Anker_Homing anker_homing;

bool Anker_Homing::is_home_z = false;
bool Anker_Homing::is_clean = false;
bool Anker_Homing::anker_z_homing_options = false;    // Actions that need to be performed only when zeroing Z

#if ENABLED(NO_MOTION_BEFORE_HOMING)

  bool Anker_Homing::no_check_all_axis; // = false

  void Anker_Homing::anker_disable_motion_before_init() {
    SET_INPUT_PULLUP(DISABLED_MOTION_BEFORE_PIN);
  }

  void Anker_Homing::anker_disable_motion_before_check() {
    no_check_all_axis = READ(DISABLED_MOTION_BEFORE_PIN) == DISABLED_MOTION_BEFORE_STATE;
  }

#endif


#if ENABLED(USE_Z_SENSORLESS)
  int Anker_Homing::is_again_homing_z_num = 0;
  bool Anker_Homing::is_again_probe_homing = false;
  ANKER_Z_AXIS Anker_Homing::first_end_z_axis = Z_AXIS_IDLE;    // Record the Z axis that reaches the end point first
#endif

#if ENABLED(USE_Z_SENSORLESS)

  #include "../bedlevel/bedlevel.h"
  #include "../../module/motion.h"
  #include "../../module/probe.h"
  #include "anker_nozzle_board.h"

  void Anker_Homing::set_first_end_z_axis(ANKER_Z_AXIS z) { first_end_z_axis = z; }
  ANKER_Z_AXIS Anker_Homing::get_first_end_z_axis() { return first_end_z_axis; }

  #define FIRST_END_Z1_POS_X              235
  #define FIRST_END_Z1_POS_Y              237
  #define FIRST_END_Z2_POS_X              0
  #define FIRST_END_Z2_POS_Y              237
  #define DUAL_Z_NOZZLE_BELOW_BED_MM      4       // The height of the nozzle below the heated bed when the double Z is locked and reset to zero
  #define DUAL_Z_NOZZLE_BELOW_BED_STEP_MM 0.1
  #define HOMING_RUN_ALIGN_MM             20
  #define HOMING_RUN_ALIGN_STEP_MM        0.1

  void Anker_Homing::anker_dual_z_run_align() {
    bool run_align = true;
    xyz_pos_t first_z1_pos = { FIRST_END_Z1_POS_X, FIRST_END_Z1_POS_Y, current_position.z },
              first_z2_pos = { FIRST_END_Z2_POS_X, FIRST_END_Z2_POS_Y, current_position.z };
    if (first_end_z_axis == Z_AXIS_IS_Z1)
      do_blocking_move_to(first_z1_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S));
    else if (first_end_z_axis == Z_AXIS_IS_Z2)
      do_blocking_move_to(first_z2_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S));

    set_anker_z_sensorless_probe_value(800);

    #if ENABLED(PROBE_CONTROL)
      WRITE(PROBE_CONTROL_PIN, !PROBE_CONTROL_STATE);
      probe.anker_level_set_probing_paused(true, ANKER_LEVELING_DELAY_BEFORE_PROBING);
      WRITE(PROBE_CONTROL_PIN, PROBE_CONTROL_STATE);
    #endif

    for (uint16_t i = 0; i < (DUAL_Z_NOZZLE_BELOW_BED_MM / DUAL_Z_NOZZLE_BELOW_BED_STEP_MM); i++) {
      do_blocking_move_to_z(current_position.z - DUAL_Z_NOZZLE_BELOW_BED_STEP_MM);
      if (PROBE_TRIGGERED()) {
        SERIAL_ECHOLNPGM(" probe first triggered!");
        run_align = false;
        break;
      }
    }
    if (run_align) {
      run_align = false;
      if (first_end_z_axis == Z_AXIS_IS_Z1) {
        for (uint16_t i = 0; i < (HOMING_RUN_ALIGN_MM / HOMING_RUN_ALIGN_STEP_MM); i++) {
          anker_align.add_z2_value(-HOMING_RUN_ALIGN_STEP_MM);
          if (PROBE_TRIGGERED()) {
            SERIAL_ECHOLNPGM("right probe triggered!");
            break;
          }
        }
      }
      else if (first_end_z_axis == Z_AXIS_IS_Z2) {
        for (uint16_t i = 0; i < (HOMING_RUN_ALIGN_MM / HOMING_RUN_ALIGN_STEP_MM); i++) {
          anker_align.add_z1_value(-HOMING_RUN_ALIGN_STEP_MM);
          if (PROBE_TRIGGERED()) {
            SERIAL_ECHOLNPGM("lift probe triggered!");
            break;
          }
        }
      }
    }

    #if ENABLED(PROBE_CONTROL)
      WRITE(PROBE_CONTROL_PIN, !PROBE_CONTROL_STATE);
    #endif
    reset_anker_z_sensorless_probe_value();
  }

  void Anker_Homing::set_trigger_per_ms() {
    trigger_per_ms = millis();
  }

  void Anker_Homing::set_trigger_ms() {
    trigger_ms = millis();
    SERIAL_ECHOLNPGM(" trigger!!");
  }

  bool Anker_Homing::is_z_top_trigger() {
    SERIAL_ECHOLNPGM(" trigger_per_ms:", trigger_per_ms);
    SERIAL_ECHOLNPGM(" trigger_ms:", trigger_ms);
    if (trigger_ms < (trigger_per_ms + ANKER_TIME_Z_MIN_LIMIT))
      return true;
    else return false;
  }

  bool Anker_Homing::is_other_z_no_trigger() {
    SERIAL_ECHOLNPGM(" other_z trigger_per_ms:", trigger_per_ms);
    SERIAL_ECHOLNPGM(" other_z trigger_ms:", trigger_ms);
    if (trigger_ms > (trigger_per_ms + ANKER_TIME_ANTHOR_Z_MAX_LIMIT))
      return true;
    else return false;
  }

  void Anker_Homing::set_probe_trigger_ms() {
    trigger_ms = millis();
    SERIAL_ECHOLNPGM(" probe trigger!!");
  }

  bool Anker_Homing::is_z_probe_no_trigger() { // If the probe does not trigger for a long time, reset Z to zero
    return (millis() > trigger_per_ms + ANKER_PROBE_TIMEOUT);
  }

#endif // USE_Z_SENSORLESS

bool Anker_Homing::is_center_home() {   // Determine whether the center point needs to be zeroed
  return anker_z_homing_options
         && TERN1(USE_Z_SENSORLESS, !is_again_probe_homing);
}

#endif // EVT_HOMING_5X
