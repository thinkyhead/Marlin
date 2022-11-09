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
 * @Date         : 2022-05-12 20:32:54
 * @LastEditors: winter.tian winter.tian@anker-in.com
 * @LastEditTime: 2022-09-16 16:57:38
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_NOZZLE_BOARD)

//#include "../../gcode/gcode.h"

typedef enum {
  ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE = 0,
  ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN = 1,
} anker_nozzle_board_serial_state;

typedef enum {
  TX_IDLE_STATE = 0,
  TX_START_STATE,
  TX_SUCCESS_STATE,
  TX_TIMEOUT_STATE,
  TX_ERROR_STATE,
} anker_nozzle_board_tx_state;

#define ANKER_NOZZLE_BOARD_TX_BUF_SIZE 32
#define ANKER_NOZZLE_BOARD_TX_CMD_NUM 16
typedef struct {
  uint8_t r_index;
  uint8_t w_index;
  char buf[ANKER_NOZZLE_BOARD_TX_CMD_NUM][ANKER_NOZZLE_BOARD_TX_BUF_SIZE];
} anker_nozzle_board_tx_cmd_ring_buf_t;

#define ANKER_NOZZLE_BOARD_RX_BUF_SIZE 256
typedef struct {
  uint8_t count;
  char buf[ANKER_NOZZLE_BOARD_RX_BUF_SIZE];
} anker_nozzle_board_rx_info_t;

typedef struct {
  char *cmd;
  char *keywords;
} anker_nozzle_board_cmd_params_t;

typedef struct {
  char *keywords;
  int (*cmd_handler)(anker_nozzle_board_cmd_params_t *);
  const char *brief;
} anker_nozzle_board_rx_cmd_array_t;

typedef struct {
  uint8_t serial_disable_state;
  uint8_t serial_state;
  char tx_buf[ANKER_NOZZLE_BOARD_TX_BUF_SIZE];
  anker_nozzle_board_tx_cmd_ring_buf_t tx_cmd_ring_buf;
  anker_nozzle_board_rx_info_t rx_info;
  anker_nozzle_board_cmd_params_t cmd_params;

  int threshold;
  int threshold_init_value;
  int threshold_cur_value;
  int fireproof_adc0;
  int fireproof_adc1;
  int z_sensorless_threshols;
  bool is_z_sensorless = false;

  uint8_t tx_deal_step;
  uint8_t tx_init_flag;
  uint8_t tx_state;
  uint8_t reset_flag;
  uint8_t read_threshold_flag;
  uint8_t com_abnormal_flag;
  char software_version[16];
  uint32_t tx_error_times;
  uint32_t abnormal_reset_times;
  uint8_t txrx_mode_flag;
  uint8_t heartbeat_flag;
  uint8_t heartbeat_abnormal_flag;
  uint8_t heartbeat_deal_step;

  void (*power_reset)();
  void (*serial_begin)();
  void (*serial_end)();
  uint8_t (*tx_ring_buf_available)();
  int (*tx_ring_buf_read)(char *buf, uint8_t size);
  int (*tx_ring_buf_write)(char *buf, uint8_t len);
  int (*tx_ring_buf_add)(char *cmd);
  void (*tx_deal)();
  void (*rx_deal)();
  void (*find_cmd_handler)(anker_nozzle_board_cmd_params_t *);
  void (*heartbeat_deal)();
  void (*nozzle_board_deal)();

} anker_nozzle_board_info_t;

void anker_nozzle_board_init();
void set_anker_z_sensorless_probe_value(int value);
void reset_anker_z_sensorless_probe_value();
anker_nozzle_board_info_t *get_anker_nozzle_board_info();

#endif // ANKER_NOZZLE_BOARD
