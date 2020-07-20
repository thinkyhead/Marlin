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
#pragma once

#include <cstdint>
#include <atomic>
#include <functional>

#include "../execution_control.h"
#include "src/inc/MarlinConfigPre.h"


typedef int16_t pin_type;

struct GpioEvent {
  enum Type {
    NOP,
    FALL,
    RISE,
    SET_VALUE,
    SETM,
    SETD,
    GET_VALUE
  };
  uint64_t timestamp;
  pin_type pin_id;
  GpioEvent::Type event;

  GpioEvent(uint64_t timestamp, pin_type pin_id, GpioEvent::Type event) : timestamp(timestamp), pin_id(pin_id), event(event) { }
};

class IOLogger {
public:
  virtual ~IOLogger(){};
  virtual void log(GpioEvent ev) = 0;
};

class Peripheral {
public:
  virtual ~Peripheral(){};
  virtual void update() = 0;
};

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
  template<class... Args>
  bool attach(Args... args) {
    callback = std::function<void(GpioEvent&)>((..., args));
    return true;
  }
  std::atomic_uint8_t pull;
  std::atomic_uint8_t dir;
  std::atomic_uint8_t mode;
  std::atomic_uint16_t value;
  std::function<void(GpioEvent&)> callback;
};

class Gpio {
public:

  static const pin_type pin_count = 255;
  static pin_data pin_map[pin_count+1];

  static bool valid_pin(pin_type pin) {
    return pin >= 0 && pin <= pin_count;
  }

  static void set(pin_type pin) {
    set(pin, 1);
  }

  static void set(pin_type pin, uint16_t value) {
    if (!valid_pin(pin)) return;
    GpioEvent::Type evt_type = value > 1 ? GpioEvent::SET_VALUE : value > pin_map[pin].value ? GpioEvent::RISE : value < pin_map[pin].value ? GpioEvent::FALL : GpioEvent::NOP;
    pin_map[pin].value = value;
    GpioEvent evt(kernel.ticks.load(), pin, evt_type);
    if (pin_map[pin].callback) {
      pin_map[pin].callback(evt);
    }
  }

  static uint16_t get(pin_type pin) {
    if (!valid_pin(pin)) return 0;
    GpioEvent evt(kernel.ticks.load(), pin, GpioEvent::GET_VALUE);
    if (pin_map[pin].callback) {
      pin_map[pin].callback(evt);
    }
    return pin_map[pin].value;
  }

  static void clear(pin_type pin) {
    set(pin, 0);
  }

  static void setMode(pin_type pin, uint8_t value) {
    if (!valid_pin(pin)) return;
    pin_map[pin].mode = pin_data::Mode::GPIO;

    GpioEvent evt(kernel.ticks.load(), pin, GpioEvent::Type::SETM);

    if (value != 1) setDir(pin, pin_data::Direction::INPUT);
    else setDir(pin, pin_data::Direction::OUTPUT);

    pin_map[pin].pull = value == 2 ? pin_data::Pull::PULLUP : value == 3 ? pin_data::Pull::PULLDOWN : pin_data::Pull::NONE;
    if (pin_map[pin].pull == pin_data::Pull::PULLUP) set(pin, pin_data::State::HIGH);

  }

  static uint8_t getMode(pin_type pin) {
    if (!valid_pin(pin)) return 0;
    return pin_map[pin].mode;
  }

  static void setDir(pin_type pin, uint8_t value) {
    if (!valid_pin(pin)) return;
    pin_map[pin].dir = value;
    GpioEvent evt(kernel.ticks.load(), pin, GpioEvent::Type::SETD);
    if (pin_map[pin].callback) pin_map[pin].callback(evt);
  }

  static uint8_t getDir(pin_type pin) {
    if (!valid_pin(pin)) return 0;
    return pin_map[pin].dir;
  }

  template<class... Args>
  static bool attach(const pin_type pin, Args... args) {
    if (!valid_pin(pin)) return false;
    return pin_map[pin].attach((..., args));
  }
};
