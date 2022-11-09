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
 * @Date         : 2022-05-24 14:08:20
 * @LastEditors: winter.tian winter.tian@anker-in.com
 * @LastEditTime: 2022-10-11 16:53:33
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if BOTH(ANKER_MAKE_API, ANKER_NOZZLE_BOARD)

#include "../../feature/anker/anker_nozzle_board.h"

#include "../gcode.h"
#include "../../module/settings.h"

#if 0
//#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//16 Kbytes
//#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//16 Kbytes
//#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//16 Kbytes
//#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//16 Kbytes
//#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//64 Kbytes
//#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//128 Kbytes
//#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//128 Kbytes

#define FLASH_TIMEOUT_VALUE 50000U /* 50 s */
#define FLASH_BOOTLOADER_ADDR_START ((uint32_t)0x08000000)
#define FLASH_BOOTLOADER_LENGTH_MAX ((uint32_t)0x0000C000)
#define FLASH_CHECK_ADDR_START ((uint32_t)0x080C0000)
#define FLASH_CHECK_ADDR_END ((uint32_t)0x080DFFFF)
typedef struct {
  __IO uint32_t init_version_length_addr;
  __IO uint32_t init_version_crc_addr;
  __IO uint32_t last_version_length_addr;
  __IO uint32_t last_version_crc_addr;
  uint32_t init_version_length_value;
  uint32_t init_version_crc_value;
  uint32_t last_version_length_value;
  uint32_t last_version_crc_value;
} flash_check_info_t;
static flash_check_info_t flash_check_info = {FLASH_CHECK_ADDR_START, FLASH_CHECK_ADDR_START + 4, FLASH_CHECK_ADDR_START + 8, FLASH_CHECK_ADDR_START + 12};
#endif

#define FLASH_BOOTLOADER_ADDR_START ((uint32_t)0x08000000)
#define FLASH_BOOTLOADER_LENGTH_MAX ((uint32_t)0x0000C000)
static const uint32_t crc32tab[] = {
  0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
  0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
  0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
  0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
  0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
  0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
  0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
  0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
  0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
  0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
  0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
  0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
  0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
  0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
  0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
  0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
  0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
  0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
  0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
  0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
  0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
  0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
  0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
  0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
  0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
  0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
  0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
  0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
  0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
  0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
  0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
  0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
  0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
  0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
  0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
  0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
  0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
  0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
  0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
  0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
  0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
  0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
  0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
  0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
  0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
  0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
  0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
  0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
  0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
  0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
  0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
  0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
  0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
  0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
  0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
  0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
  0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
  0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
  0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
  0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
  0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
  0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
  0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
  0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

static unsigned int crc32(const unsigned char *buf, unsigned int size) {
  unsigned int i, crc;
  crc = 0xFFFFFFFF;

  for (i = 0; i < size; i++)
    crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);

  return crc ^ 0xFFFFFFFF;
}

void GcodeSuite::M3001() {
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  if (p_info->read_threshold_flag == 1)
    p_info->tx_ring_buf_add((char *)"M3001");
}

void GcodeSuite::M3002() {
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  if (p_info->read_threshold_flag == 1)
    p_info->tx_ring_buf_add((char *)"M3002");
}

// M3003 get nozzle board threshold
// M3003 V200 set nozzle board threshold
void GcodeSuite::M3003() {
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  char tmp_buf[ANKER_NOZZLE_BOARD_TX_BUF_SIZE] = {0};
  bool is_set_threshold = false,
       is_set_fireproof0 = false,
       is_set_fireproof1 = false;

  if (parser.seenval('V')) {
    p_info->threshold = parser.value_int();
    is_set_threshold = true;
  }
  if (parser.seenval('A')) {
    p_info->fireproof_adc0 = parser.value_int();
    is_set_fireproof0 = true;
  }
  if (parser.seenval('B')) {
    p_info->fireproof_adc1 = parser.value_int();
    is_set_fireproof1 = true;
  }

  if (!is_set_threshold && !is_set_fireproof0 && !is_set_fireproof1) {
    SERIAL_ECHOLNPGM("echo:M3003 V ", p_info->threshold);
    SERIAL_ECHOLNPGM("echo:M3003 A0 ", p_info->fireproof_adc0);
    SERIAL_ECHOLNPGM("echo:M3003 A1 ", p_info->fireproof_adc1);
  }

  if (is_set_threshold) {
    is_set_threshold = false;
    settings.save();
    SERIAL_ECHOLNPGM("echo:M3003 V", p_info->threshold);

    p_info->serial_begin();

    tmp_buf[0] = '\0';
    sprintf(tmp_buf, "M3003 %d", p_info->threshold);
    p_info->tx_ring_buf_add(tmp_buf);

    if (is_set_fireproof0 || is_set_fireproof1) {
      if (is_set_fireproof0) {
        is_set_fireproof0 = false;
        SERIAL_ECHOLNPGM("echo:M3008 ", p_info->fireproof_adc0);
        tmp_buf[0] = '\0';
        sprintf(tmp_buf, "M3008 %d", p_info->fireproof_adc0);
        p_info->tx_ring_buf_add(tmp_buf);
      }

      if (is_set_fireproof1) {
        is_set_fireproof1 = false;
        SERIAL_ECHOLNPGM("echo:M3009 ", p_info->fireproof_adc1);
        tmp_buf[0] = '\0';
        sprintf(tmp_buf, "M3009 %d", p_info->fireproof_adc1);
        p_info->tx_ring_buf_add(tmp_buf);
      }
    }

    if (p_info->threshold != 0) {
      p_info->read_threshold_flag = 0;
      tmp_buf[0] = '\0';
      sprintf(tmp_buf, "M3010");
      p_info->tx_ring_buf_add(tmp_buf);
    }
    else
      p_info->read_threshold_flag = 1;

    p_info->power_reset();
  }
  else
    SERIAL_ECHOLNPGM("echo:M3003 V", p_info->threshold);
}

