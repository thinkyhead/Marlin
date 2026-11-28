/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2026 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#include "../test/unit_tests.h"
#include "src/lcd/marlinui.h"

// The default MAX_MESSAGE_BYTES can be 131 with long UTF filenames
MARLIN_TEST(expand_u8str, maxlen_over_127) {
  static const uint8_t limits[] = { 127, 128, 131, 255 };
  for (const uint8_t maxlen : limits) {
    char out[32] = "garbage";
    TEST_ASSERT_EQUAL(7, expand_u8str(out, F("Nozzle:"), 0, nullptr, nullptr, maxlen));
    TEST_ASSERT_EQUAL_STRING("Nozzle:", out);
  }
}

MARLIN_TEST(expand_u8str, maxlen_clips) {
  char out[32] = "garbage";
  TEST_ASSERT_EQUAL(3, expand_u8str(out, F("Nozzle:"), 0, nullptr, nullptr, 3));
  TEST_ASSERT_EQUAL_STRING("Noz", out);
}
