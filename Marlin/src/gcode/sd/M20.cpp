/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#if ENABLED(SDSUPPORT)

#include "../gcode.h"
#include "../../sd/cardreader.h"

/**
 * M20: List SD card to serial output
 */
void GcodeSuite::M20() {
  if (card.flag.mounted) {
    #if ENABLED(RRF_GCODE_DIALECT)
      const bool json = parser.intval('S') == 2;
      if (json) {
        // The P parameter gives the path
        char *path = parser.stringval('P');

        #if DISABLED(GCODE_QUOTED_STRINGS)
          // 'S' before 'P' or 'S' is part of the path
          if (path < parser.stringval('S')) path = nullptr;
        #endif

        // Get the CWD as the root for dive
        SdFile *listDirPtr;

        bool ok = true;

        // If any path was specified, dive into it
        if (path && path[0]) {
          // Ensure the path ends with a slash
          const size_t len = strlen(path);
          if (len > 1 && path[len - 1] != '/') strcat(path, "/");
          // Dive listDirPtr down to the path
          ok = (bool)card.diveToFile(false, listDirPtr, path);
        }
        else
          path = (char*)".";

        if (ok) {
          // Remove the slash at the end
          const size_t len = strlen(path);
          if (len > 1 && path[len - 1] == '/') path[len - 1] = '\0';
          // Print a flat listing of the folder in JSON
          SERIAL_ECHOPAIR("{\"dir\":\"", path, "\",");
          card.lsJSON(0, *listDirPtr); // Don't enter subfolders (but list folders)
          SERIAL_ECHOLNPAIR("}");
        }
      }
    #else
      constexpr bool json = false;
    #endif

    if (!json) {
      SERIAL_ECHOLNPGM(STR_BEGIN_FILE_LIST);
      card.ls();
      SERIAL_ECHOLNPGM(STR_END_FILE_LIST);
    }
  }
  else
    SERIAL_ECHO_MSG(STR_NO_MEDIA);
}

#endif // SDSUPPORT
