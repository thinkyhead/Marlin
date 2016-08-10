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
 * SanityCheck.h
 *
 * Test configuration values for errors at compile-time.
 */

/**
 * Due to the high number of issues related with old versions of Arduino IDE
 * we now prevent Marlin from compiling with older toolkits.
 */
#if !defined(ARDUINO) || ARDUINO < 10600
  #error "Versions of Arduino IDE prior to 1.6.0 are no longer supported, please update your toolkit."
#endif

/**
 * We try our best to include sanity checks for all the changes configuration
 * directives because people have a tendency to use outdated config files with
 * the bleding edge source code, but sometimes this is not enough. This check
 * will force a minimum config file revision, otherwise Marlin will not build.
 */
#if ! defined(CONFIGURATION_H_VERSION) || CONFIGURATION_H_VERSION < REQUIRED_CONFIGURATION_H_VERSION
  #error "You are using an old Configuration.h file, update it before building Marlin."
#endif

#if ! defined(CONFIGURATION_ADV_H_VERSION) || CONFIGURATION_ADV_H_VERSION < REQUIRED_CONFIGURATION_ADV_H_VERSION
  #error "You are using an old Configuration_adv.h file, update it before building Marlin."
#endif

/**
 * Endstops
 */
#if DISABLED(USE_AMIN_PLUG) && DISABLED(USE_AMAX_PLUG)
 #error "You must enable USE_AMIN_PLUG or USE_AMAX_PLUG."
#elif DISABLED(USE_BMIN_PLUG) && DISABLED(USE_BMAX_PLUG)
 #error "You must enable USE_BMIN_PLUG or USE_BMAX_PLUG."
#elif DISABLED(USE_CMIN_PLUG) && DISABLED(USE_CMAX_PLUG)
 #error "You must enable USE_CMIN_PLUG or USE_CMAX_PLUG."
#elif DISABLED(USE_DMIN_PLUG) && DISABLED(USE_DMAX_PLUG)
 #error "You must enable USE_DMIN_PLUG or USE_DMAX_PLUG."
#endif
