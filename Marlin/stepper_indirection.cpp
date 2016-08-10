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
  stepper_indirection.c - stepper motor driver indirection
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

#include "stepper_indirection.h"

#include "MarlinConfig.h"

#if ENABLED(HAVE_TMCDRIVER)
  #include <SPI.h>
  #include <TMC26XStepper.h>
#endif

// Stepper objects of TMC steppers used
#if ENABLED(A_IS_TMC)
  TMC26XStepper stepperA(200, A_ENABLE_PIN, A_STEP_PIN, A_DIR_PIN, A_MAX_CURRENT, A_SENSE_RESISTOR);
#endif
#if ENABLED(B_IS_TMC)
  TMC26XStepper stepperB(200, B_ENABLE_PIN, B_STEP_PIN, B_DIR_PIN, B_MAX_CURRENT, B_SENSE_RESISTOR);
#endif
#if ENABLED(C_IS_TMC)
  TMC26XStepper stepperC(200, C_ENABLE_PIN, C_STEP_PIN, C_DIR_PIN, C_MAX_CURRENT, C_SENSE_RESISTOR);
#endif
#if ENABLED(D_IS_TMC)
  TMC26XStepper stepperD(200, D_ENABLE_PIN, D_STEP_PIN, D_DIR_PIN, D_MAX_CURRENT, D_SENSE_RESISTOR);
#endif

#if ENABLED(HAVE_TMCDRIVER)
void tmc_init() {
  #if ENABLED(A_IS_TMC)
    stepperA.setMicrosteps(A_MICROSTEPS);
    stepperA.start();
  #endif
  #if ENABLED(B_IS_TMC)
    stepperB.setMicrosteps(B_MICROSTEPS);
    stepperB.start();
  #endif
  #if ENABLED(C_IS_TMC)
    stepperC.setMicrosteps(C_MICROSTEPS);
    stepperC.start();
  #endif
  #if ENABLED(D_IS_TMC)
    stepperD.setMicrosteps(D_MICROSTEPS);
    stepperD.start();
  #endif
}
#endif

// L6470 Driver objects and inits

#if ENABLED(HAVE_L6470DRIVER)
  #include <SPI.h>
  #include <L6470.h>
#endif

// L6470 Stepper objects
#if ENABLED(A_IS_L6470)
  L6470 stepperA(A_ENABLE_PIN);
#endif
#if ENABLED(B_IS_L6470)
  L6470 stepperB(B_ENABLE_PIN);
#endif
#if ENABLED(C_IS_L6470)
  L6470 stepperC(C_ENABLE_PIN);
#endif
#if ENABLED(D_IS_L6470)
  L6470 stepperD(D_ENABLE_PIN);
#endif


// init routine
#if ENABLED(HAVE_L6470DRIVER)
void L6470_init() {
  #if ENABLED(A_IS_L6470)
    stepperA.init(A_K_VAL);
    stepperA.softFree();
    stepperA.setMicroSteps(A_MICROSTEPS);
    stepperA.setOverCurrent(A_OVERCURRENT); //set overcurrent protection
    stepperA.setStallCurrent(A_STALLCURRENT);
  #endif
  #if ENABLED(B_IS_L6470)
    stepperB.init(B_K_VAL);
    stepperB.softFree();
    stepperB.setMicroSteps(B_MICROSTEPS);
    stepperB.setOverCurrent(B_OVERCURRENT); //set overcurrent protection
    stepperB.setStallCurrent(B_STALLCURRENT);
  #endif
  #if ENABLED(C_IS_L6470)
    stepperC.init(C_K_VAL);
    stepperC.softFree();
    stepperC.setMicroSteps(C_MICROSTEPS);
    stepperC.setOverCurrent(C_OVERCURRENT); //set overcurrent protection
    stepperC.setStallCurrent(C_STALLCURRENT);
  #endif
  #if ENABLED(D_IS_L6470)
    stepperD.init(D_K_VAL);
    stepperD.softFree();
    stepperD.setMicroSteps(D_MICROSTEPS);
    stepperD.setOverCurrent(D_OVERCURRENT); //set overcurrent protection
    stepperD.setStallCurrent(D_STALLCURRENT);
  #endif
}
#endif

