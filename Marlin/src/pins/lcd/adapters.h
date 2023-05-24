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

#if ENABLED(LCD_ON_EXP3)

  // Remap "EXP3" to "LCD1" for displays that only have a single 10-pin connector

  #define LCD1_01_PIN EXP3_01_PIN
  #define LCD1_02_PIN EXP3_02_PIN
  #define LCD1_03_PIN EXP3_03_PIN
  #define LCD1_04_PIN EXP3_04_PIN
  #define LCD1_05_PIN EXP3_05_PIN
  #define LCD1_06_PIN EXP3_06_PIN
  #define LCD1_07_PIN EXP3_07_PIN
  #define LCD1_08_PIN EXP3_08_PIN

#elif ENABLED(ADAPTER_MELZI_CREALITY)

  /** Creality "Melzi" LCD single-to-dual adapter
   *                   ------
   *   LCD1-1 <= BEEP | 1  2 | ENC   => LCD1-2
   *   LCD2-3 <=  EN1 | 3  4 | RESET => LCD1-4
   *   LCD2-5 <=  EN2   5  6 | A0    => LCD1-7
   *   LCD1-6 <=   CS | 7  8 | MOSI  => LCD2-6
   *              GND | 9 10 | 5V
   *                   ------
   */
  #define LCD1_01_PIN EXP1_01_PIN  // BEEPER
  #define LCD1_02_PIN EXP1_02_PIN  // ENC
  #define LCD1_06_PIN EXP1_07_PIN  // CS / RS / ST9720 CS
  #define LCD1_07_PIN EXP1_06_PIN  // A0 / D4 / ST9720 CLK / SCK

  #define LCD2_03_PIN EXP1_03_PIN  // EN1
  #define LCD2_05_PIN EXP1_05_PIN  // EN2
  #define LCD2_06_PIN EXP1_08_PIN  //      EN / ST9720 DAT / MOSI

#elif ENABLED(ADAPTER_MINIPANEL_DUAL)

  // Map old-style MINIPANEL connectors to LCD1/2.
  // Contrast with Single-to-mini and Dual-to-mini adapters,
  // if such adapters exist for MINIPANEL.

  /**
   *     SDD   MOSI  SDSS       RESET       LCDSS  A0    KILL  ENC
   *     2-7   2-6   2-2                     1-6   1-1   2-3   2-5
   *    ----------------------------- -----------------------------
   *   |  2     4     6     8    10  | 12    14    16    18    20  |
   *   |  1     3     5     7     9  | 11    13    15    17    19  |
   *    ----------------------------- -----------------------------
   *                 2-2                     2-7   1-1   2-3   2-5
   *     5V    MISO  SCK   GND   3V3         BL    BEEP  EN1   EN2
   */

  #define LCD1_01_PIN MINI_15 // BEEP
  #define LCD1_02_PIN MINI_20 // ENC
  #define LCD1_06_PIN MINI_14 // LCDSS
  #define LCD1_07_PIN MINI_16 // A0
  #define LCD1_08_PIN MINI_13 // BACKLIGHT

  #define LCD2_02_PIN MINI_05 // SCK
  #define LCD2_03_PIN MINI_17 // EN1
  #define LCD2_05_PIN MINI_19 // EN2
  #define LCD2_06_PIN MINI_04 // MOSI
  #define LCD2_07_PIN MINI_02 // SD_DETECT
  #define LCD2_08_PIN MINI_18 // KILL

#elif ENABLED(ADAPTER_CREALITY_PIN27)

  // Creality or BTT "Pin 27" Adapter with BLTouch pins

#elif ANY(ADAPTER_CREALITY_DUAL, ADAPTER_BTT_DUAL)

  /** Common LCD single-to-dual adapter
   *                   ------
   *    LCD1-1 <= SCK | 1  2 | ENC   => LCD1-2
   *    LCD2-5 <= EN1 | 3  4 | RESET => LCD1-4
   *    LCD2-3 <= EN2   5  6 | A0    => LCD1-7
   *    LCD1-6 <=  CS | 7  8 | MOSI  => LCD2-6
   *              GND | 9 10 | 5V
   *                   ------
   */
  #define LCD1_02_PIN EXP1_02_PIN  // ENC
  #define LCD1_04_PIN EXP1_04_PIN  // RESET
  #define LCD1_06_PIN EXP1_07_PIN  // CS
  #define LCD1_07_PIN EXP1_06_PIN  // A0

  #define LCD2_02_PIN EXP1_01_PIN  // SCK  - Soft SPI
  #define LCD2_03_PIN EXP1_05_PIN  // EN2
  #define LCD2_05_PIN EXP1_03_PIN  // EN1
  #define LCD2_06_PIN EXP1_08_PIN  // MOSI - Soft SPI

  #define FORCE_SOFT_SPI