// M3004 get nozzle board software version
void GcodeSuite::M3004() {
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

  if (strlen(p_info->software_version) > 0)
    SERIAL_ECHOLNPGM("echo:M3004 ", p_info->software_version);
  else
    SERIAL_ECHO_MSG("M3004 0.0.0");
}

// M3005 get abnormal_reset_times com_error_times
void GcodeSuite::M3005() {
  anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();
  SERIAL_ECHOLNPGM("echo:M3005,", p_info->heartbeat_abnormal_flag);
}

// M3009 bootloader check
void GcodeSuite::M3009() {
  uint8_t read_crc_flag = 0;
  uint32_t length = 0, crc_value = 0, calc_crc = 0;

  if (parser.seenval('L')) {
    length = parser.value_ulong();
    if (length > FLASH_BOOTLOADER_LENGTH_MAX) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }
  }
  if (parser.seenval('C')) {
    crc_value = parser.value_ulong();
    read_crc_flag = 0;
  }
  else
    read_crc_flag = 1;

  calc_crc = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
  if (read_crc_flag == 1)
    SERIAL_ECHOLNPGM("echo:M3009 success L", length, " C", calc_crc);
  else {
    if (calc_crc == crc_value)
      SERIAL_ECHOLNPGM("echo:M3009 success L", length, " C", crc_value);
    else
      SERIAL_ECHO_MSG("M3009 failure");
  }
}

#if 0

static bool flash_sector_read(uint32_t addr, uint32_t *data) {
  if ((addr < FLASH_CHECK_ADDR_START) || (addr > FLASH_CHECK_ADDR_END) || (data == NULL)) {
    SERIAL_ECHOLNPGM("flash_sector_read addr is error or data is NULL : ", addr);
    return false;
  }

  *data = *(__IO uint32_t *)addr;

  return true;
}

static bool flash_sector_erase() {
  FLASH_EraseInitTypeDef EraseInitStruct;
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t SectorError = 0;
  uint32_t tmp_addr = 0;
  uint32_t tmp_data = 0;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  HAL_FLASH_Unlock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  tmp_addr = FLASH_CHECK_ADDR_START;
  while (tmp_addr < FLASH_CHECK_ADDR_END) {
    if (flash_sector_read(tmp_addr, &tmp_data)) {
      if (tmp_data != 0XFFFFFFFF) {
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Sector = FLASH_SECTOR_10;
        EraseInitStruct.NbSectors = 1;

        DISABLE_ISRS();
        status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
        ENABLE_ISRS();

        if (status == HAL_OK) break;

        SERIAL_ECHOPGM(" HAL_FLASHEx_Erase = ", status);
        SERIAL_ECHOPGM(" GetError = ", HAL_FLASH_GetError());
        SERIAL_ECHOPGM(" SectorError = ", SectorError);
        HAL_FLASH_Lock();
        return false;
      }
      else
        tmp_addr += 4;

      FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
    }
    else {
      SERIAL_ECHOLNPGM("flash_sector_read failed!");
      HAL_FLASH_Lock();
      return false;
    }
  }

  HAL_FLASH_Lock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  return true;
}

