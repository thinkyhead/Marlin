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

#include "../menu.h"

void listWidgetCreate(LABEL title, LISTITEM *items, uint16_t max_items, uint8_t cur_page);
void listWidgetSetCurPage(uint8_t cur_page);
bool listWidgetPreviousPage(void);
bool listWidgetNextPage(void);
void listWidgetRefreshPage(void);
void listWidgetRefreshMenu(void);
void listWidgetRefreshItem(uint16_t item);
uint8_t listWidgetGetCurPage(void);

#ifdef __cplusplus
}
#endif