#elif ANY(ADAPTER_TH3D_DUAL, ADAPTER_MKS_DUAL)

  /**
   * https://www.th3dstudio.com/product/ezboard-creality-dual-lcd-adapter-board/
   *                   ------
   *    LCD1-1 <= SCK | 1  2 | ENC   => LCD1-2
   *    LCD2-5 <= EN1 | 3  4 | RESET => LCD1-4
   *    LCD2-3 <= EN2   5  6 | A0    => LCD1-7
   *    LCD1-6 <=  CS | 7  8 | MOSI  => LCD2-6
   *              GND | 9 10 | 5V
   *                   ------
   */
  #define LCD1_02_PIN EXP1_02_PIN  // ENC
  #define LCD1_04_PIN EXP1_04_PIN  // RESET
  #define LCD1_06_PIN EXP1_07_PIN  // CS
  #define LCD1_07_PIN EXP1_06_PIN  // A0

  #define LCD2_02_PIN EXP1_01_PIN  // SCK  - Soft SPI
  #define LCD2_03_PIN EXP1_05_PIN  // EN2
  #define LCD2_05_PIN EXP1_03_PIN  // EN1
  #define LCD2_06_PIN EXP1_08_PIN  // MOSI - Soft SPI

  #define FORCE_SOFT_SPI

#elif ENABLED(ADAPTER_CUSTOM_E3_DIP_TFT)

  /**
   * FYSETC_MINI_12864_2_1 / MKS_MINI_12864_V3 / BTT_MINI_12864_V1 display pinout
   *
   *                   Board                               Display
   *                   ------                               ------
   * (NEOPIXEL)  PA15 | 1  2 | PB6 (BTN_ENC)            5V |10  9 | GND
   * (BTN_EN2)   PA9  | 3  4 | RESET                    -- | 8  7 | --
   * (BTN_EN1)   PA10   5  6 | PB9 (LCD_RESET)    NEOPIXEL | 6  5   LCD RESET
   * (LCD_A0)    PB8  | 7  8 | PB7 (LCD_CS)         LCD_A0 | 4  3 | LCD_CS
   *              GND | 9 10 | 5V                  BTN_ENC | 2  1 | BEEP
   *                   ------                               ------
   *                    EXP1                                 EXP1
   *
   *
   *                  -----                                  ------
   *                  | 1 | RST                         -- |10  9 | --
   *                  | 2 | PA3 RX2              RESET_BTN | 8  7 | SD_DETECT
   *                  | 3 | PA2 TX2               LCD_MOSI | 6  5   EN2
   *                  | 4 | GND                         -- | 4  3 | EN1
   *                  | 5 | 5V                     LCD_SCK | 2  1 | --
   *                  -----                                 ------
   *                   TFT                                   EXP2

   *
   * Needs custom cable.
   *
   *    BOARD EXP1 NEOPIXEL   <-->  LCD EXP1 NEOPIXEL
   *    BOARD EXP1 BTN_ENC    <-->  LCD EXP1 BTN_ENC
   *    BOARD EXP1 BTN_EN2    <-->  LCD EXP2 EN2
   *    BOARD EXP1 RESET      <-->  LCD EXP2 RESET_BTN
   *    BOARD EXP1 BTN_EN1    <-->  LCD EXP2 EN1
   *    BOARD EXP1 LCD_RESET  <-->  LCD EXP1 LCD RESET
   *    BOARD EXP1 LCD_A0     <-->  LCD EXP1 LCD_A0
   *    BOARD EXP1 LCD_CS     <-->  LCD EXP1 LCD_CS
   *    BOARD TFT RX2         <-->  LCD EXP2 LCD_MOSI
   *    BOARD TFT TX2         <-->  LCD EXP2 LCD_SCK
   */

  #define LCD1_01_PIN                      -1   // No Beeper
  #define LCD1_02_PIN              EXP1_02_PIN  // BTN_ENC
  #define LCD1_03_PIN              EXP1_08_PIN  // DOGLCD_CS
  #define LCD1_04_PIN              EXP1_07_PIN  // DOGLCD_A0
  #define LCD1_05_PIN              EXP1_06_PIN  // LCD_RESET_PIN
  #define LCD1_06_PIN              EXP1_01_PIN  // NEOPIXEL_PIN

  #define LCD2_01_PIN                      -1   // no DOGLCD_MISO for sdcard
  #define LCD2_02_PIN                   TFT_02  // DOGLCD_SCK
  #define LCD2_03_PIN              EXP1_03_PIN  // BTN_EN1
  #define LCD2_04_PIN                      -1   // no SDSS for sdcard
  #define LCD2_05_PIN              EXP1_05_PIN  // BTN_EN2
  #define LCD2_06_PIN                   TFT_03  // DOGLCD_MOSI
  #define LCD2_07_PIN                      -1   // no SD_DETECT_PIN for sdcard
  #define LCD2_08_PIN                      -1   // no KILL_PIN

  #define FORCE_SOFT_SPI

