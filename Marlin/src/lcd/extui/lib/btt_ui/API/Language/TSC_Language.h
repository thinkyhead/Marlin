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

enum
{
  TSC_ENGLISH = 0,
  TSC_CHINESE,
  TSC_RUSSIAN,
  TSC_JAPANESE,
  TSC_ARMENIAN,
  TSC_GERMAN,
  TSC_CZECH,
  TSC_SPAIN,
  TSC_FRENCH,
  TSC_PORTUGUESE,
  TSC_ITALIAN,
  TSC_POLISH,
  TSC_SLOVAK,
  TSC_DUTCH,
  TSC_HUNGARIAN,
  TSC_TURKISH,
  TSC_GREEK,
  TSC_SLOVENIAN,
  TSC_CATALAN,
  TSC_TRAD_CHINESE,
  
  TSC_LANGUAGE_NUM,
};

enum {
#define X_WORD(NAME) LABEL_##NAME ,
#include "Language.inc"
#undef  X_WORD

  //add new keywords in 'Language.inc' file only
  //keep the following always at the end of this list
  LABEL_NUM,
  LABEL_BACKGROUND,
  LABEL_DYNAMIC,
  LABEL_CUSTOM_VALUE,
};

uint8_t * textSelect(uint16_t sel);

#ifdef __cplusplus
}
#endif
