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

typedef enum
{
  _SPI_MODE0,
  _SPI_MODE1,
  _SPI_MODE2,
  _SPI_MODE3,
}_SPI_MODE;

typedef struct
{
  uint16_t  cs;
  uint16_t  sck;
  uint16_t  miso;
  uint16_t  mosi;
  _SPI_MODE mode;
  uint8_t   dataSize;
}_SW_SPI;

void SW_SPI_Config(_SW_SPI *sw_spi, _SPI_MODE mode, uint8_t dataSize,
uint16_t  cs,
uint16_t  sck,
uint16_t  miso,
uint16_t  mosi);

uint16_t SW_SPI_Read_Write(_SW_SPI *sw_spi, uint16_t d);
void SW_SPI_CS_Set(_SW_SPI *sw_spi, uint8_t level);

#ifdef __cplusplus
}
#endif