#elif ENABLED(ADAPTER_CUSTOM_MINI_E3_TFT)

  /**
   * FYSETC_MINI_12864_2_1 / MKS_MINI_12864_V3 / BTT_MINI_12864_V1 display pinout
   *
   *       Board                      Display
   *       ------                     ------
   * PB5  | 1  2 | PA15       (BEEP) |10  9 | BTN_ENC
   * PA9  | 3  4 | RESET      LCD_CS | 8  7 | LCD A0
   * PA10 | 5  6 | PB9       LCD_RST | 6  5 | RED
   * PB8  | 7  8 | PB15      (GREEN) | 4  3 | (BLUE)
   * GND  | 9 10 | 5V            GND | 2  1 | 5V
   *       ------                     ------
   *        EXP1                       EXP1
   *
   *            ---                   ------
   *       RST | 1 |          (MISO) |10  9 | SCK
   * (RX2) PA3 | 2 |         BTN_EN1 | 8  7 | (SS)
   * (TX2) PA2 | 3 |         BTN_EN2 | 6  5 | MOSI
   *       GND | 4 |            (CD) | 4  3 | (RST)
   *        5V | 5 |             GND | 2  1 | (KILL)
   *            ---                   ------
   *            TFT                    EXP2
   *
   * Needs custom cable:
   *
   *    Board             Display
   *
   *   EXP1-10 ---------- EXP1-1   5V
   *   EXP1-9 ----------- EXP1-2   GND
   *   EXP1-8 ----------- EXP2-6   EN2
   *   EXP1-7 ----------- EXP1-5   RED
   *   EXP1-6 ----------- EXP2-8   EN1
   *   EXP1-5 ----------- n/c
   *   EXP1-4 ----------- EXP1-6   RESET
   *   EXP1-3 ----------- EXP1-8   LCD_CS
   *   EXP1-2 ----------- EXP1-9   ENC
   *   EXP1-1 ----------- EXP1-7   LCD_A0
   *
   *    TFT-2 ----------- EXP2-5   MOSI
   *    TFT-3 ----------- EXP2-9   SCK
   *
   * for backlight configuration see steps 2 (V2.1) and 3 in https://wiki.fysetc.com/Mini12864_Panel/
   */

  #define LCD1_01_PIN                      -1   // No Beeper
  #define LCD1_02_PIN              EXP1_02_PIN  // BTN_ENC
  #define LCD1_03_PIN              EXP1_03_PIN  // DOGLCD_CS
  #define LCD1_04_PIN              EXP1_01_PIN  // DOGLCD_A0
  #define LCD1_05_PIN              EXP1_06_PIN  // LCD_RESET_PIN
  #define LCD1_06_PIN              EXP1_07_PIN  // NEOPIXEL_PIN

  #define LCD2_01_PIN                      -1   // no DOGLCD_MISO for sdcard
  #define LCD2_02_PIN                   TFT_03  // DOGLCD_SCK
  #define LCD2_03_PIN              EXP1_06_PIN  // BTN_EN1
  #define LCD2_04_PIN                      -1   // no SDSS for sdcard
  #define LCD2_05_PIN              EXP1_08_PIN  // BTN_EN2
  #define LCD2_06_PIN                   TFT_02  // DOGLCD_MOSI
  #define LCD2_07_PIN                      -1   // no SD_DETECT_PIN for sdcard
  #define LCD2_08_PIN                      -1   // no KILL_PIN

  #define FORCE_SOFT_SPI

