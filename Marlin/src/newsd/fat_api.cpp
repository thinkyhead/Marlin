/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2024 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
#include "../inc/MarlinConfig.h"

#if ENABLED(SDSUPPORT)

#include "fat_api.h"
#include "./API/vfs.h"
#include "../MarlinCore.h"

// public:

bool FatApi::get_filename(const char* path, char* sfn, char* lfn) {
  FILINFO inf;
  FRESULT rst = f_stat(path, &inf);
  if (rst != FR_OK) return false;
  if (sfn) {
    strncpy(sfn, inf.altname, FF_SFN_BUF);
  }
  if (lfn) {
    strncpy(lfn, inf.fname, FF_LFN_BUF);
  }
  return true;
}

bool FatApi::get_filename(char* sfn, char* lfn) {
  return get_filename(infoFile.title, sfn, lfn);
}

bool FatApi::open(const char* path, BYTE mode) {
  FRESULT rst = f_open(&fat_fil, path, mode);
  if (rst == FR_OK) {
    file_opened = true;
  } else {
    file_opened = false;
  }
  return file_opened;
}

bool FatApi::sync() {
  return (f_sync(&fat_fil) == FR_OK);
}

bool FatApi::close() {
  file_opened = false;
  return (f_close(&fat_fil) == FR_OK);
}

bool FatApi::remove(const char* path) {
  if (file_opened) return false;

  return (f_unlink(path) == FR_OK);
}

bool FatApi::seekSet(const uint32_t pos) {
  // error if file not open or seek past end of file
  if (!isOpen() || pos > fileSize()) return false;

  return (f_lseek(&fat_fil, pos) == FR_OK);
}

int16_t FatApi::read() {
  UINT br;
  uint8_t b;
  return f_read(&fat_fil, &b, 1, &br) == FR_OK ? b : -1;
}

int16_t FatApi::read(void* buf, uint16_t nbyte) {
  UINT br;
  return f_read(&fat_fil, buf, nbyte, &br) == FR_OK ? br : -1;
}

size_t FatApi::write(uint8_t b) {
  UINT bw;
  return f_write(&fat_fil, &b, 1, &bw) == FR_OK ? bw : 0;
}

int16_t FatApi::write(const void* buf, uint16_t nbyte) {  
  UINT bw;
  return f_write(&fat_fil, buf, nbyte, &bw) == FR_OK ? bw : 0;
}

void FatApi::write(const char* str) {
  write(str, strlen(str));
}

void FatApi::write_P(PGM_P str) {
  write((const char*) str);
}

void FatApi::writeln_P(PGM_P str) {
  write((const char*) str);
}

#endif