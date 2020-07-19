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
#ifdef __PLAT_NATIVE_REALTIME__

#include "gpio.h"

struct pin_data {
  enum Mode {
    GPIO,
    ADC,
    SPI,
    I2C,
    UART
  };
  enum Direction {
    INPUT,
    OUTPUT
  };
  enum Pull {
    NONE,
    PULLUP,
    PULLDOWN,
    TRISTATE
  };
  enum State {
    LOW,
    HIGH
  };
  uint8_t pull;
  uint8_t dir;
  uint8_t mode;
  uint16_t value;
};

static pin_data pin_map[Gpio::pin_count+1] = {};

bool Gpio::valid_pin(pin_type pin) {
  return pin >= 0 && pin <= pin_count;
}

void Gpio::set(pin_type pin) {
  set(pin, 1);
}

void Gpio::set(pin_type pin, bool value) {
  if (!valid_pin(pin)) return;
  pin_map[pin].value = value;
}

bool Gpio::get(pin_type pin) {
  if (!valid_pin(pin)) return 0;
  return pin_map[pin].value;
}

//todo: dont just output room temperature
uint16_t Gpio::get_adc(pin_type pin) {
  if (!valid_pin(pin)) return 0;
  return pin_map[pin].value;
}

void Gpio::clear(pin_type pin) {
  set(pin, 0);
}

void Gpio::setMode(pin_type pin, uint8_t value) {
  if (!valid_pin(pin)) return;
  pin_map[pin].mode = pin_data::Mode::GPIO;

  if (value != 1) setDir(pin, pin_data::Direction::INPUT);
  else setDir(pin, pin_data::Direction::OUTPUT);

  pin_map[pin].pull = value == 2 ? pin_data::Pull::PULLUP : value == 3 ? pin_data::Pull::PULLDOWN : pin_data::Pull::NONE;
  if (pin_map[pin].pull == pin_data::Pull::PULLUP) set(pin, pin_data::State::HIGH);

}

uint8_t Gpio::getMode(pin_type pin) {
  if (!valid_pin(pin)) return 0;
  return pin_map[pin].mode;
}

void Gpio::setDir(pin_type pin, uint8_t value) {
  if (!valid_pin(pin)) return;
  pin_map[pin].dir = value;
}

uint8_t Gpio::getDir(pin_type pin) {
  if (!valid_pin(pin)) return 0;
  return pin_map[pin].dir;
}

#endif // __PLAT_NATIVE_REALTIME__