#elif ENABLED(ADAPTER_CUSTOM_CREALITY_V4_DEBUG_HEADER)

  /**
   *
   *                 Board (RET6 12864 LCD)              Display
   *                 ------                               ------
   *  (EN1)    PC6  | 1  2 | PB2  (BTN_ENC)           5V |10  9 | GND
   *  (LCD_CS) PB10 | 3  4 | PB11 (LCD RESET)         -- | 8  7 | --
   *  (LCD_A0) PB14   5  6 | PB13 (EN2)           (DIN)  | 6  5   (LCD RESET)
   *  (LCD_SCK)PB12 | 7  8 | PB15 (MOSI)        (LCD_A0) | 4  3 | (LCD_CS)
   *            GND | 9 10 | 5V                (BTN_ENC) | 2  1 | --
   *                 ------                               ------
   *                  EXP3                                 EXP1
   *
   *                                                      ------
   *                -----                             -- |10  9 | --
   *                | 1 | VCC                    (RESET) | 8  7 | --
   *                | 2 | PA13 (DIN)             (MOSI)  | 6  5   (EN2)
   *                | 3 | PA14                        -- | 4  3 | (EN1)
   *                | 4 | GND                   (LCD_SCK)| 2  1 | --
   *                -----                                 ------
   *              Debug port                               EXP2
   *
   * Needs custom cable. Connect EN2-EN2, LCD_CS-LCD_CS and so on.
   * Debug port is just above EXP1. You may need to add pins.
   *
   */

  #define LCD1_01_PIN                      -1   // No Beeper
  #define LCD1_02_PIN              EXP3_02_PIN  // BTN_ENC
  #define LCD1_03_PIN              EXP3_03_PIN  // DOGLCD_CS
  #define LCD1_04_PIN              EXP3_05_PIN  // DOGLCD_A0
  #define LCD1_05_PIN              EXP3_04_PIN  // LCD_RESET_PIN
  #define LCD1_06_PIN              DEBUG_02_PIN // NEOPIXEL_PIN

  #define LCD2_01_PIN                      -1   // no DOGLCD_MISO for sdcard
  #define LCD2_02_PIN              EXP3_07_PIN  // DOGLCD_SCK
  #define LCD2_03_PIN              EXP3_01_PIN  // BTN_EN1
  #define LCD2_04_PIN                      -1   // no SDSS for sdcard
  #define LCD2_05_PIN              EXP3_06_PIN  // BTN_EN2
  #define LCD2_06_PIN              EXP3_08_PIN  // DOGLCD_MOSI
  #define LCD2_07_PIN                      -1   // no SD_DETECT_PIN for sdcard
  #define LCD2_08_PIN                      -1   // no KILL_PIN

  #define FORCE_SOFT_SPI

