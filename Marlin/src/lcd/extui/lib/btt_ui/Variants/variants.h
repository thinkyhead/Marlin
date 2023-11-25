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

#include "../TSC_Configuration.h"

/*
* hardware source
* TIM3 for Buzzer timer
* TIM6 for Neopixel RGB
* TIM7 for OS Timer
*/

// Type of LCD driver, now support[RM68042, ILI9488, ILI9341, ST7789, HX8558, SSD1963].
#define RM68042 0
#define ILI9488 1
#define ILI9341 2
#define ST7789  3
#define HX8558  4
#define SSD1963 5

#if defined(TFT24_V1_1)
  #include "pin_TFT24_V1_1.h"
#elif defined(TFT28_V1_0)
  #include "pin_TFT28_V1_0.h"
#elif defined(TFT28_V3_0)
  #include "pin_TFT28_V3_0.h"
#elif defined(TFT35_V1_0)
  #include "pin_TFT35_V1_0.h"
#elif defined(TFT35_V1_1)
  #include "pin_TFT35_V1_1.h"
#elif defined(TFT35_V1_2)
  #include "pin_TFT35_V1_2.h"
#elif defined(TFT35_V2_0)
  #include "pin_TFT35_V2_0.h"
#elif defined(TFT35_V3_0)
  #include "pin_TFT35_V3_0.h"  
#elif defined(TFT43_V3_0)
  #include "pin_TFT43_V3_0.h"
#elif defined(TFT50_V3_0)
  #include "pin_TFT50_V3_0.h"
#elif defined(TFT70_V3_0)
  #include "pin_TFT70_V3_0.h"
#elif defined(TFT35_E3_V3_0)
  #include "pin_TFT35_E3_V3_0.h"
#elif defined(MKS_32_V1_4)
  #include "pin_MKS_TFT32_V1_4.h"
#endif

#define LCD_ENCODER_SUPPORT (defined(LCD_ENCA_PIN) && defined(LCD_ENCB_PIN) && defined(LCD_BTN_PIN))

#define LCD_DRIVER_IS(n) (TFTLCD_DRIVER == n)
