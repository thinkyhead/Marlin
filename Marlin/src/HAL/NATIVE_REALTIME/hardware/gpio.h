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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "Clock.h"
#include <src/inc/MarlinConfigPre.h>
#include <stdint.h>

typedef int16_t pin_type;

struct Gpio {
  static constexpr pin_type pin_count = 255;

  static bool valid_pin(pin_type pin);
  static void set(pin_type pin);
  static void set(pin_type pin, bool value);
  static bool get(pin_type pin);
  static uint16_t get_adc(pin_type pin);
  static void clear(pin_type pin);
  static void setMode(pin_type pin, uint8_t value);
  static uint8_t getMode(pin_type pin);
  static void setDir(pin_type pin, uint8_t value);
  static uint8_t getDir(pin_type pin);
};