#elif ENABLED(SKR_MINI_SCREEN_ADAPTER)
  /** https://github.com/VoronDesign/Voron-Hardware/tree/master/SKR-Mini_Screen_Adaptor/SRK%20Mini%20E3%20V3.0
   *
   *            SKR Mini E3 V3.0                   SKR Mini Screen Adaptor
   *                 ------                                ------
   *            PB5 | 1  2 | PA15                      -- | 1  2 | PB5  (BTN_ENC)
   *            PA9 | 3  4 | RESET           (LCD CS) PB8 | 3  4 | PD6  (LCD_A0)
   *           PA10   5  6 | PB9              (RESET) PB9   5  6 | PA15 (DIN)
   *            PB8 | 7  8 | PD6                       -- | 7  8 | --
   *            GND | 9 10 | 5V                       GND | 9 10 | 5V
   *                 ------                                ------
   *                  EXP1                                  EXP1
   *
   *                 ------                                ------
   *            5V  | 1  2 | GND                     MISO | 1  2 | SCK
   *            CS  | 3  4 | SCK               (EN1) PA10 | 3  4 | --
   *          MOSI  | 5  6 | MISO              (EN2)  PA9   5  6 | MOSI
   *           3V3  | 7  8 | GND                       -- | 7  8 | --
   *                 ------                           GND | 9 10 | RESET (Kill)
   *                  SPI                                  ------
   *                                                        EXP2
   */

  #if ENABLED(FYSETC_MINI_12864_2_1)
    #define LCD1_01_PIN                      -1   // No Beeper
    #define LCD1_02_PIN              EXP1_01_PIN  // BTN_ENC
    #define LCD1_03_PIN              EXP1_07_PIN  // DOGLCD_CS
    #define LCD1_04_PIN              EXP1_08_PIN  // DOGLCD_A0
    #define LCD1_05_PIN              EXP1_06_PIN  // LCD_RESET_PIN
    #define LCD1_06_PIN              EXP1_02_PIN  // NEOPIXEL_PIN

    #define LCD2_01_PIN                      -1   // no DOGLCD_MISO for sdcard
    #define LCD2_02_PIN              SPI1_04_PIN  // DOGLCD_SCK
    #define LCD2_03_PIN              EXP1_03_PIN  // BTN_EN1
    #define LCD2_04_PIN                      -1   // no SDSS for sdcard
    #define LCD2_05_PIN              EXP1_05_PIN  // BTN_EN2
    #define LCD2_06_PIN              SPI1_05_PIN  // DOGLCD_MOSI
    #define LCD2_07_PIN                      -1   // no SD_DETECT_PIN for sdcard
    #define LCD2_08_PIN                      -1   // no KILL_PIN

    #define FORCE_SOFT_SPI

  #else
    #error "Only CR10_FYSETC_MINI_12864_2_1 and compatibles are currently supported on the BIGTREE_SKR_MINI_E3 with SKR_MINI_SCREEN_ADAPTER"
  #endif

#elif ENABLED(ADAPTER_CUSTOM_E3_V3_NEO)

  /**
   *        BTT SKR Mini E3 V3.0 Board                    Display
   *                 ------                               ------
   *    (EN2)  PB5  | 1  2 | PA15(BTN_ENC)            5V |10  9 | GND
   *  (LCD_CS) PA9  | 3  4 | RST (RESET)              -- | 8  7 | --
   *  (LCD_A0) PA10   5  6 | PB9 (EN1)            (DIN)  | 6  5   (RESET)
   *  (LCD_SCK)PB8  | 7  8 | PD6 (MOSI)         (LCD_A0) | 4  3 | (LCD_CS)
   *            GND | 9 10 | 5V                (BTN_ENC) | 2  1 | --
   *                 ------                               ------
   *                  EXP1                                 EXP1
   *
   *                                                      ------
   *                                                  -- |10  9 | --
   *                   ---                       (RESET) | 8  7 | --
   *                  | 3 |                      (MOSI)  | 6  5   (EN2)
   *                  | 2 | (DIN)                     -- | 4  3 | (EN1)
   *                  | 1 |                     (LCD_SCK)| 2  1 | --
   *                   ---                                ------
   *                NeoPixel                               EXP2
   *
   * Needs custom cable. Connect EN2-EN2, LCD_CS-LCD_CS and so on.
   *
   * Check the index/notch position twice!!!
   * On BTT boards pins from IDC10 connector are numbered in unusual order.
   */

  #define LCD1_01_PIN                      -1   // No Beeper
  #define LCD1_02_PIN              EXP1_02_PIN  // BTN_ENC
  #define LCD1_03_PIN              EXP1_03_PIN  // DOGLCD_CS
  #define LCD1_04_PIN              EXP1_05_PIN  // DOGLCD_A0
  #define LCD1_05_PIN              EXP1_04_PIN  // LCD_RESET_PIN
  #define LCD1_06_PIN              NEOPIXEL_PIN // NEOPIXEL_PIN

  #define LCD2_01_PIN                      -1   // no DOGLCD_MISO for sdcard
  #define LCD2_02_PIN              EXP1_07_PIN  // DOGLCD_SCK
  #define LCD2_03_PIN              EXP1_06_PIN  // BTN_EN1
  #define LCD2_04_PIN                      -1   // no SDSS for sdcard
  #define LCD2_05_PIN              EXP1_01_PIN  // BTN_EN2
  #define LCD2_06_PIN              EXP1_08_PIN  // DOGLCD_MOSI
  #define LCD2_07_PIN                      -1   // no SD_DETECT_PIN for sdcard
  #define LCD2_08_PIN                      -1   // no KILL_PIN

  #define FORCE_SOFT_SPI

