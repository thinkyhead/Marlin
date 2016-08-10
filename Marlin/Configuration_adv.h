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
 * Configuration_adv.h
 *
 * Advanced settings.
 * Only change these if you know exactly what you're doing.
 * Some of these settings can damage your printer if improperly set!
 *
 * Basic settings can be found in Configuration.h
 *
 */
#ifndef CONFIGURATION_ADV_H
#define CONFIGURATION_ADV_H

/**
 *
 *  ***********************************
 *  **  ATTENTION TO ALL DEVELOPERS  **
 *  ***********************************
 *
 * You must increment this version number for every significant change such as,
 * but not limited to: ADD, DELETE RENAME OR REPURPOSE any directive/option.
 *
 * Note: Update also Version.h !
 */
#define CONFIGURATION_ADV_H_VERSION 010100

// @section extruder

//This is for controlling a fan to cool down the stepper drivers
//it will turn on when any driver is enabled
//and turn off after the set amount of seconds from last driver being disabled again
#define CONTROLLERFAN_PIN -1 //Pin used for the fan to cool controller (-1 to disable)
#define CONTROLLERFAN_SECS 60 //How many seconds, after all motors were disabled, the fan should run
#define CONTROLLERFAN_SPEED 255  // == full speed

//===========================================================================
//============================ Mechanical Settings ==========================
//===========================================================================

// @section homing

// If you want endstops to stay on (by default) even when not homing
// enable this option. Override at any time with M120, M121.
//#define ENDSTOPS_ALWAYS_ON_DEFAULT
// @section homing

//homing hits the endstop, then retracts by this distance, before it tries to slowly bump again:
#define Z_HOME_BUMP_MM 2
#define HOMING_BUMP_DIVISOR 2  // Re-Bump Speed Divisor (Divides the Homing Feedrate)

// @section machine

#define AXIS_RELATIVE_MODES {false, false, false, false}

// By default pololu step drivers require an active high signal. However, some high power drivers require an active low signal as step.
#define INVERT_A_STEP_PIN false
#define INVERT_B_STEP_PIN false
#define INVERT_C_STEP_PIN false
#define INVERT_D_STEP_PIN false

// Default stepper release if idle. Set to 0 to deactivate.
// Steppers will shut down DEFAULT_STEPPER_DEACTIVE_TIME seconds after the last move when DISABLE_INACTIVE_? is true.
// Time can be set by M18 and M84.
#define DEFAULT_STEPPER_DEACTIVE_TIME 120
#define DISABLE_INACTIVE_A true
#define DISABLE_INACTIVE_B true
#define DISABLE_INACTIVE_C true  // set to false if the nozzle will fall down on your printed part when print has finished.
#define DISABLE_INACTIVE_D true

#define DEFAULT_MINIMUMFEEDRATE       0.0     // minimum feedrate
#define DEFAULT_MINTRAVELFEEDRATE     0.0

// @section extras

// minimum time in microseconds that a movement needs to take if the buffer is emptied.
#define DEFAULT_MINSEGMENTTIME        20000

// If defined the movements slow down when the look ahead buffer is only half full
//#define SLOWDOWN

// Frequency limit
// See nophead's blog for more info
// Not working O
//#define XY_FREQUENCY_LIMIT  15

// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
// of the buffer and all stops. This should not be much greater than zero and should only be changed
// if unwanted behavior is observed on a user's machine when running at very slow speeds.
#define MINIMUM_PLANNER_SPEED 0.05// (mm/sec)

// Microstep setting (Only functional when stepper driver microstep pins are connected to MCU.
#define MICROSTEP_MODES {16,16,16,16,16} // [1,2,4,8,16]

// Motor Current setting (Only functional when motor driver current ref pins are connected to a digital trimpot on supported boards)
#define DIGIPOT_MOTOR_CURRENT {135,135,135,135,135} // Values 0-255 (RAMBO 135 = ~0.75A, 185 = ~1A)

// Motor Current controlled via PWM (Overridable on supported boards with PWM-driven motor driver current)
//#define PWM_MOTOR_CURRENT {1300, 1300, 1250} // Values in milliamps

// uncomment to enable an I2C based DIGIPOT like on the Azteeg X3 Pro
//#define DIGIPOT_I2C
// Number of channels available for I2C digipot, For Azteeg X3 Pro we have 8
#define DIGIPOT_I2C_NUM_CHANNELS 8
// actual motor currents in Amps, need as many here as DIGIPOT_I2C_NUM_CHANNELS
#define DIGIPOT_I2C_MOTOR_CURRENTS {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}

