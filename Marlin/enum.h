/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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

#ifndef __ENUM_H__
#define __ENUM_H__

/**
 * Axis indices as enumerated constants
 */
enum AxisEnum {
  NO_AXIS = -1,
  A_AXIS  = 0,
  B_AXIS  = 1,
  C_AXIS  = 2,
  D_AXIS  = 3
};

#define LOOP_ABCD(VAR) for (uint8_t VAR=A_AXIS; VAR<=D_AXIS; VAR++)

typedef enum {
  LINEARUNIT_MM,
  LINEARUNIT_INCH
} LinearUnit;

/**
 * Debug flags
 * Not yet widely applied
 */
enum DebugFlags {
  DEBUG_NONE          = 0,
  DEBUG_ECHO          = _BV(0), ///< Echo commands in order as they are processed
  DEBUG_INFO          = _BV(1), ///< Print messages for code that has debug output
  DEBUG_ERRORS        = _BV(2), ///< Not implemented
  DEBUG_DRYRUN        = _BV(3), ///< Ignore temperature setting and E movement commands
  DEBUG_COMMUNICATION = _BV(4), ///< Not implemented
  DEBUG_LEVELING      = _BV(5)  ///< Print detailed output for homing and leveling
};

enum EndstopEnum {
  A_MIN,
  B_MIN,
  C_MIN,
  D_MIN,
  A_MAX,
  B_MAX,
  C_MAX,
  D_MAX
};

#if ENABLED(EMERGENCY_PARSER)
  enum e_parser_state {
    state_RESET,
    state_N,
    state_M,
    state_M1,
    state_M11,
    state_M112,
    state_M4,
    state_M41,
    state_M410,
    state_IGNORE // to '\n'
  };
#endif

#endif // __ENUM_H__
