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

typedef void (*FP_TASK)(void *);

typedef struct
{
  uint32_t time_ms;
  uint32_t next_time;
  FP_TASK  task;
  void    *para;
  uint8_t  is_exist;
  uint8_t  is_repeat;
}OS_TASK;


void OS_TimerInitMs(void);
uint32_t OS_GetTimeMs(void);
void Delay_ms(uint16_t ms);

void OS_TaskInit(OS_TASK *task, uint32_t time_ms, FP_TASK function,void *para);
void OS_TaskCheck(OS_TASK *task);
void OS_TaskEnable(OS_TASK *task, uint8_t is_exec, uint8_t is_repeat);
void OS_TaskDisable(OS_TASK *task);

#ifdef __cplusplus
}
#endif
