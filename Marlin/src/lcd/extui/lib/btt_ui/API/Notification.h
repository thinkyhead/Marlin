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

#include "menu.h"

#define TOAST_X_PAD           START_X
#define TOAST_Y_PAD           3

#define TOAST_MSG_COUNT       5
#define TOAST_MSG_LENGTH      35
#define TOAST_DISPLAY_LENGTH  TOAST_MSG_LENGTH

#define MAX_MSG_COUNT         5
#define MAX_MSG_TITLE_LENGTH  15
#define MAX_MSG_LENGTH        70

typedef struct
{
  DIALOG_TYPE style;
  uint8_t isNew;
  char text[TOAST_MSG_LENGTH];
}TOAST;

typedef struct
{
  DIALOG_TYPE style;
  char title[MAX_MSG_TITLE_LENGTH];
  char text[MAX_MSG_LENGTH];
}NOTIFICATION;

bool toastRunning(void);
void addToast(DIALOG_TYPE style, char * text);
void drawToast(bool redraw);
void loopToast(void);
void addNotification(DIALOG_TYPE style, char * title, char * text, bool ShowDialog);
void replayNotification(uint8_t index);
NOTIFICATION * getNotification(uint8_t index);
bool hasNotification(void);
void titleBarPress(void);
void clearNotification(void);
void setNotificationHandler(void (*handler)());

#ifdef __cplusplus
}
#endif
