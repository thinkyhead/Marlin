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

////////////////////////////
// VENDOR VERSION EXAMPLE //
////////////////////////////

/**
 * Marlin release version identifier
 *
 * Notes:
 *   N2-1.3-STOCK  = 1.3 Board + Stock Neptune 2 (Single fan + Manual Bed Level)
 *   N2-1.3-BLT    = 1.3 Board + BLTouch + Single Stock Fan
 *   N2-1.3-MBL-DF = 1.3 Board + Manual Bed Level + Dual Fan
 *   N2-1.3-BLT-DF = 1.3 Board + BLTouch + Dual Fan
 *   N2-1.3-ST-LA  = 1.3 Board + Stock Neptune 2 + Linear Advance
 * TODO:
 *   N2-1.2-STOCK  = 1.2 Board + Stock Neptune 2 (Single fan + Manual Bed Level)
 *   N2-1.2-BLT    = 1.2 Board + BLTouch + Single Stock Fan
 *   N2-1.2-MBL-DF = 1.2 Board + Manual Bed Level + Dual Fan
 *   N2-1.2-BLT-DF = 1.2 Board + BLTouch + Dual Fan
 */
#define SHORT_BUILD_VERSION "2.0.9.3-N2-1.3-BLT-DF"

/**
 * Verbose version identifier which should contain a reference to the location
 * from where the binary was downloaded or the source code was compiled.
 */
//#define DETAILED_BUILD_VERSION SHORT_BUILD_VERSION

/**
 * The STRING_DISTRIBUTION_DATE represents when the binary file was built,
 * here we define this default string as the date where the latest release
 * version was tagged.
 */
#define STRING_DISTRIBUTION_DATE "2021-12-29"

/**
 * Defines a generic printer name to be output to the LCD after booting Marlin.
 */
#define MACHINE_NAME "Elegoo Neptune 2"

/**
 * The SOURCE_CODE_URL is the location where users will find the Marlin Source
 * Code which is installed on the device. In most cases —unless the manufacturer
 * has a distinct Github fork— the Source Code URL should just be the main
 * Marlin repository.
 */
#define SOURCE_CODE_URL "github.com/just-trey/Marlin/tree/N2-COLOR-UI"

/**
 * Default generic printer UUID.
 */
//#define DEFAULT_MACHINE_UUID "cede2a2f-41a2-4748-9b12-c55c62f367ff"

/**
 * The WEBSITE_URL is the location where users can get more information such as
 * documentation about a specific Marlin release.
 */
//#define WEBSITE_URL "marlinfw.org"

/**
 * Set the vendor info the serial USB interface, if changable
 * Currently only supported by DUE platform
 */
//#define USB_DEVICE_VENDOR_ID           0x0000
//#define USB_DEVICE_PRODUCT_ID          0x0000
//#define USB_DEVICE_MANUFACTURE_NAME    WEBSITE_URL