#endif

// Provide One-to-One mapping by default

#define _LCD_SET(N,I) && (LCD##I##_0##N##_PIN == 0)

#if defined(EXP1_01_PIN) REPEAT2_S(1,9,_LCD_SET,1)
  #define LCD1_01_PIN EXP1_01_PIN
  #define LCD1_02_PIN EXP1_02_PIN
  #define LCD1_03_PIN EXP1_03_PIN
  #define LCD1_04_PIN EXP1_04_PIN
  #define LCD1_05_PIN EXP1_05_PIN
  #define LCD1_06_PIN EXP1_06_PIN
  #define LCD1_07_PIN EXP1_07_PIN
  #define LCD1_08_PIN EXP1_08_PIN
#endif

#if defined(EXP2_01_PIN) REPEAT2_S(1,9,_LCD_SET,2)
  #define LCD2_01_PIN EXP2_01_PIN
  #define LCD2_02_PIN EXP2_02_PIN
  #define LCD2_03_PIN EXP2_03_PIN
  #define LCD2_04_PIN EXP2_04_PIN
  #define LCD2_05_PIN EXP2_05_PIN
  #define LCD2_06_PIN EXP2_06_PIN
  #define LCD2_07_PIN EXP2_07_PIN
  #define LCD2_08_PIN EXP2_08_PIN
#endif

// Define "LCD3" pins if EXP3 is not remapped to LCD1
// May not be needed if we just always remap "LCD1" to EXP3

#if defined(EXP3_01_PIN) REPEAT2_S(1,9,_LCD_SET,3)
  #define LCD3_01_PIN EXP3_01_PIN
  #define LCD3_02_PIN EXP3_02_PIN
  #define LCD3_03_PIN EXP3_03_PIN
  #define LCD3_04_PIN EXP3_04_PIN
  #define LCD3_05_PIN EXP3_05_PIN
  #define LCD3_06_PIN EXP3_06_PIN
  #define LCD3_07_PIN EXP3_07_PIN
  #define LCD3_08_PIN EXP3_08_PIN
#endif

#undef _LCD_SET

#ifndef LCD1_01_PIN
  #define LCD1_01_PIN -1
#endif
#ifndef LCD1_02_PIN
  #define LCD1_02_PIN -1
#endif
#ifndef LCD1_03_PIN
  #define LCD1_03_PIN -1
#endif
#ifndef LCD1_04_PIN
  #define LCD1_04_PIN -1
#endif
#ifndef LCD1_05_PIN
  #define LCD1_05_PIN -1
#endif
#ifndef LCD1_06_PIN
  #define LCD1_06_PIN -1
#endif
#ifndef LCD1_07_PIN
  #define LCD1_07_PIN -1
#endif
#ifndef LCD1_08_PIN
  #define LCD1_08_PIN -1
#endif

#ifndef LCD2_01_PIN
  #define LCD2_01_PIN -1
#endif
#ifndef LCD2_02_PIN
  #define LCD2_02_PIN -1
#endif
#ifndef LCD2_03_PIN
  #define LCD2_03_PIN -1
#endif
#ifndef LCD2_04_PIN
  #define LCD2_04_PIN -1
#endif
#ifndef LCD2_05_PIN
  #define LCD2_05_PIN -1
#endif
#ifndef LCD2_06_PIN
  #define LCD2_06_PIN -1
#endif
#ifndef LCD2_07_PIN
  #define LCD2_07_PIN -1
#endif
#ifndef LCD2_08_PIN
  #define LCD2_08_PIN -1
#endif
