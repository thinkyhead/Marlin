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

// #define BUZZER_CACHE_SIZE 5
// typedef struct
// {
//   uint32_t frequency[BUZZER_CACHE_SIZE];
//   uint32_t duration[BUZZER_CACHE_SIZE];
//   uint16_t wIndex;
//   uint16_t rIndex;
//   uint16_t count;
// }BUZZER;

typedef enum
{
  sound_success,
  sound_error,
  sound_ok,
  sound_cancel,
  sound_notify,
  sound_deny,
  sound_toast,
  sound_keypress,
}SOUND;


// void Buzzer_Config(void);
// void Buzzer_DeConfig(void);
void Buzzer_TurnOn(const uint32_t frequency, const uint32_t duration);
void Buzzer_play(SOUND sound);
void loopBuzzer(void);

#ifdef BUZZER_PIN
#define BUZZER_PLAY(x) Buzzer_play(x)
#else
#define BUZZER_PLAY(x)
#endif

#ifdef __cplusplus
}
#endif
