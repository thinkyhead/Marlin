/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#ifdef STM32H7xx
  #include "stm32h7xx_hal.h"
#else
  #error "LTDC TFT is currently only supported on STM32H7 hardware."
#endif
#include "../ltdc/ltdc.h"

#define DATASIZE_8BIT    SPI_DATASIZE_8BIT
#define DATASIZE_16BIT   SPI_DATASIZE_16BIT
#define TFT_IO TFT_LTDC

class TFT_LTDC {
  private:
    static uint16_t x_min, x_max, y_min, y_max, x_cur, y_cur; 
    static uint8_t reg;
    static uint32_t ReadID(uint16_t Reg);
    static void Transmit(uint16_t Data) {
      LTDC_Draw_Point(x_cur, y_cur, Data);
      x_cur++;
      if (x_cur > x_max) {
        x_cur = x_min;
        y_cur++;
        if (y_cur > y_max) y_cur = y_min;
      }
    }
    static void TransmitDMA(uint32_t MemoryIncrease, uint16_t *Data, uint16_t Count);

  public:
    static void Init();
    static uint32_t GetID();
    static bool isBusy();
    static void Abort() {/* __HAL_DMA_DISABLE(&DMAtx);*/ }

    static void DataTransferBegin(uint16_t DataWidth = DATASIZE_16BIT) {}
    static void DataTransferEnd() {};

    static void WriteData(uint16_t data);
    static void WriteReg(uint16_t Reg);

    static void WriteSequence(uint16_t *Data, uint16_t Count) { TransmitDMA(DMA_PINC_ENABLE, Data, Count); }
    static void WriteMultiple(uint16_t Color, uint16_t Count) { static uint16_t Data; Data = Color; TransmitDMA(DMA_PINC_DISABLE, &Data, Count); }
};

#ifdef STM32F1xx
  #define FSMC_PIN_DATA   STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, AFIO_NONE)
#elif defined(STM32F4xx)
  #define FSMC_PIN_DATA   STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, GPIO_AF12_FSMC)
  #define FSMC_BANK1_1    0x60000000U
  #define FSMC_BANK1_2    0x64000000U
  #define FSMC_BANK1_3    0x68000000U
  #define FSMC_BANK1_4    0x6C000000U
#else
  //#error No configuration for this MCU
#endif
