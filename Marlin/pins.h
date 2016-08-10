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

#ifndef PINS_H
#define PINS_H

#define IS_RAMPS_EFB
#include "pins_RAMPS.h"

// Define certain undefined pins
#ifndef A_MS1_PIN
  #define A_MS1_PIN -1
#endif
#ifndef A_MS2_PIN
  #define A_MS2_PIN -1
#endif
#ifndef B_MS1_PIN
  #define B_MS1_PIN -1
#endif
#ifndef B_MS2_PIN
  #define B_MS2_PIN -1
#endif
#ifndef C_MS1_PIN
  #define C_MS1_PIN -1
#endif
#ifndef C_MS2_PIN
  #define C_MS2_PIN -1
#endif
#ifndef D_MS1_PIN
  #define D_MS1_PIN -1
#endif
#ifndef D_MS2_PIN
  #define D_MS2_PIN -1
#endif

#ifndef CONTROLLERFAN_PIN
  #define CONTROLLERFAN_PIN  -1
#endif

#ifndef PS_ON_PIN
  #define PS_ON_PIN -1
#endif
#ifndef KILL_PIN
  #define KILL_PIN -1
#endif
#ifndef SUICIDE_PIN
  #define SUICIDE_PIN -1
#endif

//
// Disable unused endstop / probe pins
//
#if DISABLED(USE_AMAX_PLUG)
  #undef A_MAX_PIN
  #define A_MAX_PIN          -1
#endif

#if DISABLED(USE_BMAX_PLUG)
  #undef B_MAX_PIN
  #define B_MAX_PIN          -1
#endif

#if DISABLED(USE_CMAX_PLUG)
  #undef C_MAX_PIN
  #define C_MAX_PIN          -1
#endif

#if DISABLED(USE_DMAX_PLUG)
  #undef D_MAX_PIN
  #define D_MAX_PIN          -1
#endif

#if DISABLED(USE_AMIN_PLUG)
  #undef A_MIN_PIN
  #define A_MIN_PIN          -1
#endif

#if DISABLED(USE_BMIN_PLUG)
  #undef B_MIN_PIN
  #define B_MIN_PIN          -1
#endif

#if DISABLED(USE_CMIN_PLUG)
  #undef C_MIN_PIN
  #define C_MIN_PIN          -1
#endif

#if DISABLED(USE_DMIN_PLUG)
  #undef D_MIN_PIN
  #define D_MIN_PIN          -1
#endif

#define SENSITIVE_PINS { 0, 1, \
    A_STEP_PIN, A_DIR_PIN, A_ENABLE_PIN, A_MIN_PIN, A_MAX_PIN, \
    B_STEP_PIN, B_DIR_PIN, B_ENABLE_PIN, B_MIN_PIN, B_MAX_PIN, \
    C_STEP_PIN, C_DIR_PIN, C_ENABLE_PIN, C_MIN_PIN, C_MAX_PIN, \
    D_STEP_PIN, D_DIR_PIN, D_ENABLE_PIN, D_MIN_PIN, D_MAX_PIN, \
    PS_ON_PIN, CONTROLLERFAN_PIN, \
    A_MS1_PIN, A_MS2_PIN, B_MS1_PIN, B_MS2_PIN, C_MS1_PIN, C_MS2_PIN \
  }

#define HAS_DIGIPOTSS (PIN_EXISTS(DIGIPOTSS))

/**
 * Define SPI Pins: SCK, MISO, MOSI, SS
 */
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
  #define AVR_SCK_PIN  13
  #define AVR_MISO_PIN 12
  #define AVR_MOSI_PIN 11
  #define AVR_SS_PIN   10
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) || defined(__AVR_ATmega1284P__)
  #define AVR_SCK_PIN  7
  #define AVR_MISO_PIN 6
  #define AVR_MOSI_PIN 5
  #define AVR_SS_PIN   4
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #define AVR_SCK_PIN  52
  #define AVR_MISO_PIN 50
  #define AVR_MOSI_PIN 51
  #define AVR_SS_PIN   53
#elif defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB647__)
  #if ENABLED(AT90USBxx_TEENSYPP_ASSIGNMENTS)
    // Teensy pin assignments
    #define AVR_SCK_PIN  21
    #define AVR_MISO_PIN 23
    #define AVR_MOSI_PIN 22
    #define AVR_SS_PIN   20
  #else
    // Traditional pin assignments
    #define AVR_SCK_PIN  9
    #define AVR_MISO_PIN 11
    #define AVR_MOSI_PIN 10
    #define AVR_SS_PIN   8
  #endif
#elif defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
  #define AVR_SCK_PIN  10
  #define AVR_MISO_PIN 12
  #define AVR_MOSI_PIN 11
  #define AVR_SS_PIN   16
#endif

#ifndef SCK_PIN
  #define SCK_PIN  AVR_SCK_PIN
#endif
#ifndef MISO_PIN
  #define MISO_PIN AVR_MISO_PIN
#endif
#ifndef MOSI_PIN
  #define MOSI_PIN AVR_MOSI_PIN
#endif
#ifndef SS_PIN
  #define SS_PIN   AVR_SS_PIN
#endif

#endif //__PINS_H
