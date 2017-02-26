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

#include "MarlinConfig.h"

#if ENABLED(MAKERARM_SCARA)

#include "Tool.h"

#include <Wire.h>

Tool::Tool() {
}

// Activate this tool
void Tool::activate() {
}

// Init the eeprom of the attached tool
void Tool::init_eeprom() {
}

// Read the tool eeprom
void Tool::read_eeprom() {
}

// Write current values into the tool eeprom
void Tool::write_eeprom() {
}

// A tool was attached
void Tool::attach() {
}

// A handler for all tool attach detection
void Tool::autodetect() {
}

#endif // MAKERARM_SCARA
