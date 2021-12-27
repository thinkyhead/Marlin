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
 * Configuration.h
 *
 * Basic settings such as:
 *
 * - Type of electronics
 * - Type of temperature sensor
 * - Printer geometry
 * - Endstop configuration
 * - LCD controller
 * - Extra features
 *
 * Advanced settings can be found in Configuration_adv.h
 */
#define CONFIGURATION_H_VERSION 02000903

//===========================================================================
//============================= Getting Started =============================
//===========================================================================

/**
 * Here are some useful links to help get your machine configured and calibrated:
 *
 * Example Configs:     https://github.com/MarlinFirmware/Configurations/branches/all
 *
 * Průša Calculator:    https://blog.prusaprinters.org/calculator_3416/
 *
 * Calibration Guides:  https://reprap.org/wiki/Calibration
 *                      https://reprap.org/wiki/Triffid_Hunter%27s_Calibration_Guide
 *                      https://sites.google.com/site/repraplogphase/calibration-of-your-reprap
 *                      https://youtu.be/wAL9d7FgInk
 *
 * Calibration Objects: https://www.thingiverse.com/thing:5573
 *                      https://www.thingiverse.com/thing:1278865
 */

//===========================================================================
//========================== DELTA / SCARA / TPARA ==========================
//===========================================================================
//
// Download configurations from the link above and customize for your machine.
// Examples are located in config/examples/delta, .../SCARA, and .../TPARA.
//
//===========================================================================

// @section info

// Author info of this build printed to the host during boot and M115
#define STRING_CONFIG_H_AUTHOR "(tw, Neptune 2 BLTouch)" // Who made the changes.
//#define CUSTOM_VERSION_FILE Version.h // Path from the root directory (no quotes)

/**
 * *** VENDORS PLEASE READ ***
 *
 * Marlin allows you to add a custom boot image for Graphical LCDs.
 * With this option Marlin will first show your custom screen followed
 * by the standard Marlin logo with version number and web URL.
 *
 * We encourage you to take advantage of this new feature and we also
 * respectfully request that you retain the unmodified Marlin boot screen.
 */

// Show the Marlin bootscreen on startup. ** ENABLE FOR PRODUCTION **
#define SHOW_BOOTSCREEN

// Show the bitmap in Marlin/_Bootscreen.h on startup.
//#define SHOW_CUSTOM_BOOTSCREEN

// Show the bitmap in Marlin/_Statusscreen.h on the status screen.
//#define CUSTOM_STATUS_SCREEN_IMAGE

// @section machine

// Choose the name from boards.h that matches your setup
#ifndef MOTHERBOARD
  #define MOTHERBOARD BOARD_MKS_ROBIN_NANO_V1_3_F4
#endif

/**
 * Select the serial port on the board to use for communication with the host.
 * This allows the connection of wireless adapters (for instance) to non-default port pins.
 * Serial port -1 is the USB emulated serial port, if available.
 * Note: The first serial port (-1 or 0) will always be used by the Arduino bootloader.
 *
 * :[-1, 0, 1, 2, 3, 4, 5, 6, 7]
 */
#define SERIAL_PORT 3

/**
 * Serial Port Baud Rate
 * This is the default communication speed for all serial ports.
 * Set the baud rate defaults for additional serial ports below.
 *
 * 250000 works in most cases, but you might try a lower speed if
 * you commonly experience drop-outs during host printing.
 * You may try up to 1000000 to speed up SD file transfer.
 *
 * :[2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]
 */
#define BAUDRATE 250000
//#define BAUD_RATE_GCODE     // Enable G-code M575 to set the baud rate

/**
 * Select a secondary serial port on the board to use for communication with the host.
 * Currently Ethernet (-2) is only supported on Teensy 4.1 boards.
 * :[-2, -1, 0, 1, 2, 3, 4, 5, 6, 7]
 */
//#define SERIAL_PORT_2 -1
//#define BAUDRATE_2 250000   // Enable to override BAUDRATE

