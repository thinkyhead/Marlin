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

#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_MAKE_API)

#include "../gcode.h"

#if ENABLED(BOARD_CONFIGURE)
  #include "../../feature/anker/board_configure.h"
#endif

/**
 * M116 M116 S0 Check the firmware version number
 * M116 S1 Check the hardware version number
 * M116 S2 Check the model of the main control chip
 */
void GcodeSuite::M116() {
  if (parser.seen('S')) {
    switch (parser.value_int()) {
      case 0:
        SERIAL_ECHOLNPGM(DETAILED_BUILD_VERSION);
        break;
      #if ENABLED(BOARD_CONFIGURE)
        case 1: SERIAL_ECHOLN(board_configure.board_version); break;
        case 2: SERIAL_ECHOLN(board_configure.board_chip); break;
      #endif
      default: break;
    }
  }
  else
   SERIAL_ECHOLNPGM(DETAILED_BUILD_VERSION);
}

#endif // ANKER_MAKE_API
