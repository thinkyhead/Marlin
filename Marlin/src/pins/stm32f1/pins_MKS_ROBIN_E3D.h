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
 * MKS Robin E3D (STM32F103RCT6) board pin assignments
 */

#if HAS_MULTI_HOTEND || E_STEPPERS > 1
  #error "MKS Robin E3D only supports one hotend / E-stepper. Comment out this line to continue."
#endif

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "MKS Robin E3D"
#endif

//
// Steppers
//
#ifndef X_CS_PIN
  #define X_CS_PIN                          PC7
#endif
#ifndef Y_CS_PIN
  #define Y_CS_PIN                          PD2
#endif
#ifndef Z_CS_PIN
  #define Z_CS_PIN                          PC12
#endif
#ifndef E0_CS_PIN
  #define E0_CS_PIN                         PC11
#endif

//
// Software SPI pins for TMC2130 stepper drivers
//
#if ENABLED(TMC_USE_SW_SPI)
  #ifndef TMC_SW_MOSI
    #define TMC_SW_MOSI                     PB15
  #endif
  #ifndef TMC_SW_MISO
    #define TMC_SW_MISO                     PB14
  #endif
  #ifndef TMC_SW_SCK
    #define TMC_SW_SCK                      PB13
  #endif
#endif

#include "pins_MKS_ROBIN_E3_common.h"

#ifndef BEEPER_PIN
  #define BEEPER_PIN                 EXP1_01_PIN
#endif

/**
 * Note: MKS Robin TFT screens use various TFT controllers.
 * If the screen stays white, disable 'LCD_RESET_PIN'
 * to let the bootloader init the screen.
 */

#if HAS_SPI_TFT

  // Shared SPI TFT

  #define LCD_BACKLIGHT_PIN          EXP1_03_PIN

  #define TOUCH_CS_PIN               EXP1_05_PIN  // SPI1_NSS
  #define TOUCH_SCK_PIN              EXP2_02_PIN  // SPI1_SCK
  #define TOUCH_MISO_PIN             EXP2_01_PIN  // SPI1_MISO
  #define TOUCH_MOSI_PIN             EXP2_06_PIN  // SPI1_MOSI

  #define BTN_EN1                    EXP2_03_PIN
  #define BTN_EN2                    EXP2_05_PIN
  #define BTN_ENC                    EXP1_02_PIN

  #define TFT_CS_PIN                 EXP1_07_PIN
  #define TFT_SCK_PIN                EXP2_02_PIN
  #define TFT_MISO_PIN               EXP2_01_PIN
  #define TFT_MOSI_PIN               EXP2_06_PIN
  #define TFT_DC_PIN                 EXP1_08_PIN
  #define TFT_RST_PIN                EXP1_04_PIN
  #define TFT_A0_PIN                  TFT_DC_PIN

  #define TFT_RESET_PIN              EXP1_04_PIN
  #define TFT_BACKLIGHT_PIN          EXP1_03_PIN

  #define TOUCH_BUTTONS_HW_SPI
  #define TOUCH_BUTTONS_HW_SPI_DEVICE          1

  #ifndef TFT_WIDTH
    #define TFT_WIDTH                        480
  #endif
  #ifndef TFT_HEIGHT
    #define TFT_HEIGHT                       320
  #endif

  #define LCD_READ_ID                       0xD3
  #define LCD_USE_DMA_SPI

#endif

#if HAS_SPI_GRAPHICAL_TFT
  // Emulated DOGM SPI
  #define LCD_PINS_ENABLE            EXP1_03_PIN
  #define LCD_PINS_RS                EXP1_04_PIN
  #define BTN_ENC                    EXP1_02_PIN
  #define BTN_EN1                    EXP2_03_PIN
  #define BTN_EN2                    EXP2_05_PIN
#elif ENABLED(TFT_480x320_SPI)
  #define TFT_DRIVER                      ST7796
  #define TFT_BUFFER_SIZE                  14400
#endif

#if HAS_WIRED_LCD && !HAS_SPI_TFT

  // NON TFT Displays

  #if ENABLED(MKS_MINI_12864)

    // MKS MINI12864 and MKS LCD12864B
    // If using MKS LCD12864A (Need to remove RPK2 resistor)

    #define LCD_BACKLIGHT_PIN               -1
    #define LCD_RESET_PIN                   -1
    #define DOGLCD_A0                EXP1_07_PIN
    #define DOGLCD_CS                EXP1_06_PIN
    #define DOGLCD_SCK               EXP2_02_PIN
    #define DOGLCD_MOSI              EXP2_06_PIN

    // Required for MKS_MINI_12864 with this board
    #define MKS_LCD12864B
    #undef SHOW_BOOTSCREEN

  #else                                           // !MKS_MINI_12864

    #define LCD_PINS_D4              EXP1_05_PIN
    #if IS_ULTIPANEL
      #define LCD_PINS_D5            EXP1_06_PIN
      #define LCD_PINS_D6            EXP1_07_PIN
      #define LCD_PINS_D7            EXP1_08_PIN

      #if ENABLED(REPRAP_DISCOUNT_FULL_GRAPHIC_SMART_CONTROLLER)
        #define BTN_ENC_EN           LCD_PINS_D7  // Detect the presence of the encoder
      #endif

    #endif

    #ifndef BOARD_ST7920_DELAY_1
      #define BOARD_ST7920_DELAY_1 DELAY_NS(125)
    #endif
    #ifndef BOARD_ST7920_DELAY_2
      #define BOARD_ST7920_DELAY_2 DELAY_NS(125)
    #endif
    #ifndef BOARD_ST7920_DELAY_3
      #define BOARD_ST7920_DELAY_3 DELAY_NS(125)
    #endif

  #endif // !MKS_MINI_12864

#endif // HAS_WIRED_LCD && !HAS_SPI_TFT
