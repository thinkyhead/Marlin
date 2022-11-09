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
 * anker_log_debug.cpp
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_LOG_DEBUG)

#include "../../core/serial.h"
#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../module/planner.h"

int anker_debug_flag;

void anker_check_block_buf() {
  static millis_t next_report_ms = millis() + 5000;

  const millis_t ms = millis();
  if (ELAPSED(ms, next_report_ms)) {
    next_report_ms = ms + 5000;
    SERIAL_ECHOLNPGM("next_report_ms:", next_report_ms);
    SERIAL_ECHOLNPGM("anker_debug_flag:", anker_debug_flag);
    if (queue.ring_buffer.empty() && anker_debug_flag)
      SERIAL_ECHOLNPGM("queue.ring_buffer empty");
    if (!planner.get_current_block() && anker_debug_flag)
      SERIAL_ECHOLNPGM("current_block NULL");
  }
}

void set_anker_debug_flag(const int value) { anker_debug_flag = value; }

int get_anker_debug_flag() { return anker_debug_flag; }

#endif // ANKER_LOG_DEBUG
