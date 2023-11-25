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

#include "../API/menu.h"

typedef enum
{
  GKEY_0 = 0,
  GKEY_1,
  GKEY_2,
  GKEY_DEL,
  GKEY_3,
  GKEY_4,
  GKEY_5,
  GKEY_SPACE,
  GKEY_6,
  GKEY_7,
  GKEY_8,
  GKEY_ABC_123,
  GKEY_9,
  GKEY_10,
  GKEY_11,
  GKEY_12,
  GKEY_13,
  GKEY_14,
  GKEY_15,
  GKEY_16,
  GKEY_17,
  GKEY_18,
  GKEY_19,
  GKEY_20,
  GKEY_BACK,
  GKEY_SEND,
  GKEY_IDLE = IDLE_TOUCH,
}GKEY_VALUES;
// typedef enum
// {
//   GKEY_0 = 0,
//   GKEY_1,
//   GKEY_2,
//   GKEY_3,
//   GKEY_4,
//   GKEY_DEL,
//   GKEY_5,
//   GKEY_6,
//   GKEY_7,
//   GKEY_8,
//   GKEY_9,
//   GKEY_SPACE,
//   GKEY_10,
//   GKEY_11,
//   GKEY_12,
//   GKEY_13,
//   GKEY_14,
//   GKEY_ABC_123,
//   GKEY_15,
//   GKEY_16,
//   GKEY_17,
//   GKEY_18,
//   GKEY_19,
//   GKEY_20,
//   GKEY_BACK,
//   GKEY_SEND,
//   GKEY_IDLE = IDLE_TOUCH,
// }GKEY_VALUES;

typedef enum
{
  TERMINAL_GCODE,
  TERMINAL_ACK,
}TERMINAL_SRC;


void menuSendGcode(void);
// void menuTerminal(void);
void sendGcodeTerminalCache(char *stream, TERMINAL_SRC src);

#ifdef __cplusplus
}
#endif
