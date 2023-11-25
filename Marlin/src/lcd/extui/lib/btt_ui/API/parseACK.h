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

#include <stdint.h>
#include <stdbool.h>
#include "../TSC_Configuration.h"

static const char errormagic[]         = "Error:";
static const char echomagic[]          = "echo:";

#define ACK_MAX_SIZE 2048

typedef enum                      // popup message types available to display an echo message
{
  ECHO_NOTIFY_NONE = 0,            // ignore the echo message
  ECHO_NOTIFY_TOAST,               // Show a non invasive toast on the title bar for a preset duration.
  ECHO_NOTIFY_DIALOG,              // Show a window to notify the user and alow interaction.
} ECHO_NOTIFY_TYPE;

typedef enum                           // append at the end of this list the id of any new echo message for
{                                      // which a specific popup message type must be used to notify the user
  ECHO_ID_BUSY = 0,
  ECHO_ID_FRESH_FILE,
  ECHO_ID_DOING_FILE,
  ECHO_ID_PROBE_OFFSET,
  ECHO_ID_FLOW,
  ECHO_ID_ECHO,
  ECHO_ID_ECHO_G,
  ECHO_ID_ECHO_M,
  ECHO_ID_CAP,
  ECHO_ID_CONFIG,
  ECHO_ID_SETTINGS,
  ECHO_ID_SOFT_ENDSTOP,
  ECHO_ID_BED_LEVELING,
  ECHO_ID_FADE_HEIGHT,
  ECHO_ID_TOOL_CHANGE,
  ECHO_ID_COUNT,
} ECHO_ID;

typedef struct
{
  ECHO_NOTIFY_TYPE  notifyType;
  const char *const msg;
} ECHO;

void setCurrentAckSrc(uint8_t src);
void parseACK(void);
void parseRcvGcode(void);

void syncWriteToL1Cache(char ch);
void setIgnoreEcho(ECHO_ID msgId, bool state);

#ifdef __cplusplus
}
#endif

