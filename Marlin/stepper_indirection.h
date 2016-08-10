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

/**
  stepper_indirection.h - stepper motor driver indirection macros
  to allow some stepper functions to be done via SPI/I2c instead of direct pin manipulation
  Part of Marlin

  Copyright (c) 2015 Dominik Wenger

  Marlin is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Marlin is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Marlin.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STEPPER_INDIRECTION_H
#define STEPPER_INDIRECTION_H

#include "MarlinConfig.h"

// TMC26X drivers have STEP/DIR on normal pins, but ENABLE via SPI
#if ENABLED(HAVE_TMCDRIVER)
  #include <SPI.h>
  #include <TMC26XStepper.h>
  void tmc_init();
#endif

// L6470 has STEP on normal pins, but DIR/ENABLE via SPI
#if ENABLED(HAVE_L6470DRIVER)
  #include <SPI.h>
  #include <L6470.h>
  void L6470_init();
#endif

// X Stepper
#if ENABLED(HAVE_L6470DRIVER) && ENABLED(A_IS_L6470)
  extern L6470 stepperA;
  #define A_ENABLE_INIT NOOP
  #define A_ENABLE_WRITE(STATE) do{if(STATE) stepperA.Step_Clock(stepperA.getStatus() & STATUS_HIZ); else stepperA.softFree();}while(0)
  #define A_ENABLE_READ (stepperA.getStatus() & STATUS_HIZ)
  #define A_DIR_INIT NOOP
  #define A_DIR_WRITE(STATE) stepperA.Step_Clock(STATE)
  #define A_DIR_READ (stepperA.getStatus() & STATUS_DIR)
#else
  #if ENABLED(HAVE_TMCDRIVER) && ENABLED(X_IS_TMC)
    extern TMC26XStepper stepperA;
    #define A_ENABLE_INIT NOOP
    #define A_ENABLE_WRITE(STATE) stepperA.setEnabled(STATE)
    #define A_ENABLE_READ stepperA.isEnabled()
  #else
    #define A_ENABLE_INIT SET_OUTPUT(A_ENABLE_PIN)
    #define A_ENABLE_WRITE(STATE) WRITE(A_ENABLE_PIN,STATE)
    #define A_ENABLE_READ READ(A_ENABLE_PIN)
  #endif
  #define A_DIR_INIT SET_OUTPUT(A_DIR_PIN)
  #define A_DIR_WRITE(STATE) WRITE(A_DIR_PIN,STATE)
  #define A_DIR_READ READ(A_DIR_PIN)
#endif
#define A_STEP_INIT SET_OUTPUT(A_STEP_PIN)
#define A_STEP_WRITE(STATE) WRITE(A_STEP_PIN,STATE)
#define A_STEP_READ READ(A_STEP_PIN)

// Y Stepper
#if ENABLED(HAVE_L6470DRIVER) && ENABLED(B_IS_L6470)
  extern L6470 stepperB;
  #define B_ENABLE_INIT NOOP
  #define B_ENABLE_WRITE(STATE) do{if(STATE) stepperB.Step_Clock(stepperB.getStatus() & STATUS_HIZ); else stepperB.softFree();}while(0)
  #define B_ENABLE_READ (stepperB.getStatus() & STATUS_HIZ)
  #define B_DIR_INIT NOOP
  #define B_DIR_WRITE(STATE) stepperB.Step_Clock(STATE)
  #define B_DIR_READ (stepperB.getStatus() & STATUS_DIR)
#else
  #if ENABLED(HAVE_TMCDRIVER) && ENABLED(Y_IS_TMC)
    extern TMC26XStepper stepperB;
    #define B_ENABLE_INIT NOOP
    #define B_ENABLE_WRITE(STATE) stepperB.setEnabled(STATE)
    #define B_ENABLE_READ stepperB.isEnabled()
  #else
    #define B_ENABLE_INIT SET_OUTPUT(B_ENABLE_PIN)
    #define B_ENABLE_WRITE(STATE) WRITE(B_ENABLE_PIN,STATE)
    #define B_ENABLE_READ READ(B_ENABLE_PIN)
  #endif
  #define B_DIR_INIT SET_OUTPUT(B_DIR_PIN)
  #define B_DIR_WRITE(STATE) WRITE(B_DIR_PIN,STATE)
  #define B_DIR_READ READ(B_DIR_PIN)
#endif
#define B_STEP_INIT SET_OUTPUT(B_STEP_PIN)
#define B_STEP_WRITE(STATE) WRITE(B_STEP_PIN,STATE)
#define B_STEP_READ READ(B_STEP_PIN)

// Z Stepper
#if ENABLED(HAVE_L6470DRIVER) && ENABLED(C_IS_L6470)
  extern L6470 stepperC;
  #define C_ENABLE_INIT NOOP
  #define C_ENABLE_WRITE(STATE) do{if(STATE) stepperC.Step_Clock(stepperC.getStatus() & STATUS_HIZ); else stepperC.softFree();}while(0)
  #define C_ENABLE_READ (stepperC.getStatus() & STATUS_HIZ)
  #define C_DIR_INIT NOOP
  #define C_DIR_WRITE(STATE) stepperC.Step_Clock(STATE)
  #define C_DIR_READ (stepperC.getStatus() & STATUS_DIR)
#else
  #if ENABLED(HAVE_TMCDRIVER) && ENABLED(Z_IS_TMC)
    extern TMC26XStepper stepperC;
    #define C_ENABLE_INIT NOOP
    #define C_ENABLE_WRITE(STATE) stepperC.setEnabled(STATE)
    #define C_ENABLE_READ stepperC.isEnabled()
  #else
    #define C_ENABLE_INIT SET_OUTPUT(C_ENABLE_PIN)
    #define C_ENABLE_WRITE(STATE) WRITE(C_ENABLE_PIN,STATE)
    #define C_ENABLE_READ READ(C_ENABLE_PIN)
  #endif
  #define C_DIR_INIT SET_OUTPUT(C_DIR_PIN)
  #define C_DIR_WRITE(STATE) WRITE(C_DIR_PIN,STATE)
  #define C_DIR_READ READ(C_DIR_PIN)
#endif
#define C_STEP_INIT SET_OUTPUT(C_STEP_PIN)
#define C_STEP_WRITE(STATE) WRITE(C_STEP_PIN,STATE)
#define C_STEP_READ READ(C_STEP_PIN)

// D Stepper
#if ENABLED(HAVE_L6470DRIVER) && ENABLED(D_IS_L6470)
  extern L6470 stepperD;
  #define D_ENABLE_INIT NOOP
  #define D_ENABLE_WRITE(STATE) do{if(STATE) stepperD.Step_Clock(stepperD.getStatus() & STATUS_HIZ); else stepperD.softFree();}while(0)
  #define D_ENABLE_READ (stepperD.getStatus() & STATUS_HIZ)
  #define D_DIR_INIT NOOP
  #define D_DIR_WRITE(STATE) stepperD.Step_Clock(STATE)
  #define D_DIR_READ (stepperD.getStatus() & STATUS_DIR)
#else
  #if ENABLED(HAVE_TMCDRIVER) && ENABLED(D_IS_TMC)
    extern TMC26XStepper stepperD;
    #define D_ENABLE_INIT NOOP
    #define D_ENABLE_WRITE(STATE) stepperD.setEnabled(STATE)
    #define D_ENABLE_READ stepperD.isEnabled()
  #else
    #define D_ENABLE_INIT SET_OUTPUT(D_ENABLE_PIN)
    #define D_ENABLE_WRITE(STATE) WRITE(D_ENABLE_PIN,STATE)
    #define D_ENABLE_READ READ(D_ENABLE_PIN)
  #endif
  #define D_DIR_INIT SET_OUTPUT(D_DIR_PIN)
  #define D_DIR_WRITE(STATE) WRITE(D_DIR_PIN,STATE)
  #define D_DIR_READ READ(D_DIR_PIN)
#endif
#define D_STEP_INIT SET_OUTPUT(D_STEP_PIN)
#define D_STEP_WRITE(STATE) WRITE(D_STEP_PIN,STATE)
#define D_STEP_READ READ(D_STEP_PIN)

/**
 * Extruder indirection for the single E axis
 */
#define E_STEP_WRITE(v) D_STEP_WRITE(v)
#define NORM_E_DIR() D_DIR_WRITE(!INVERT_D_DIR)
#define REV_E_DIR() D_DIR_WRITE(INVERT_D_DIR)

#endif // STEPPER_INDIRECTION_H
