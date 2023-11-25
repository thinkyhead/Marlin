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

#include "stdint.h"
#include "stdbool.h"

#define CMD_MAX_LIST 20
#define CMD_MAX_CHAR 48

typedef struct
{
  char gcode[CMD_MAX_CHAR];
  uint8_t src;   // 0: TouchScreen Cmd, 1: Serial Port 2 rx Cmd, 2: Serial Port 3 rx Cmd
}GCODE;

typedef struct
{
  GCODE   queue[CMD_MAX_LIST];
  uint8_t index_r; // Ring buffer read position
  uint8_t index_w; // Ring buffer write position
  uint8_t count;   // Count of commands in the queue
}GCODE_QUEUE;

extern GCODE_QUEUE infoCmd;
extern GCODE_QUEUE infoCacheCmd;


bool storeCmd(char * const format,...);
void mustStoreCmd(const char * format,...);
void mustStoreScript(const char * format,...);
bool storeCmdFromUART(uint8_t port, const char * gcode);
void mustStoreCacheCmd(const char * format,...);
bool moveCacheToCmd(void);
void clearCmdQueue(void);
void parseQueueCmd(void);
void sendQueueCmd(void);

#ifdef __cplusplus
}
#endif
