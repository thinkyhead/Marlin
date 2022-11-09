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
 * @Date         : 2022-04-27 20:35:23
 * @LastEditors  : harley
 * @LastEditTime :
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_ALIGN)

#include "anker_align.h"
#include "../../module/stepper.h"
#include "../../module/probe.h"
#include "../../gcode/gcode.h"
#include "../../module/temperature.h"

Anker_Align anker_align;

float Anker_Align::z1_value = 0;
float Anker_Align::z2_value = 0;
//uint8_t Anker_Align::anker_is_leveling=0;
xy_pos_t Anker_Align::xy[NUM_Z_STEPPER_DRIVERS];

void Anker_Align::init() {
  anker_align.xy[0].x = PROBING_MARGIN;
  anker_align.xy[0].y = 117.5;
  anker_align.xy[1].x = X_BED_SIZE - PROBING_MARGIN;
  anker_align.xy[1].y = 117.5;
  TERN_(ALIGN_PER_RESET, anker_align.reset());
}

void Anker_Align::add_z_value(float value, uint8_t choose) {
  stepper.set_separate_multi_axis(true);
  // Lock all steppers except one
  stepper.set_all_z_lock(true, choose);
  // Do a move to correct part of the misalignment for  the current stepper
  do_blocking_move_to_z(current_position.z + value);

  // Back to normal stepper operations
  stepper.set_all_z_lock(false);
  stepper.set_separate_multi_axis(false);
}

void Anker_Align::add_z1_value(float value) {
  anker_align.z1_value += value;
  anker_align.add_z_value(value, 0);
}

void Anker_Align::add_z2_value(float value) {
  anker_align.z2_value += value;
  anker_align.add_z_value(value, 1);
}

void Anker_Align::add_z1_value_no_save(float value) {
  anker_align.add_z_value(value, 0);
}

void Anker_Align::add_z2_value_no_save(float value) {
  anker_align.add_z_value(value, 1);
}

void Anker_Align::run_align() {
  float value = 0;
  value = anker_align.z1_value;
  anker_align.z1_value = 0;
  anker_align.add_z1_value(value);
  value = anker_align.z2_value;
  anker_align.z2_value = 0;
  anker_align.add_z2_value(value);
}

void Anker_Align::reset() {
  anker_align.z1_value = 0;
  anker_align.z2_value = 0;
  anker_align.run_align();
}

void Anker_Align::auto_align() {
  uint16_t num = 0;
  const ProbePtRaise raise_after =  PROBE_PT_RAISE;
  anker_align.init();
  gcode.process_subcommands_now_P(PSTR("G28"));
  for (num = 0; num < ALIGN_NUM; num++) {
    do_blocking_move_to_z(current_position.z + ALIGN_RISE);
    const float z1 = probe.probe_at_point(anker_align.xy[0], raise_after, 0, true, false);
    do_blocking_move_to_z(current_position.z + ALIGN_RISE);
    const float z2 = probe.probe_at_point(anker_align.xy[1], raise_after, 0, true, false);
    float rise_z   = 0;
    rise_z = (SCREW_DISTANCE / (anker_align.xy[1].x - anker_align.xy[0].x)) * ABS(z1 - z2);
    SERIAL_ECHOLNPGM(" z1:", z1);
    SERIAL_ECHOLNPGM(" z2:", z2);
    if (ABS(z1 - z2) > ALIGN_MAX_VALUE) {
      SERIAL_ECHOLNPGM("ok");
      SERIAL_ERROR_MSG("Adjustment range over 2mm!!\n");
      SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
      kill();
    }
    if (ABS(z1 - z2) <= ALIGN_ALLOWED) {
      SERIAL_ECHO_MSG("align ok!");
      gcode.process_subcommands_now_P(PSTR("M500\n"));
      break;
    }
    else if (z1 > z2) {
      anker_align.add_z1_value(rise_z);
    }
    else if (z2 > z1) {
      anker_align.add_z2_value(rise_z);
    }

    anker_align.xy[0].x = PROBING_MARGIN;
    anker_align.xy[0].y += 1.5;
    anker_align.xy[1].x = X_BED_SIZE - PROBING_MARGIN;
    anker_align.xy[1].y += 1.5;

    if (num == ALIGN_NUM - 1) {
      if (ABS(z1 - z2) > ALIGN_ALLOWED) {
        SERIAL_ECHO_MSG("Please check the Z-axis limit!");
        anker_align.reset();
      }
    }

  }
  gcode.process_subcommands_now_P(PSTR("G2001\n"));
}

#endif // ANKER_ALIGN

#if ENABLED(ANKER_ALIGN_ONLY_Z)

#include "anker_align.h"
#include "../../module/stepper.h"
#include "../../module/probe.h"
#include "../../gcode/gcode.h"
#include "../../module/temperature.h"

Anker_Align_Only_Z anker_align_only_z;
xy_pos_t Anker_Align_Only_Z::xy;

void Anker_Align_Only_Z::init() {
  //anker_align_only_z.xy[0].x = PROBING_MARGIN;
  //anker_align_only_z.xy[0].y = Y_BED_SIZE / 2;
  anker_align_only_z.xy.x = 190;
  anker_align_only_z.xy.y = 100;
}

void Anker_Align_Only_Z::auto_run() {
  uint16_t num = 0;
  const ProbePtRaise raise_after =  PROBE_PT_RAISE;
  gcode.process_subcommands_now_P(PSTR("G28"));
  anker_align_only_z.init();
  for (num = 0; num < ALIGN_NUM; num++) {
    do_blocking_move_to_z(current_position.z + ALIGN_RISE);
    const float z1 = probe.probe_at_point(  0, 100, raise_after, 0, true, false),
                z2 = probe.probe_at_point(200, 100, raise_after, 0, true, false);
    SERIAL_ECHOLNPGM(" z1:", z1);
    SERIAL_ECHOLNPGM(" z2:", z2);
  }
}

#endif // ANKER_ALIGN_ONLY_Z