/**
 * Select a third serial port on the board to use for communication with the host.
 * Currently only supported for AVR, DUE, LPC1768/9 and STM32/STM32F1
 * :[-1, 0, 1, 2, 3, 4, 5, 6, 7]
 */
//#define SERIAL_PORT_3 1
//#define BAUDRATE_3 250000   // Enable to override BAUDRATE

// Enable the Bluetooth serial interface on AT90USB devices
//#define BLUETOOTH

// Name displayed in the LCD "Ready" message and Info menu
#define CUSTOM_MACHINE_NAME "Elegoo Neptune 2"

// Printer's unique ID, used by some programs to differentiate between machines.
// Choose your own or use a service like https://www.uuidgenerator.net/version4
//#define MACHINE_UUID "00000000-0000-0000-0000-000000000000"

/**
 * Define the number of coordinated linear axes.
 * See https://github.com/DerAndere1/Marlin/wiki
 * Each linear axis gets its own stepper control and endstop:
 *
 *   Steppers: *_STEP_PIN, *_ENABLE_PIN, *_DIR_PIN, *_ENABLE_ON
 *   Endstops: *_STOP_PIN, USE_*MIN_PLUG, USE_*MAX_PLUG
 *       Axes: *_MIN_POS, *_MAX_POS, INVERT_*_DIR
 *    Planner: DEFAULT_AXIS_STEPS_PER_UNIT, DEFAULT_MAX_FEEDRATE
 *             DEFAULT_MAX_ACCELERATION, AXIS_RELATIVE_MODES,
 *             MICROSTEP_MODES, MANUAL_FEEDRATE
 *
 * :[3, 4, 5, 6]
 */
//#define LINEAR_AXES 3

/**
 * Axis codes for additional axes:
 * This defines the axis code that is used in G-code commands to
 * reference a specific axis.
 * 'A' for rotational axis parallel to X
 * 'B' for rotational axis parallel to Y
 * 'C' for rotational axis parallel to Z
 * 'U' for secondary linear axis parallel to X
 * 'V' for secondary linear axis parallel to Y
 * 'W' for secondary linear axis parallel to Z
 * Regardless of the settings, firmware-internal axis IDs are
 * I (AXIS4), J (AXIS5), K (AXIS6).
 */
#if LINEAR_AXES >= 4
  #define AXIS4_NAME 'A' // :['A', 'B', 'C', 'U', 'V', 'W']
#endif
#if LINEAR_AXES >= 5
  #define AXIS5_NAME 'B' // :['A', 'B', 'C', 'U', 'V', 'W']
#endif
#if LINEAR_AXES >= 6
  #define AXIS6_NAME 'C' // :['A', 'B', 'C', 'U', 'V', 'W']
#endif

// @section extruder

// This defines the number of extruders
// :[0, 1, 2, 3, 4, 5, 6, 7, 8]
#define EXTRUDERS 1

// Generally expected filament diameter (1.75, 2.85, 3.0, ...). Used for Volumetric, Filament Width Sensor, etc.
#define DEFAULT_NOMINAL_FILAMENT_DIA 1.75

// For Cyclops or any "multi-extruder" that shares a single nozzle.
//#define SINGLENOZZLE

// Save and restore temperature and fan speed on tool-change.
// Set standby for the unselected tool with M104/106/109 T...
#if ENABLED(SINGLENOZZLE)
  //#define SINGLENOZZLE_STANDBY_TEMP
  //#define SINGLENOZZLE_STANDBY_FAN
#endif

/**
 * Multi-Material Unit
 * Set to one of these predefined models:
 *
 *   PRUSA_MMU1           : Průša MMU1 (The "multiplexer" version)
 *   PRUSA_MMU2           : Průša MMU2
 *   PRUSA_MMU2S          : Průša MMU2S (Requires MK3S extruder with motion sensor, EXTRUDERS = 5)
 *   EXTENDABLE_EMU_MMU2  : MMU with configurable number of filaments (ERCF, SMuFF or similar with Průša MMU2 compatible firmware)
 *   EXTENDABLE_EMU_MMU2S : MMUS with configurable number of filaments (ERCF, SMuFF or similar with Průša MMU2 compatible firmware)
 *
 * Requires NOZZLE_PARK_FEATURE to park print head in case MMU unit fails.
 * See additional options in Configuration_adv.h.
 */