static bool flash_sector_write(uint32_t addr, uint32_t data) {
  HAL_StatusTypeDef status = HAL_ERROR;

  if ((addr < FLASH_CHECK_ADDR_START) || (addr > FLASH_CHECK_ADDR_END)) {
    SERIAL_ECHOLNPGM("flash_sector_write addr is error : ", addr);
    return false;
  }

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  HAL_FLASH_Unlock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
  if (status != HAL_OK) {
    SERIAL_ECHOLNPGM(" HAL_FLASH_Program = ", status,
                      " GetError = ", HAL_FLASH_GetError(),
                      " address = ", addr);
    HAL_FLASH_Lock();
    return false;
  }

  HAL_FLASH_Lock();
  FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  return true;
}

void GcodeSuite::M3009() {
  uint8_t version = 0;
  uint32_t length = 0, crc_value = 0;

  if (parser.seen('A')) {
    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value)))
    ) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }

    SERIAL_ECHOLNPGM(
      "echo:M3009 A 1L", flash_check_info.init_version_length_value,
                  " 1C", flash_check_info.init_version_crc_value,
                  " 2L", flash_check_info.last_version_length_value,
                  " 2C", flash_check_info.last_version_crc_value
    );
  }
  else if (parser.seen('B')) {
    if (parser.seenval('L')) length = parser.value_ulong();

    if (length <= FLASH_BOOTLOADER_LENGTH_MAX) {
      crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
      SERIAL_ECHOLNPGM("echo:M3009 B L", length, " C", crc_value);
    }
    else {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }
  }
  else if (parser.seen('W')) {
    SERIAL_ECHOLN(parser.command_ptr);
    version = parser.value_byte();
    if (parser.seen('L')) {
      length = parser.value_ulong();
      SERIAL_ECHOLNPGM("length = ", length);
      if (length > FLASH_BOOTLOADER_LENGTH_MAX) {
        SERIAL_ECHO_MSG("M3009 failure");
        return;
      }
    }
    if (parser.seen('C')) {
      crc_value = parser.value_ulong();
      SERIAL_ECHOLNPGM("crc = ", crc_value);
    }

    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value)))
    ) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }

    if (version == 1) {
      flash_check_info.init_version_length_value = length;
      flash_check_info.init_version_crc_value = crc_value;
      SERIAL_ECHOLNPGM("length = ", flash_check_info.init_version_length_value, "crc = ", flash_check_info.init_version_crc_value);
    }
    else if (version == 2) {
      flash_check_info.last_version_length_value = length;
      flash_check_info.last_version_crc_value = crc_value;
    }

    if (!flash_sector_erase()) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }
    if ((!flash_sector_write(flash_check_info.init_version_length_addr, flash_check_info.init_version_length_value)) ||
        (!flash_sector_write(flash_check_info.init_version_crc_addr, flash_check_info.init_version_crc_value)) ||
        (!flash_sector_write(flash_check_info.last_version_length_addr, flash_check_info.last_version_length_value)) ||
        (!flash_sector_write(flash_check_info.last_version_crc_addr, flash_check_info.last_version_crc_value))
    ) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }

    SERIAL_ECHOLNPGM("echo:M3009 W", version, " L", length, " C", crc_value);
  }
  else {
    if ((!flash_sector_read(flash_check_info.init_version_length_addr, &(flash_check_info.init_version_length_value))) ||
        (!flash_sector_read(flash_check_info.init_version_crc_addr, &(flash_check_info.init_version_crc_value))) ||
        (!flash_sector_read(flash_check_info.last_version_length_addr, &(flash_check_info.last_version_length_value))) ||
        (!flash_sector_read(flash_check_info.last_version_crc_addr, &(flash_check_info.last_version_crc_value)))
    ) {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }

    length = flash_check_info.init_version_length_value;
    if (length <= FLASH_BOOTLOADER_LENGTH_MAX) {
      crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
      if (crc_value == flash_check_info.init_version_crc_value)
        SERIAL_ECHOLNPGM("echo:M3009 R1 L", length, " C", crc_value);
      else {
        length = flash_check_info.last_version_length_value;
        if (length <= FLASH_BOOTLOADER_LENGTH_MAX) {
          crc_value = crc32((unsigned char *)FLASH_BOOTLOADER_ADDR_START, length);
          if (crc_value == flash_check_info.last_version_crc_value)
            SERIAL_ECHOLNPGM("echo:M3009 R2 L", length, " C", crc_value);
          else
            SERIAL_ECHOLNPGM("echo:M3009 failure Rx L", length, " C", crc_value);
        }
        else {
          SERIAL_ECHO_MSG("M3009 failure");
          return;
        }
      }
    }
    else {
      SERIAL_ECHO_MSG("M3009 failure");
      return;
    }
  }
}

#endif // 0

#endif // ANKER_MAKE_API && ANKER_NOZZLE_BOARD
