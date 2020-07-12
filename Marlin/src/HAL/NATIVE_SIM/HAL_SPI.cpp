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

/**
 * Software SPI functions originally from Arduino Sd2Card Library
 * Copyright (c) 2009 by William Greiman
 */

#ifdef __PLAT_NATIVE_SIM__

#include "../../inc/MarlinConfig.h"
#include <SPI.h>

// Software SPI

static uint8_t SPI_speed = 0;
uint8_t swSpiTransfer_mode_0(uint8_t b, const uint8_t spi_speed, const pin_t sck_pin, const pin_t miso_pin, const pin_t mosi_pin ) {
  LOOP_L_N(i, 8) {
    if (spi_speed == 0) {
      WRITE_PIN(mosi_pin, !!(b & 0x80));
      WRITE_PIN(sck_pin, HIGH);
      b <<= 1;
      if (miso_pin >= 0 && READ_PIN(miso_pin)) b |= 1;
      WRITE_PIN(sck_pin, LOW);
    }
    else {
      const uint8_t state = (b & 0x80) ? HIGH : LOW;
      LOOP_L_N(j, spi_speed)
        WRITE_PIN(mosi_pin, state);

      LOOP_L_N(j, spi_speed + (miso_pin >= 0 ? 0 : 1))
        WRITE_PIN(sck_pin, HIGH);

      b <<= 1;
      if (miso_pin >= 0 && READ_PIN(miso_pin)) b |= 1;

      LOOP_L_N(j, spi_speed)
        WRITE_PIN(sck_pin, LOW);
    }
  }

  return b;
}

static uint8_t spiTransfer(uint8_t b) {
  return swSpiTransfer_mode_0(b, SPI_speed, SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN);
}

void spiBegin() {
  OUT_WRITE(SD_SS_PIN, HIGH);
  SET_OUTPUT(SD_SCK_PIN);
  SET_INPUT(SD_MISO_PIN);
  SET_OUTPUT(SD_MOSI_PIN);
}

void spiInit(uint8_t spiRate) {
  // SPI_speed = swSpiInit(spiRate, SD_SCK_PIN, SD_MOSI_PIN);
  WRITE(SD_MOSI_PIN, HIGH);
  WRITE(SD_SCK_PIN, LOW);
}

uint8_t spiRec() { return spiTransfer(0xFF); }

void spiRead(uint8_t*buf, uint16_t nbyte) {
  for (int i = 0; i < nbyte; i++)
    buf[i] = spiTransfer(0xFF);
}

void spiSend(uint8_t b) { (void)spiTransfer(b); }

void spiSend(const uint8_t* buf, size_t nbyte) {
  for (uint16_t i = 0; i < nbyte; i++)
    (void)spiTransfer(buf[i]);
}

void spiSendBlock(uint8_t token, const uint8_t* buf) {
  (void)spiTransfer(token);
  for (uint16_t i = 0; i < 512; i++)
    (void)spiTransfer(buf[i]);
}

SPIClass::SPIClass(uint8_t spiPortNumber) {
  _settings[0].dataSize = DATA_SIZE_8BIT;
  _settings[0].clock = 0;
  _settings[0].dataMode = 0;
  _settings[0].bitOrder = 0;
  setModule(spiPortNumber);
}

void SPIClass::begin() {

}

void SPIClass::end() {

}

void SPIClass::beginTransaction(const SPISettings& s) {
  setClock(s.clock);
  setDataMode(s.dataMode);
  setDataSize(s.dataSize);
  setBitOrder(s.bitOrder);
}

void SPIClass::endTransaction() {

}

// Transfer using 1 "Data Size"
uint8_t SPIClass::transfer(uint16_t data) {
  return spiTransfer(data);
}
// Transfer 2 bytes in 8 bit mode
uint16_t SPIClass::transfer16(uint16_t data) {
  return (spiTransfer(data >> 8) << 8) | (spiTransfer(data & 0xFF) & 0xFF);
}

void SPIClass::send(uint8_t data) {
  spiSend(data);
}

uint16_t SPIClass::read() {
  return spiRec();
}

void SPIClass::read(uint8_t *buf, uint32_t len) {
  spiRead(buf, len);
}

void SPIClass::dmaSend(void *buf, uint16_t length, bool minc) {
  uint8_t *ptr = (uint8_t*)buf;
  while(length--) {
    if (_currentSetting->dataSize == DATA_SIZE_16BIT) spiSend(*(ptr + 1));
    spiSend(*ptr);
    if (minc) ptr += _currentSetting->dataSize;
  }
}

uint8_t SPIClass::dmaTransfer(const void * transmitBuf, void * receiveBuf, uint16_t length) {
  //todo: transmit?!
  read((uint8_t*)receiveBuf, length);
  return 1;
}

void SPIClass::setModule(uint8_t device) {
  _currentSetting = &_settings[device - 1];
}

void SPIClass::setClock(uint32_t clock) {
  _currentSetting->clock = clock;
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
  _currentSetting->bitOrder = bitOrder;
}

void SPIClass::setDataMode(uint8_t dataMode) {
  _currentSetting->dataMode = dataMode;
}

void SPIClass::setDataSize(uint32_t ds) {
  _currentSetting->dataSize = ds;
}

SPIClass SPI(1);

#endif // __PLAT_NATIVE_SIM__
