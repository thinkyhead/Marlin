/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2022 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

/*
 * @Author       : winter
 * @Date         : 2022-05-13 14:28:19
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 12:08:20
 * @Description  :
 */

typedef struct {
  void (*queue_translate_xyze_axis)(char *buf);
} anker_m_cmdbuf_info_t;

void anker_m_cmdbuf_init();
anker_m_cmdbuf_info_t* get_anker_m_cmdbuf_info();
