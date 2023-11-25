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

#include <stdbool.h>
#include <stdint.h>
#include "../../Variants/variants.h"
#include "GUI.h"

#define SPACE_X ((BTT_LCD_WIDTH - ICON_WIDTH*4)/4)
#define START_X (SPACE_X/2)
#define SPACE_X_PER_ICON (BTT_LCD_WIDTH/4)

#define SPACE_Y ((BTT_LCD_HEIGHT - ICON_START_Y - ICON_HEIGHT*2)/2)

/*about GCODE KEY*/
#define GKEY_WIDTH  (BTT_LCD_WIDTH/2.5/4)
#define GKEY_HEIGHT ((BTT_LCD_HEIGHT - ICON_START_Y)/6)

void LCD_DMA_Config(void);

void lcd_frame_display(uint16_t sx,uint16_t sy,uint16_t w,uint16_t h, uint32_t addr);
void LOGO_ReadDisplay(void);
void ICON_ReadDisplay(uint16_t sx, uint16_t sy, uint8_t icon);
bool model_DirectDisplay(GUI_POINT pos, char *gcode);
bool model_DecodeToFlash(char *gcode);
void ICON_CustomReadDisplay(uint16_t sx,uint16_t sy,uint16_t w, uint16_t h, uint32_t addr);
void SMALLICON_ReadDisplay(uint16_t sx,uint16_t sy, uint8_t icon);
void ICON_PressedDisplay(uint16_t sx, uint16_t sy, uint8_t icon);

#ifdef __cplusplus
}
#endif
