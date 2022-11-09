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
 * @Author       : tyrone
 * @Date         : 2022-08-16
 * @LastEditors  : tyrone
 * @LastEditTime : 2022-08-16
 * @Description  :
 */

#include "../../inc/MarlinConfig.h"

#if BOTH(ANKER_MAKE_API, ANKER_LIN_PARAMETER)

#include "../gcode.h"
#include "../../module/settings.h"

#define LIN_PARAM_LEN  12
static float lin_parameter[LIN_PARAM_LEN] = {0};

/**
 * M2030: if doesn't see any character then exit, others get the value after the character
 */
void GcodeSuite::M2030() {
  if (!parser.seen("ABCDEF")) {
    SERIAL_ECHOLNPGM("M2030 need parameter");
    return;
  }

  char *p = parser.command_ptr;
  int len = strlen(parser.command_ptr);

  if (len > 96) {
    SERIAL_ECHOLN("M2030 order too long");
    return;
  }

  for (int i = 0; i < len; i++) {
    if (*p >= 'A' && *p <= 'Z') {
      p++;
      float fl_result = strtof(p, nullptr);
      lin_parameter[*p - 'A'] = fl_result;
      SERIAL_ECHOPGM("lin_parameter:", *p - 'A');
      SERIAL_ECHOLNPAIR_F(" - result:", fl_result);
    }
    p++;
  }
}

float GcodeSuite::M2031() {
  if (!parser.seen("ABCDEF")) {
    SERIAL_ECHOLN("M2031 need parameter");
    return 0;
  }

  int i = -1;
  if (parser.seen_test('A')) i = 0;
  if (parser.seen_test('B')) i = 1;
  if (parser.seen_test('C')) i = 2;
  if (parser.seen_test('D')) i = 3;
  if (parser.seen_test('E')) i = 4;
  if (parser.seen_test('F')) i = 5;
  if (i < 0) return 0;

  SERIAL_ECHOLNPGM("lin_parameter[", i, "]:", lin_parameter[i]);
  return lin_parameter[i];
}

#endif // ANKER_MAKE_API && ANKER_LIN_PARAMETER
