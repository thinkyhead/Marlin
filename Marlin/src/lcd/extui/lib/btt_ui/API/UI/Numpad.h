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

#include "../../Variants/variants.h"
#include "list_item.h"

#define SKEYHEIGHT      (BTT_LCD_HEIGHT-ICON_START_Y)/4
#define SKEYWIDTH       BTT_LCD_WIDTH/4

#define KEY_NUM 16
#define FLOAT_BUFLONG 7
#define INT_BUFLONG 6

typedef enum
{
  SKEY_0 = 0,
  SKEY_1,
  SKEY_2,
  SKEY_3,
  SKEY_4,
  SKEY_5,
  SKEY_6,
  SKEY_7,
  SKEY_8,
  SKEY_9,
  SKEY_10,
  SKEY_11,
  SKEY_IDLE = IDLE_TOUCH,
}SKEY_VALUES;

typedef enum
{
  NUM_KEY_1 = 0,
  NUM_KEY_2,
  NUM_KEY_3,
  NUM_KEY_OK,
  NUM_KEY_4,
  NUM_KEY_5,
  NUM_KEY_6,
  NUM_KEY_DEL,
  NUM_KEY_7,
  NUM_KEY_8,
  NUM_KEY_9,
  NUM_KEY_EXIT,
  NUM_KEY_DEC,
  NUM_KEY_0,
  NUM_KEY_MINUS,
  NUM_KEY_RESET,

  NUM_KEY_IDLE = IDLE_TOUCH,
}NUM_KEY_VALUES;


float numPadFloat(uint8_t* title, float old_val, float reset_val, bool negative);

int32_t numPadInt(uint8_t* title, int32_t old_val, int32_t reset_val, bool negative);

#ifdef __cplusplus
}
#endif
