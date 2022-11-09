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
 * @Author       : Harley
 * @Date         : 2022-05-28 10:50:23
 * @LastEditors  : Harley
 * @LastEditTime : 2022-05-28 14:03:20
 * @Description  :
 */
#include "anker_z_sensorless.h"

#if ENABLED(USE_Z_SENSORLESS)

#include "../../module/stepper/trinamic.h"

#if ENABLED(ANKER_NOZZLE_BOARD)
  #include "anker_nozzle_board.h"
#endif

Use_Z_Sensorless use_z_sensorless;

uint16_t Use_Z_Sensorless::z1_stall_value = 0,
         Use_Z_Sensorless::z2_stall_value = 0;

void Use_Z_Sensorless::init() {
  stepperZ.anker_homing_threshold(use_z_sensorless.z1_stall_value);
  #ifdef ANKER_Z2_STALL_SENSITIVITY
    stepperZ2.anker_homing_threshold(use_z_sensorless.z2_stall_value);
  #endif
}

void Use_Z_Sensorless::reset() {
  use_z_sensorless.z1_stall_value = ANKER_Z_STALL_SENSITIVITY;
  #ifdef ANKER_Z2_STALL_SENSITIVITY
    use_z_sensorless.z2_stall_value = ANKER_Z2_STALL_SENSITIVITY;
  #endif
}

void Use_Z_Sensorless::set_z1_value(uint16_t z1_value) {
  use_z_sensorless.z1_stall_value = z1_value;
  stepperZ.anker_homing_threshold(use_z_sensorless.z1_stall_value);
}

#ifdef ANKER_Z2_STALL_SENSITIVITY
  void Use_Z_Sensorless::set_z2_value(uint16_t z2_value) {
    use_z_sensorless.z2_stall_value = z2_value;
    stepperZ2.anker_homing_threshold(use_z_sensorless.z2_stall_value);
  }
#endif

void Use_Z_Sensorless::report() {
  SERIAL_ECHOLNPGM(" z1-stall:", use_z_sensorless.z1_stall_value);
  #ifdef ANKER_Z2_STALL_SENSITIVITY
    SERIAL_ECHOLNPGM(" z2-stall:", use_z_sensorless.z2_stall_value);
  #endif
}

void Use_Z_Sensorless::z_sensorless_kill(uint8_t sta) {
  if (sta == Z_SENSORLESS_KILL_STA && get_anker_nozzle_board_info()->is_z_sensorless) {
    SERIAL_ECHO(Z_SENSORLESS_KILL_MSG);
    kill();
  }
}

#endif // USE_Z_SENSORLESS
