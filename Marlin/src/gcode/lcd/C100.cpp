/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#if BOTH(HAS_PROBE_SETTINGS, DGUS_LCD_UI_CREALITY_TOUCH)

#include "../gcode.h"
#include "../../lcd/extui/dgus_creality/DGUSDisplay.h"
#include "../../lcd/extui/dgus_creality/DGUSScreenHandler.h"

void GcodeSuite::C100() {
  // C100 R1 (reset display)
  if (parser.boolval('R')) {
    dgusdisplay.ResetDisplay();
    safe_delay(1000); // Wait for display to come back
    ScreenHandler.Buzzer(250, 250);
  }

  // C100 P[num] (switch to page)
  if (parser.seenval('P')) {
    ScreenHandler.GotoScreen(static_cast<DGUSLCD_Screens>(parser.value_byte()));
    ScreenHandler.Buzzer(250, 250);
  }
}

#endif // HAS_PROBE_SETTINGS && DGUS_LCD_UI_CREALITY_TOUCH
