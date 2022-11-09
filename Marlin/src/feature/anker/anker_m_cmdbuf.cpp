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

/*
 * @Author       : winter
 * @Date         : 2022-05-13 14:28:04
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 12:09:41
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if BOTH(ANKER_MAKE_API, ANKER_M_CMDBUF)

#include "anker_m_cmdbuf.h"
#include "../../gcode/queue.h"
#include "../../gcode/gcode.h"
#include "../../gcode/parser.h"

static void anker_m_cmdbuf_translate_xyze_axis(char *buf) {
  char *token;
       if ((token = strchr(buf, 'A'))) *token = 'X';
  else if ((token = strchr(buf, 'B'))) *token = 'Y';
  else if ((token = strchr(buf, 'C'))) *token = 'Z';
  else if ((token = strchr(buf, 'D'))) *token = 'E';
}

void anker_m_cmdbuf_init() {
  anker_m_cmdbuf_info_t *p_info = get_anker_m_cmdbuf_info();
  p_info->queue_translate_xyze_axis = anker_m_cmdbuf_translate_xyze_axis;
}

anker_m_cmdbuf_info_t *get_anker_m_cmdbuf_info() {
  static anker_m_cmdbuf_info_t anker_m_cmdbuf_info = {0};
  return &anker_m_cmdbuf_info;
}

#endif // ANKER_MAKE_API && ANKER_M_CMDBUF