//===========================================================================
//=============================Additional Features===========================
//===========================================================================

const unsigned int dropsegments = 5; //everything with less than this number of steps will be ignored as move and joined with the next movement

//===========================================================================
//================================= Buffers =================================
//===========================================================================

#define BLOCK_BUFFER_SIZE 16

// @section serial

// The ASCII buffer for serial input
#define MAX_CMD_SIZE 96
#define BUFSIZE 4

/******************************************************************************\
 * enable this section if you have TMC26X motor drivers.
 * you need to import the TMC26XStepper library into the Arduino IDE for this
 ******************************************************************************/

// @section tmc

//#define HAVE_TMCDRIVER
#if ENABLED(HAVE_TMCDRIVER)

  //#define A_IS_TMC
  #define A_MAX_CURRENT 1000  //in mA
  #define A_SENSE_RESISTOR 91 //in mOhms
  #define A_MICROSTEPS 16     //number of microsteps

  //#define B_IS_TMC
  #define B_MAX_CURRENT 1000  //in mA
  #define B_SENSE_RESISTOR 91 //in mOhms
  #define B_MICROSTEPS 16     //number of microsteps

  //#define C_IS_TMC
  #define C_MAX_CURRENT 1000  //in mA
  #define C_SENSE_RESISTOR 91 //in mOhms
  #define C_MICROSTEPS 16     //number of microsteps

  //#define D_IS_TMC
  #define D_MAX_CURRENT 1000  //in mA
  #define D_SENSE_RESISTOR 91 //in mOhms
  #define D_MICROSTEPS 16     //number of microsteps

#endif

/******************************************************************************\
 * enable this section if you have L6470  motor drivers.
 * you need to import the L6470 library into the Arduino IDE for this
 ******************************************************************************/

// @section l6470

//#define HAVE_L6470DRIVER
#if ENABLED(HAVE_L6470DRIVER)

  //#define A_IS_L6470
  #define A_MICROSTEPS 16     //number of microsteps
  #define A_K_VAL 50          // 0 - 255, Higher values, are higher power. Be careful not to go too high
  #define A_OVERCURRENT 2000  //maxc current in mA. If the current goes over this value, the driver will switch off
  #define A_STALLCURRENT 1500 //current in mA where the driver will detect a stall

  //#define B_IS_L6470
  #define B_MICROSTEPS 16     //number of microsteps
  #define B_K_VAL 50          // 0 - 255, Higher values, are higher power. Be careful not to go too high
  #define B_OVERCURRENT 2000  //maxc current in mA. If the current goes over this value, the driver will switch off
  #define B_STALLCURRENT 1500 //current in mA where the driver will detect a stall

  //#define C_IS_L6470
  #define C_MICROSTEPS 16     //number of microsteps
  #define C_K_VAL 50          // 0 - 255, Higher values, are higher power. Be careful not to go too high
  #define C_OVERCURRENT 2000  //maxc current in mA. If the current goes over this value, the driver will switch off
  #define C_STALLCURRENT 1500 //current in mA where the driver will detect a stall

  //#define D_IS_L6470
  #define D_MICROSTEPS 16     //number of microsteps
  #define D_K_VAL 50          // 0 - 255, Higher values, are higher power. Be careful not to go too high
  #define D_OVERCURRENT 2000  //maxc current in mA. If the current goes over this value, the driver will switch off
  #define D_STALLCURRENT 1500 //current in mA where the driver will detect a stall

#endif

/**
 * TWI/I2C BUS
 *
 * This feature is an EXPERIMENTAL feature so it shall not be used on production
 * machines. Enabling this will allow you to send and receive I2C data from slave
 * devices on the bus.
 *
 * ; Example #1
 * ; This macro send the string "Marlin" to the slave device with address 0x63 (99)
 * ; It uses multiple M155 commands with one B<base 10> arg
 * M155 A99  ; Target slave address
 * M155 B77  ; M
 * M155 B97  ; a
 * M155 B114 ; r
 * M155 B108 ; l
 * M155 B105 ; i
 * M155 B110 ; n
 * M155 S1   ; Send the current buffer
 *
 * ; Example #2
 * ; Request 6 bytes from slave device with address 0x63 (99)
 * M156 A99 B5
 *
 * ; Example #3
 * ; Example serial output of a M156 request
 * echo:i2c-reply: from:99 bytes:5 data:hello
 */

// @section i2cbus

#define EXPERIMENTAL_I2CBUS
#define I2C_SLAVE_ADDRESS  10 // Set an address ID from 8 to 127 to act as a slave

#endif // CONFIGURATION_ADV_H
