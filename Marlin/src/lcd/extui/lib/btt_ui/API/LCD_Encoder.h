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

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"
#define LCD_CHANGE_MODE_INTERVALS 1500 //long pressed 1.5s
#define LCD_BUTTON_INTERVALS      20   //20ms
#define LCD_FREE_WIDTH         (BTT_LCD_WIDTH - BTT_LCD_WIDTH / 5)
#define LCD_FREE_HEIGHT        (BTT_LCD_HEIGHT / 5)
#define EN_A (1<<0)
#define EN_B (1<<1)
#define EN_C (1<<2)
#define B01  1
#define B10  2

extern int16_t encoderPosition;

void HW_EncoderInit(void);
bool encoder_ReadBtn(uint16_t intervals);
bool LCD_ReadPen(uint16_t intervals);
bool LCD_BtnTouch(uint16_t intervals);
uint8_t LCD_ReadTouch(void);
void loopCheckEncoderSteps(void);
uint8_t encoder_GetPos(void);
bool encoder_CheckState(void);
void sendEncoder(uint8_t num);

#ifdef __cplusplus
}
#endif