//#define MMU_MODEL PRUSA_MMU2

// A dual extruder that uses a single stepper motor
//#define SWITCHING_EXTRUDER
#if ENABLED(SWITCHING_EXTRUDER)
  #define SWITCHING_EXTRUDER_SERVO_NR 0
  #define SWITCHING_EXTRUDER_SERVO_ANGLES { 0, 90 } // Angles for E0, E1[, E2, E3]
  #if EXTRUDERS > 3
    #define SWITCHING_EXTRUDER_E23_SERVO_NR 1
  #endif
#endif

// A dual-nozzle that uses a servomotor to raise/lower one (or both) of the nozzles
//#define SWITCHING_NOZZLE
#if ENABLED(SWITCHING_NOZZLE)
  #define SWITCHING_NOZZLE_SERVO_NR 0
  //#define SWITCHING_NOZZLE_E1_SERVO_NR 1          // If two servos are used, the index of the second
  #define SWITCHING_NOZZLE_SERVO_ANGLES { 0, 90 }   // Angles for E0, E1 (single servo) or lowered/raised (dual servo)
#endif

/**
 * Two separate X-carriages with extruders that connect to a moving part
 * via a solenoid docking mechanism. Requires SOL1_PIN and SOL2_PIN.
 */
//#define PARKING_EXTRUDER

/**
 * Two separate X-carriages with extruders that connect to a moving part
 * via a magnetic docking mechanism using movements and no solenoid
 *
 * project   : https://www.thingiverse.com/thing:3080893
 * movements : https://youtu.be/0xCEiG9VS3k
 *             https://youtu.be/Bqbcs0CU2FE
 */
//#define MAGNETIC_PARKING_EXTRUDER

#if EITHER(PARKING_EXTRUDER, MAGNETIC_PARKING_EXTRUDER)

  #define PARKING_EXTRUDER_PARKING_X { -78, 184 }     // X positions for parking the extruders
  #define PARKING_EXTRUDER_GRAB_DISTANCE 1            // (mm) Distance to move beyond the parking point to grab the extruder

  #if ENABLED(PARKING_EXTRUDER)

    #define PARKING_EXTRUDER_SOLENOIDS_INVERT           // If enabled, the solenoid is NOT magnetized with applied voltage
    #define PARKING_EXTRUDER_SOLENOIDS_PINS_ACTIVE LOW  // LOW or HIGH pin signal energizes the coil
    #define PARKING_EXTRUDER_SOLENOIDS_DELAY 250        // (ms) Delay for magnetic field. No delay if 0 or not defined.
    //#define MANUAL_SOLENOID_CONTROL                   // Manual control of docking solenoids with M380 S / M381

  #elif ENABLED(MAGNETIC_PARKING_EXTRUDER)

    #define MPE_FAST_SPEED      9000      // (mm/min) Speed for travel before last distance point
    #define MPE_SLOW_SPEED      4500      // (mm/min) Speed for last distance travel to park and couple
    #define MPE_TRAVEL_DISTANCE   10      // (mm) Last distance point
    #define MPE_COMPENSATION       0      // Offset Compensation -1 , 0 , 1 (multiplier) only for coupling

  #endif

#endif

/**
 * Switching Toolhead
 *
 * Support for swappable and dockable toolheads, such as
 * the E3D Tool Changer. Toolheads are locked with a servo.
 */
//#define SWITCHING_TOOLHEAD

/**
 * Magnetic Switching Toolhead
 *
 * Support swappable and dockable toolheads with a magnetic
 * docking mechanism using movement and no servo.
 */
//#define MAGNETIC_SWITCHING_TOOLHEAD

