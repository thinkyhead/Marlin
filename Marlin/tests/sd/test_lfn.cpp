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
#include "src/inc/MarlinConfig.h"

#if ALL(HAS_MEDIA, UTF_FILENAME_SUPPORT)

#include "src/sd/cardreader.h"

// One byte past the name buffer, to catch a decoder that reads or relies on it
static char lfn[LONG_FILENAME_BYTES + 1];

// Fill the buffer as readDir does, with UTF-16LE, then decode it
static void decode(const uint16_t ch, const uint8_t len) {
  memset(lfn, 0x55, sizeof(lfn));
  for (uint8_t i = 0; i < len; ++i) {
    lfn[i * 2] = ch & 0xFF;
    lfn[i * 2 + 1] = ch >> 8;
  }
  if (len < LONG_FILENAME_CHARS) lfn[len * 2] = lfn[len * 2 + 1] = '\0';
  SdBaseFile::convertUtf16ToUtf8(lfn);
}

MARLIN_TEST(lfn, full_length_name_is_terminated) {
  decode('a', LONG_FILENAME_CHARS);
  TEST_ASSERT_EQUAL(LONG_FILENAME_CHARS, strlen(lfn));
  TEST_ASSERT_EQUAL(0x55, lfn[LONG_FILENAME_BYTES]);
}

MARLIN_TEST(lfn, full_length_two_byte_name_fits) {
  decode(0x0436, LONG_FILENAME_CHARS);
  TEST_ASSERT_EQUAL(LONG_FILENAME_BYTES - 1, strlen(lfn));
  TEST_ASSERT_EQUAL(0x55, lfn[LONG_FILENAME_BYTES]);
}

// A low byte of 0x80 or more must not be sign-extended
MARLIN_TEST(lfn, latin1_character) {
  decode(0x00E9, 1);
  TEST_ASSERT_EQUAL_STRING("\xC3\xA9", lfn);
}

MARLIN_TEST(lfn, low_byte_zero_is_not_the_end) {
  decode(0x0100, 1);
  TEST_ASSERT_EQUAL_STRING("\xC4\x80", lfn);
}

#endif
