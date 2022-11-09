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

/**
 * handshake.cpp
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(HANDSHAKE)

#include "handshake.h"

HandShake hand_shake;

void HandShake::init() {
  pinMode(HANDSHAKE_SDO, INPUT_FLOATING);
  SET_OUTPUT(HEATER_EN_PIN);
  WRITE(HEATER_EN_PIN, !HEATER_EN_STATE);

  if (READ(HANDSHAKE_SDO))
    WRITE(HEATER_EN_PIN, HEATER_EN_STATE);
  else
    SERIAL_ECHO_MSG("The nozzle is connected incorrectly");
}

void HandShake::handshake_test() {
  #if 0
    u8 status = 0;
    switch (status) {
      case 0: break;
      case 1: break;
      case 2: break;
      default: break;
    }

    SET_INPUT(HANDSHAKE_SDO);
    WRITE(HANDSHAKE_SDO, !HANDSHAKE_STATE);
    WRITE(PROBE_CONTROL_PIN, PROBE_CONTROL_STATE); // open power
    for (uint16_t time = 0; time < 20; time++) {
      WRITE(HANDSHAKE_SDO, !HANDSHAKE_STATE);
      _delay_ms(10);
      WRITE(HANDSHAKE_SDO, HANDSHAKE_STATE);
      _delay_ms(10);
    }
    SET_INPUT(HANDSHAKE_SDO);
  #endif
}

void HandShake::check() {
  static millis_t next_report_ms = millis() + HANDSHAKE_TIME;

  const millis_t ms = millis();
  if (ELAPSED(ms, next_report_ms)) {
    next_report_ms = ms + HANDSHAKE_TIME;
    pinMode(HANDSHAKE_SDO, INPUT_FLOATING);
    if (READ(HANDSHAKE_SDO))
      WRITE(HEATER_EN_PIN, HEATER_EN_STATE);
    else {
      WRITE(HEATER_EN_PIN, !HEATER_EN_STATE);
      //SERIAL_ECHO_MSG("The nozzle is connected incorrectly");
      SERIAL_ECHOLNPGM("echo:The nozzle:", READ(HANDSHAKE_SDO));
    }
  }
}

#endif // HANDSHAKE