/**
 * Electromagnetic Switching Toolhead
 *
 * Parking for CoreXY / HBot kinematics.
 * Toolheads are parked at one edge and held with an electromagnet.
 * Supports more than 2 Toolheads. See https://youtu.be/JolbsAKTKf4
 */
//#define ELECTROMAGNETIC_SWITCHING_TOOLHEAD

#if ANY(SWITCHING_TOOLHEAD, MAGNETIC_SWITCHING_TOOLHEAD, ELECTROMAGNETIC_SWITCHING_TOOLHEAD)
  #define SWITCHING_TOOLHEAD_Y_POS          235         // (mm) Y position of the toolhead dock
  #define SWITCHING_TOOLHEAD_Y_SECURITY      10         // (mm) Security distance Y axis
  #define SWITCHING_TOOLHEAD_Y_CLEAR         60         // (mm) Minimum distance from dock for unobstructed X axis
  #define SWITCHING_TOOLHEAD_X_POS          { 215, 0 }  // (mm) X positions for parking the extruders
  #if ENABLED(SWITCHING_TOOLHEAD)
    #define SWITCHING_TOOLHEAD_SERVO_NR       2         // Index of the servo connector
    #define SWITCHING_TOOLHEAD_SERVO_ANGLES { 0, 180 }  // (degrees) Angles for Lock, Unlock
  #elif ENABLED(MAGNETIC_SWITCHING_TOOLHEAD)
    #define SWITCHING_TOOLHEAD_Y_RELEASE      5         // (mm) Security distance Y axis
    #define SWITCHING_TOOLHEAD_X_SECURITY   { 90, 150 } // (mm) Security distance X axis (T0,T1)
    //#define PRIME_BEFORE_REMOVE                       // Prime the nozzle before release from the dock
    #if ENABLED(PRIME_BEFORE_REMOVE)
      #define SWITCHING_TOOLHEAD_PRIME_MM           20  // (mm)   Extruder prime length
      #define SWITCHING_TOOLHEAD_RETRACT_MM         10  // (mm)   Retract after priming length
      #define SWITCHING_TOOLHEAD_PRIME_FEEDRATE    300  // (mm/min) Extruder prime feedrate
      #define SWITCHING_TOOLHEAD_RETRACT_FEEDRATE 2400  // (mm/min) Extruder retract feedrate
    #endif
  #elif ENABLED(ELECTROMAGNETIC_SWITCHING_TOOLHEAD)
    #define SWITCHING_TOOLHEAD_Z_HOP          2         // (mm) Z raise for switching
  #endif
#endif

/**
 * "Mixing Extruder"
 *   - Adds G-codes M163 and M164 to set and "commit" the current mix factors.
 *   - Extends the stepping routines to move multiple steppers in proportion to the mix.
 *   - Optional support for Repetier Firmware's 'M164 S<index>' supporting virtual tools.
 *   - This implementation supports up to two mixing extruders.
 *   - Enable DIRECT_MIXING_IN_G1 for M165 and mixing in G1 (from Pia Taubert's reference implementation).
 */
//#define MIXING_EXTRUDER
#if ENABLED(MIXING_EXTRUDER)
  #define MIXING_STEPPERS 2        // Number of steppers in your mixing extruder
  #define MIXING_VIRTUAL_TOOLS 16  // Use the Virtual Tool method with M163 and M164
  //#define DIRECT_MIXING_IN_G1    // Allow ABCDHI mix factors in G1 movement commands
  //#define GRADIENT_MIX           // Support for gradient mixing with M166 and LCD
  //#define MIXING_PRESETS         // Assign 8 default V-tool presets for 2 or 3 MIXING_STEPPERS
  #if ENABLED(GRADIENT_MIX)
    //#define GRADIENT_VTOOL       // Add M166 T to use a V-tool index as a Gradient alias
  #endif
#endif

