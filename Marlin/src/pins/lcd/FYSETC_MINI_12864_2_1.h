/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2023 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
 * pins/lcd/FYSETC_MINI_12864_2_1.h
 */

// Same as MKS_MINI_12864_V3 and BTT_MINI_12864_V1 with plugs rotated 180 degrees

// U8GLIB_MINI12864_2X_HAL

//  FYSETC_MINI_12864
//  DOGLCD
//   HAS_MARLINUI_U8GLIB
//  IS_ULTIPANEL
//   HAS_WIRED_LCD
//    HAS_DISPLAY
//     HAS_STATUS_MESSAGE
//   IS_NEWPANEL
//   HAS_MARLINUI_MENU
//    HAS_MANUAL_MOVE_MENU
//  LED_COLORS_REDUCE_GREEN

/**
 *   FYSECTC Mini 12864 2.1  https://wiki.fysetc.com/Mini12864_Panel/
 *
 *
 *   EXP1                                     EXP2
 *
 *   9     7     5     3     1                9     7     5     3     1
 *   GND   NC    LRST  LCS   BEEP             GND   CD    EN2   EN1   MISO
 *   VCC   NC    DIN   LRS   ENC              SCK   RST   MOSI  SDSS  SCK
 *   10    8    (6)    4     2                10    8    (6)    4     2
 *
 */

#define BEEPER_PIN        LCD1_01_PIN
#define BTN_ENC           LCD1_02_PIN
#define DOGLCD_CS         LCD1_03_PIN
#define DOGLCD_A0         LCD1_04_PIN
#define LCD_RESET_PIN     LCD1_05_PIN
#define NEOPIXEL_PIN      LCD1_06_PIN
#define DOGLCD_MISO       LCD2_01_PIN
#define DOGLCD_SCK        LCD2_02_PIN
#define BTN_EN1           LCD2_03_PIN
#ifndef SDSS
  #define SDSS            LCD2_04_PIN
#endif
#define BTN_EN2           LCD2_05_PIN
#define DOGLCD_MOSI       LCD2_06_PIN
#ifndef SD_DETECT_PIN
  #define SD_DETECT_PIN   LCD2_07_PIN
#endif
#define KILL_PIN          LCD2_08_PIN
