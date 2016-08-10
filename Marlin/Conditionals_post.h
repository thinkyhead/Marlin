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
 * Conditionals_post.h
 * Defines that depend on configuration but are not editable.
 */

#ifndef CONDITIONALS_POST_H
#define CONDITIONALS_POST_H

  #if ENABLED(EMERGENCY_PARSER)
    #define EMERGENCY_PARSER_CAPABILITIES " EMERGENCY_CODES:M108,M112,M410"
  #else
    #define EMERGENCY_PARSER_CAPABILITIES ""
  #endif

  /**
   * Axis lengths
   */
  #define Z_MAX_LENGTH (Z_MAX_POS - (Z_MIN_POS))

  /**
   * Set the home position based on settings or manual overrides
   */
  #define Z_HOME_POS (Z_HOME_DIR < 0 ? Z_MIN_POS : Z_MAX_POS)

  /**
   * MAX_STEP_FREQUENCY differs for TOSHIBA
   */
  #if ENABLED(CONFIG_STEPPERS_TOSHIBA)
    #define MAX_STEP_FREQUENCY 10000 // Max step frequency for Toshiba Stepper Controllers
  #else
    #define MAX_STEP_FREQUENCY 40000 // Max step frequency for Ultimaker (5000 pps / half step)
  #endif

  // MS1 MS2 Stepper Driver Microstepping mode table
  #define MICROSTEP1 LOW,LOW
  #define MICROSTEP2 HIGH,LOW
  #define MICROSTEP4 LOW,HIGH
  #define MICROSTEP8 HIGH,HIGH
  #define MICROSTEP16 HIGH,HIGH

  /**
   * Set defaults for missing (newer) options
   */
  #ifndef DISABLE_INACTIVE_A
    #define DISABLE_INACTIVE_A DISABLE_A
  #endif
  #ifndef DISABLE_INACTIVE_B
    #define DISABLE_INACTIVE_B DISABLE_B
  #endif
  #ifndef DISABLE_INACTIVE_C
    #define DISABLE_INACTIVE_C DISABLE_C
  #endif
  #ifndef DISABLE_INACTIVE_D
    #define DISABLE_INACTIVE_D DISABLE_D
  #endif

  // Power Signal Control Definitions
  // By default use ATX definition
  #ifndef POWER_SUPPLY
    #define POWER_SUPPLY 1
  #endif
  #if (POWER_SUPPLY == 1)     // 1 = ATX
    #define PS_ON_AWAKE  LOW
    #define PS_ON_ASLEEP HIGH
  #elif (POWER_SUPPLY == 2)   // 2 = X-Box 360 203W
    #define PS_ON_AWAKE  HIGH
    #define PS_ON_ASLEEP LOW
  #endif
  #define HAS_POWER_SWITCH (POWER_SUPPLY > 0 && PIN_EXISTS(PS_ON))

  /**
   * Set ENDSTOPPULLUPS for active endstop switches
   */
  #if ENABLED(ENDSTOPPULLUPS)
    #if ENABLED(USE_AMAX_PLUG)
      #define ENDSTOPPULLUP_AMAX
    #endif
    #if ENABLED(USE_BMAX_PLUG)
      #define ENDSTOPPULLUP_BMAX
    #endif
    #if ENABLED(USE_CMAX_PLUG)
      #define ENDSTOPPULLUP_CMAX
    #endif
    #if ENABLED(USE_DMAX_PLUG)
      #define ENDSTOPPULLUP_DMAX
    #endif
    #if ENABLED(USE_AMIN_PLUG)
      #define ENDSTOPPULLUP_AMIN
    #endif
    #if ENABLED(USE_BMIN_PLUG)
      #define ENDSTOPPULLUP_BMIN
    #endif
    #if ENABLED(USE_CMIN_PLUG)
      #define ENDSTOPPULLUP_CMIN
    #endif
    #if ENABLED(USE_DMIN_PLUG)
      #define ENDSTOPPULLUP_DMIN
    #endif
  #endif

  /**
   * Shorthand for pin tests, used wherever needed
   */
  #define HAS_CONTROLLERFAN (PIN_EXISTS(CONTROLLERFAN))
  #define HAS_KILL (PIN_EXISTS(KILL))
  #define HAS_SUICIDE (PIN_EXISTS(SUICIDE))
  #define HAS_A_MIN (PIN_EXISTS(A_MIN))
  #define HAS_A_MAX (PIN_EXISTS(A_MAX))
  #define HAS_B_MIN (PIN_EXISTS(B_MIN))
  #define HAS_B_MAX (PIN_EXISTS(B_MAX))
  #define HAS_C_MIN (PIN_EXISTS(C_MIN))
  #define HAS_C_MAX (PIN_EXISTS(C_MAX))
  #define HAS_D_MIN (PIN_EXISTS(D_MIN))
  #define HAS_D_MAX (PIN_EXISTS(D_MAX))
  #define HAS_MICROSTEPS (PIN_EXISTS(A_MS1))
  #define HAS_A_ENABLE (PIN_EXISTS(A_ENABLE))
  #define HAS_B_ENABLE (PIN_EXISTS(B_ENABLE))
  #define HAS_C_ENABLE (PIN_EXISTS(C_ENABLE))
  #define HAS_D_ENABLE (PIN_EXISTS(D_ENABLE))
  #define HAS_A_DIR (PIN_EXISTS(A_DIR))
  #define HAS_B_DIR (PIN_EXISTS(B_DIR))
  #define HAS_C_DIR (PIN_EXISTS(C_DIR))
  #define HAS_D_DIR (PIN_EXISTS(D_DIR))
  #define HAS_A_STEP (PIN_EXISTS(A_STEP))
  #define HAS_B_STEP (PIN_EXISTS(B_STEP))
  #define HAS_C_STEP (PIN_EXISTS(C_STEP))
  #define HAS_D_STEP (PIN_EXISTS(D_STEP))
  #define HAS_DIGIPOTSS (PIN_EXISTS(DIGIPOTSS))
  #define HAS_BUZZER (PIN_EXISTS(BEEPER) || ENABLED(LCD_USE_I2C_BUZZER))

  #define HAS_MOTOR_CURRENT_PWM (PIN_EXISTS(MOTOR_CURRENT_PWM_XY) || PIN_EXISTS(MOTOR_CURRENT_PWM_Z) || PIN_EXISTS(MOTOR_CURRENT_PWM_E))

#endif // CONDITIONALS_POST_H