// Offset of the extruders (uncomment if using more than one and relying on firmware to position when changing).
// The offset has to be X=0, Y=0 for the extruder 0 hotend (default extruder).
// For the other hotends it is their distance from the extruder 0 hotend.
//#define HOTEND_OFFSET_X { 0.0, 20.00 } // (mm) relative X-offset for each nozzle
//#define HOTEND_OFFSET_Y { 0.0, 5.00 }  // (mm) relative Y-offset for each nozzle
//#define HOTEND_OFFSET_Z { 0.0, 0.00 }  // (mm) relative Z-offset for each nozzle

// @section machine

/**
 * Power Supply Control
 *
 * Enable and connect the power supply to the PS_ON_PIN.
 * Specify whether the power supply is active HIGH or active LOW.
 */
//#define PSU_CONTROL
//#define PSU_NAME "Power Supply"

#if ENABLED(PSU_CONTROL)
  //#define MKS_PWC                 // Using the MKS PWC add-on
  //#define PS_OFF_CONFIRM          // Confirm dialog when power off
  //#define PS_OFF_SOUND            // Beep 1s when power off
  #define PSU_ACTIVE_STATE LOW      // Set 'LOW' for ATX, 'HIGH' for X-Box

  //#define PSU_DEFAULT_OFF         // Keep power off until enabled directly with M80
  //#define PSU_POWERUP_DELAY 250   // (ms) Delay for the PSU to warm up to full power

  //#define PSU_POWERUP_GCODE  "M355 S1"  // G-code to run after power-on (e.g., case light on)
  //#define PSU_POWEROFF_GCODE "M355 S0"  // G-code to run before power-off (e.g., case light off)

  //#define AUTO_POWER_CONTROL      // Enable automatic control of the PS_ON pin
  #if ENABLED(AUTO_POWER_CONTROL)
    #define AUTO_POWER_FANS         // Turn on PSU if fans need power
    #define AUTO_POWER_E_FANS
    #define AUTO_POWER_CONTROLLERFAN
    #define AUTO_POWER_CHAMBER_FAN
    #define AUTO_POWER_COOLER_FAN
    //#define AUTO_POWER_E_TEMP        50 // (°C) Turn on PSU if any extruder is over this temperature
    //#define AUTO_POWER_CHAMBER_TEMP  30 // (°C) Turn on PSU if the chamber is over this temperature
    //#define AUTO_POWER_COOLER_TEMP   26 // (°C) Turn on PSU if the cooler is over this temperature
    #define POWER_TIMEOUT              30 // (s) Turn off power if the machine is idle for this duration
    //#define POWER_OFF_DELAY          60 // (s) Delay of poweroff after M81 command. Useful to let fans run for extra time.
  #endif
#endif

//===========================================================================
//============================= Thermal Settings ============================
//===========================================================================
// @section temperature

