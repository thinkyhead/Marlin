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

/**
 * Duet 2 Maestro
 */

#include "env_validate.h"

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "Duet 2 Maestro"
#endif

//
// Servos
//
//#define SERVO0_PIN                          -1
//#define SERVO1_PIN                          -1

//
// Limit Switches
//
#define X_STOP_PIN                            22  // PB26
#define Y_STOP_PIN                            18  // PA11
#define Z_STOP_PIN                            19  // PA10

//
// Z Probe (when not Z_MIN_PIN)
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN                     22
#endif

//
// Steppers
//
#define X_STEP_PIN                            66  // PB15      MOTOR 2
#define X_DIR_PIN                             54  // PA16
#define X_ENABLE_PIN                          67  // PB16

#define Y_STEP_PIN                            34  // PA29      MOTOR 3
#define Y_DIR_PIN                             35  // PB1
#define Y_ENABLE_PIN                          36  // PB0

#define Z_STEP_PIN                            73  // PA21      MOTOR 1
#define Z_DIR_PIN                             75  // PA26
#define Z_ENABLE_PIN                          74  // PA25

#define E0_STEP_PIN                           53  // PB14      MOTOR 4
#define E0_DIR_PIN                            78  // PB23
#define E0_ENABLE_PIN                         37  // PB22

// Microstepping mode pins
#define Z_MS1_PIN                             52  // PB21 MODE0     MOTOR 1
#define Z_MS2_PIN                             52  // PB21 MODE1
#define Z_MS3_PIN                             65  // PB20 MODE2

#define X_MS1_PIN                             43  // PA20 MODE0     MOTOR 2
#define X_MS2_PIN                             43  // PA20 MODE1
#define X_MS3_PIN                             42  // PA19 MODE2

#define Y_MS1_PIN                             77  // PA28 MODE0     MOTOR 3
#define Y_MS2_PIN                             77  // PA28 MODE1
#define Y_MS3_PIN                             76  // PA27 MODE2

#define E0_MS1_PIN                            38  // PB11 MODE0     MOTOR 4
#define E0_MS2_PIN                            38  // PB11 MODE1
#define E0_MS3_PIN                            39  // PB10 MODE2

// Motor current PWM pins
#define MOTOR_CURRENT_PWM_X_PIN               62  // PB17        MOTOR 1
#define MOTOR_CURRENT_PWM_Z_PIN               63  // PB18        MOTOR 2
#define MOTOR_CURRENT_PWM_Y_PIN               64  // PB19        MOTOR 3
#define MOTOR_CURRENT_PWM_E_PIN               61  // PA2         MOTOR 4

#define DEFAULT_PWM_MOTOR_CURRENT { 300, 400, 1000}  // XY Z E0, 1000 = 1000mAh

//
// Temperature Sensors
//
#define TEMP_0_PIN                             2  // D56 PA23
#define TEMP_BED_PIN                           5  // D59 PA4

//
// Heaters / Fans
//
#define HEATER_0_PIN                          40  // PA5
#define HEATER_BED_PIN                        41  // PB24

#ifndef FAN_PIN
  #define FAN_PIN                             13  // PB27 Fan1A
#endif
#define FAN1_PIN                              58  // PA6  Fan1B
#define FAN2_PIN                               4  // PC29

#define FET_SAFETY_PIN                        31  // PA7  must be pulsed low every 50 mS or FETs are turned off
#define FET_SAFETY_DELAY                      50  // 50 mS delay between pulses
#define FET_SAFETY_INVERTED                 true  // true - negative going pulse of 2 uS

/////////////////////////////////////////////////////////

#define SD_MISO_PIN                           68  // set to unused pins for now
#define SD_MOSI_PIN                           69  // set to unused pins for now
#define SD_SCK_PIN                            70  // set to unused pins for now
#define SDSS                                  71  // set to unused pins for now

#if HAS_TMC_UART

  /**
   * Address for the UART Configuration of the TMC2209. Override in Configuration files.
   * To test TMC2209 Steppers enable TMC_DEBUG in Configuration_adv.h and test the M122 command with voltage on the steppers.
   */
  #ifndef X_SLAVE_ADDRESS
    #define X_SLAVE_ADDRESS                 0b00
  #endif
  #ifndef Y_SLAVE_ADDRESS
    #define Y_SLAVE_ADDRESS                 0b01
  #endif
  #ifndef Z_SLAVE_ADDRESS
    #define Z_SLAVE_ADDRESS                 0b10
  #endif
  #ifndef E0_SLAVE_ADDRESS
    #define E0_SLAVE_ADDRESS                0b11
  #endif
  #ifndef E1_SLAVE_ADDRESS
    #define E1_SLAVE_ADDRESS                0b00
  #endif

  /**
   * TMC2208/TMC2209 stepper drivers
   *  It seems to work perfectly fine on Software Serial, if an advanced user wants to test, you could use the SAMD51 Serial1 and Serial 2. Be careful with the Sercom configurations.
   *  Steppers 1,2,3,4 (X,Y,Z,E0) are on the Serial1, Sercom (RX = 0, TX = 1), extra stepper 5 (E1 or any axis you want) is on Serial2, Sercom (RX = 17, TX = 16)
   */

  //#define X_HARDWARE_SERIAL  Serial1
  //#define Y_HARDWARE_SERIAL  Serial1
  //#define Z_HARDWARE_SERIAL  Serial1
  //#define E0_HARDWARE_SERIAL Serial1
  //#define E1_HARDWARE_SERIAL Serial2

  #define TMC_BAUD_RATE 250000

  //
  // Software serial
  //
  #define X_SERIAL_TX_PIN                      0
  #define X_SERIAL_RX_PIN                      1

  #define Y_SERIAL_TX_PIN        X_SERIAL_TX_PIN
  #define Y_SERIAL_RX_PIN        X_SERIAL_RX_PIN

  #define Z_SERIAL_TX_PIN        X_SERIAL_TX_PIN
  #define Z_SERIAL_RX_PIN        X_SERIAL_RX_PIN

  #define E0_SERIAL_TX_PIN       X_SERIAL_TX_PIN
  #define E0_SERIAL_RX_PIN       X_SERIAL_RX_PIN

  #define E1_SERIAL_TX_PIN                    17
  #define E1_SERIAL_RX_PIN                    16

#endif