/**
 * --NORMAL IS 4.7kΩ PULLUP!-- 1kΩ pullup can be used on hotend sensor, using correct resistor and table
 *
 * Temperature sensors available:
 *
 *  SPI RTD/Thermocouple Boards - IMPORTANT: Read the NOTE below!
 *  -------
 *    -5 : MAX31865 with Pt100/Pt1000, 2, 3, or 4-wire  (only for sensors 0-1)
 *                  NOTE: You must uncomment/set the MAX31865_*_OHMS_n defines below.
 *    -3 : MAX31855 with Thermocouple, -200°C to +700°C (only for sensors 0-1)
 *    -2 : MAX6675  with Thermocouple, 0°C to +700°C    (only for sensors 0-1)
 *
 *  NOTE: Ensure TEMP_n_CS_PIN is set in your pins file for each TEMP_SENSOR_n using an SPI Thermocouple. By default,
 *        Hardware SPI on the default serial bus is used. If you have also set TEMP_n_SCK_PIN and TEMP_n_MISO_PIN,
 *        Software SPI will be used on those ports instead. You can force Hardware SPI on the default bus in the
 *        Configuration_adv.h file. At this time, separate Hardware SPI buses for sensors are not supported.
 *
 *  Analog Themocouple Boards
 *  -------
 *    -4 : AD8495 with Thermocouple
 *    -1 : AD595  with Thermocouple
 *
 *  Analog Thermistors - 4.7kΩ pullup - Normal
 *  -------
 *     1 : 100kΩ  EPCOS - Best choice for EPCOS thermistors
 *   331 : 100kΩ  Same as #1, but 3.3V scaled for MEGA
 *   332 : 100kΩ  Same as #1, but 3.3V scaled for DUE
 *     2 : 200kΩ  ATC Semitec 204GT-2
 *   202 : 200kΩ  Copymaster 3D
 *     3 : ???Ω   Mendel-parts thermistor
 *     4 : 10kΩ   Generic Thermistor !! DO NOT use for a hotend - it gives bad resolution at high temp. !!
 *     5 : 100kΩ  ATC Semitec 104GT-2/104NT-4-R025H42G - Used in ParCan, J-Head, and E3D, SliceEngineering 300°C
 *   501 : 100kΩ  Zonestar - Tronxy X3A
 *   502 : 100kΩ  Zonestar - used by hot bed in Zonestar Průša P802M
 *   512 : 100kΩ  RPW-Ultra hotend
 *     6 : 100kΩ  EPCOS - Not as accurate as table #1 (created using a fluke thermocouple)
 *     7 : 100kΩ  Honeywell 135-104LAG-J01
 *    71 : 100kΩ  Honeywell 135-104LAF-J01
 *     8 : 100kΩ  Vishay 0603 SMD NTCS0603E3104FXT
 *     9 : 100kΩ  GE Sensing AL03006-58.2K-97-G1
 *    10 : 100kΩ  RS PRO 198-961
 *    11 : 100kΩ  Keenovo AC silicone mats, most Wanhao i3 machines - beta 3950, 1%
 *    12 : 100kΩ  Vishay 0603 SMD NTCS0603E3104FXT (#8) - calibrated for Makibox hot bed
 *    13 : 100kΩ  Hisens up to 300°C - for "Simple ONE" & "All In ONE" hotend - beta 3950, 1%
 *    15 : 100kΩ  Calibrated for JGAurora A5 hotend
 *    18 : 200kΩ  ATC Semitec 204GT-2 Dagoma.Fr - MKS_Base_DKU001327
 *    22 : 100kΩ  GTM32 Pro vB - hotend - 4.7kΩ pullup to 3.3V and 220Ω to analog input
 *    23 : 100kΩ  GTM32 Pro vB - bed - 4.7kΩ pullup to 3.3v and 220Ω to analog input
 *    30 : 100kΩ  Kis3d Silicone heating mat 200W/300W with 6mm precision cast plate (EN AW 5083) NTC100K - beta 3950
 *    60 : 100kΩ  Maker's Tool Works Kapton Bed Thermistor - beta 3950
 *    61 : 100kΩ  Formbot/Vivedino 350°C Thermistor - beta 3950
 *    66 : 4.7MΩ  Dyze Design High Temperature Thermistor
 *    67 : 500kΩ  SliceEngineering 450°C Thermistor
 *    70 : 100kΩ  bq Hephestos 2
 *    75 : 100kΩ  Generic Silicon Heat Pad with NTC100K MGB18-104F39050L32
 *  2000 : 100kΩ  Ultimachine Rambo TDK NTCG104LH104KT1 NTC100K motherboard Thermistor
 *
 *  Analog Thermistors - 1kΩ pullup - Atypical, and requires changing out the 4.7kΩ pullup for 1kΩ.
 *  -------                           (but gives greater accuracy and more stable PID)
 *    51 : 100kΩ  EPCOS (1kΩ pullup)
 *    52 : 200kΩ  ATC Semitec 204GT-2 (1kΩ pullup)
 *    55 : 100kΩ  ATC Semitec 104GT-2 - Used in ParCan & J-Head (1kΩ pullup)
 *
 *  Analog Thermistors - 10kΩ pullup - Atypical
 *  -------
 *    99 : 100kΩ  Found on some Wanhao i3 machines with a 10kΩ pull-up resistor
 *
 *  Analog RTDs (Pt100/Pt1000)
 *  -------
 *   110 : Pt100  with 1kΩ pullup (atypical)
 *   147 : Pt100  with 4.7kΩ pullup
 *  1010 : Pt1000 with 1kΩ pullup (atypical)
 *  1047 : Pt1000 with 4.7kΩ pullup (E3D)
 *    20 : Pt100  with circuit in the Ultimainboard V2.x with mainboard ADC reference voltage = INA826 amplifier-board supply voltage.
 *                NOTE: (1) Must use an ADC input with no pullup. (2) Some INA826 amplifiers are unreliable at 3.3V so consider using sensor 147, 110, or 21.
 *    21 : Pt100  with circuit in the Ultimainboard V2.x with 3.3v ADC reference voltage (STM32, LPC176x....) and 5V INA826 amplifier board supply.
 *                NOTE: ADC pins are not 5V tolerant. Not recommended because it's possible to damage the CPU by going over 500°C.
 *   201 : Pt100  with circuit in Overlord, similar to Ultimainboard V2.x
 *
 *  Custom/Dummy/Other Thermal Sensors
 *  ------
 *     0 : not used
 *  1000 : Custom - Specify parameters in Configuration_adv.h
 *
 *   !!! Use these for Testing or Development purposes. NEVER for production machine. !!!
 *   998 : Dummy Table that ALWAYS reads 25°C or the temperature defined below.
 *   999 : Dummy Table that ALWAYS reads 100°C or the temperature defined below.
 *
 */
#define TEMP_SENSOR_0 1
#define TEMP_SENSOR_1 0
#define TEMP_SENSOR_2 0
#define TEMP_SENSOR_3 0
#define TEMP_SENSOR_4 0
#define TEMP_SENSOR_5 0
#define TEMP_SENSOR_6 0
#define TEMP_SENSOR_7 0
#define TEMP_SENSOR_BED 1
#define TEMP_SENSOR_PROBE 0
#define TEMP_SENSOR_CHAMBER 0
#define TEMP_SENSOR_COOLER 0
#define TEMP_SENSOR_BOARD 0
#define TEMP_SENSOR_REDUNDANT 0

// Dummy thermistor constant temperature readings, for use with 998 and 999
#define DUMMY_THERMISTOR_998_VALUE  25
#define DUMMY_THERMISTOR_999_VALUE 100

// Resistor values when using MAX31865 sensors (-5) on TEMP_SENSOR_0 / 1
//#define MAX31865_SENSOR_OHMS_0      100   // (Ω) Typically 100 or 1000 (PT100 or PT1000)
//#define MAX31865_CALIBRATION_OHMS_0 430   // (Ω) Typically 430 for Adafruit PT100; 4300 for Adafruit PT1000
//#define MAX31865_SENSOR_OHMS_1      100
//#define MAX31865_CALIBRATION_OHMS_1 430

#define TEMP_RESIDENCY_TIME         10  // (seconds) Time to wait for hotend to "settle" in M109
#define TEMP_WINDOW                  1  // (°C) Temperature proximity for the "temperature reached" timer
#define TEMP_HYSTERESIS              3  // (°C) Temperature proximity considered "close enough" to the target

#define TEMP_BED_RESIDENCY_TIME     10  // (seconds) Time to wait for bed to "settle" in M190
#define TEMP_BED_WINDOW              1  // (°C) Temperature proximity for the "temperature reached" timer
#define TEMP_BED_HYSTERESIS          3  // (°C) Temperature proximity considered "close enough" to the target

#define TEMP_CHAMBER_RESIDENCY_TIME 10  // (seconds) Time to wait for chamber to "settle" in M191
#define TEMP_CHAMBER_WINDOW          1  // (°C) Temperature proximity for the "temperature reached" timer
#define TEMP_CHAMBER_HYSTERESIS      3  // (°C) Temperature proximity considered "close enough" to the target

/**
