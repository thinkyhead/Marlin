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

/**
 * temperature_anker.cpp - AnkerMake Temperature Add-ons
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(ANKER_TEMP_WATCH)

#include "temperature.h"

#if ENABLED(ANKERUI)
  #include "src/user/marlin_api.h"
#endif

#include "settings.h"
#if ENABLED(ANKER_NOZZLE_BOARD)
  #include "../feature/anker/anker_nozzle_board.h"
#endif

#define TEMP_WATCH_ERROR 0x5A00

#if HAS_HOTEND
  anker_hotend_watch_t Temperature::anker_watch_hotend[HOTENDS];

  celsius_float_t Temperature::hotend_maxtemp_pre_value = 0.0;
  uint8_t Temperature::hotend_maxtemp_pre_stable_cnt = 0;

  uint8_t Temperature::hotend_minraw_err_cnt[HOTENDS] = { 0 };
  uint8_t Temperature::hotend_maxraw_err_cnt[HOTENDS] = { 0 };
  uint8_t Temperature::hotend_maxtemp_err_cnt[HOTENDS] = { 0 };

  uint8_t Temperature::hotend_mos2_temp_watch_deal_step = 0;
  uint8_t Temperature::hotend_mos2_deal_enable_flag = 0;
  uint8_t Temperature::hotend_temp_shock_process_step = 0;
  uint8_t Temperature::hotend_temp_slide_window_process_step = 0;
#endif

#if HAS_HEATED_BED
  uint8_t Temperature::bed_minraw_err_cnt = 0;
  uint8_t Temperature::bed_maxraw_err_cnt = 0;
  uint8_t Temperature::bed_maxtemp_err_cnt = 0;

  uint8_t Temperature::bed_mos2_temp_watch_deal_step = 0;
  uint8_t Temperature::bed_mos2_deal_enable_flag = 0;
  uint8_t Temperature::bed_temp_shock_process_step = 0;
  uint8_t Temperature::bed_temp_slide_window_process_step = 0;
#endif

bool Temperature::temp_watch_is_error() {
  return ((thermalManager.temp_watch_error_flag & 0xff00) == TEMP_WATCH_ERROR);
}

void Temperature::_hotend_temp_watch() {
  static int err_count = 0;
  static bool start = 0;
  static celsius_float_t base_temp = 0.0;
  celsius_float_t cur_temp =0, target_temp = 0;
  cur_temp = degHotend(0);
  target_temp = degTargetHotend(0);

  if ((cur_temp > 80 && cur_temp > target_temp + 30) && (target_temp < 30)) {
    if (cur_temp >= base_temp + 3) {
      if ((!start)) {
        base_temp = cur_temp ;
        start = true;
      }
      err_count++;
      MYSERIAL2.printf("hotend: err_count: %d, cur_temp: %f, target_temp: %f, base_temp: %f\r\n", err_count, cur_temp, target_temp, base_temp);
    }
    else
      err_count = 0;
  }
  else {
    err_count = 0;
    base_temp = 0;
    start     = 0;
  }

  if (err_count > 15 && (cur_temp >= base_temp + 30 && cur_temp < HEATER_0_MAXTEMP)) {
    thermalManager.temp_watch_error_flag |= TEMP_WATCH_ERROR;
    hotend_mos2_deal_enable_flag = 1;
    err_count = 0;
    base_temp = 0;
    start     = 0;
  }
}

void Temperature::_hotend_mos2_temp_watch() {
  static int time_count = 0;
  static celsius_float_t base_temp = 0.0;
  celsius_float_t cur_temp =0;

  switch(hotend_mos2_temp_watch_deal_step) {
    case 0: {
      if (hotend_mos2_deal_enable_flag == 1) {
        hotend_mos2_deal_enable_flag = 0;
        base_temp = degHotend(0);
        cur_temp = base_temp;
        time_count = 0;
        hotend_mos2_temp_watch_deal_step++;
        MYSERIAL2.printf("hotend: base_temp=%f, cur_temp=%f\r\n", base_temp, cur_temp);
      }
      break;
    }
    case 1: {
      cur_temp = degHotend(0);
      time_count++;
      MYSERIAL2.printf("hotend: time_count=%d, cur_temp=%f\r\n", time_count, cur_temp);
      if (time_count >= 20) {
        if ((cur_temp > base_temp + 30) && cur_temp <= HEATER_0_MAXTEMP)
          thermalManager.temp_watch_error_flag |= 0x0002;
        else
          thermalManager.temp_watch_error_flag |= 0x0001;

        if (thermalManager.temp_watch_error_flag & 0x0002) {
          settings.save();
          _delay_ms(100);
        }
        hotend_mos2_temp_watch_deal_step++;
      }
      break;
    }
    case 2: {
      cur_temp = degHotend(0);
      time_count++;
      MYSERIAL2.printf("hotend: cnt=%d, cur=%f, base=%f\r\n", time_count, cur_temp, base_temp);
      if (time_count >= 5) {
        time_count = 0;
        MYSERIAL2.printf("temp_watch_error_flag: %#x\r\n", thermalManager.temp_watch_error_flag);
        if (thermalManager.temp_watch_error_flag & 0x0002) {
          MYSERIAL2.printf("TempErrorCode:1001\r\n");
          MYSERIAL2.printf("Error:Demage:2 hotend\r\n");
        }
        else if (thermalManager.temp_watch_error_flag & 0x0001) {
          MYSERIAL2.printf("TempErrorCode:1000\r\n");
          MYSERIAL2.printf("Error:Demage:1 hotend\r\n");
        }
      }
      break;
    }
    default: {
      hotend_mos2_temp_watch_deal_step = 0;
      break;
    }
  }
}

void Temperature::_bed_temp_watch() {
  static int err_count = 0;
  static bool start = 0;
  static celsius_float_t base_temp = 0.0;
  celsius_float_t cur_temp =0, target_temp = 0;
  cur_temp = degBed();
  target_temp = degTargetBed();

  if ((cur_temp > 50 && cur_temp > target_temp + 20) && (target_temp < 30)) {
    if (cur_temp >= base_temp + 3) {
      if (!start) {
        base_temp = cur_temp;
        start = true;
      }
      err_count++;
      MYSERIAL2.printf("bed: err_count: %d, cur_temp: %f, target_temp: %f, base_temp: %f\r\n", err_count, cur_temp, target_temp, base_temp);
    }
    else
      err_count = 0;
  }
  else {
    err_count = 0;
    base_temp = 0;
    start     = 0;
  }

  if (err_count > 15  && (cur_temp >= base_temp + 10 && cur_temp < BED_MAXTEMP)) {
    thermalManager.temp_watch_error_flag |= TEMP_WATCH_ERROR;
    bed_mos2_deal_enable_flag = 1;
    err_count = 0;
    base_temp = 0;
    start     = 0;
  }
}

void Temperature::_bed_mos2_temp_watch() {
  static int time_count = 0;
  static celsius_float_t base_temp = 0.0;
  celsius_float_t cur_temp =0;

  switch(bed_mos2_temp_watch_deal_step) {
    case 0: {
      if (bed_mos2_deal_enable_flag == 1) {
        bed_mos2_deal_enable_flag = 0;
        base_temp = degBed();
        cur_temp = base_temp;
        time_count = 0;
        bed_mos2_temp_watch_deal_step++;
        MYSERIAL2.printf("bed: base_temp=%f, cur_temp=%f\r\n", base_temp, cur_temp);
      }
    } break;

    case 1: {
      cur_temp = degBed();
      time_count++;
      MYSERIAL2.printf("bed: time_count=%d, cur_temp=%f\r\n", time_count, cur_temp);
      if (time_count >= 30) {
        if ((cur_temp > base_temp + 3) && cur_temp <= BED_MAXTEMP)
          thermalManager.temp_watch_error_flag |= 0x0020;
        else
          thermalManager.temp_watch_error_flag |= 0x0010;

        if (thermalManager.temp_watch_error_flag & 0x0020) {
          settings.save();
          _delay_ms(100);
        }
        bed_mos2_temp_watch_deal_step++;
      }
    } break;

    case 2: {
      cur_temp = degBed();
      time_count++;
      MYSERIAL2.printf("bed: cnt=%d, cur=%f, base=%f\r\n", time_count, cur_temp, base_temp);
      if (time_count >= 5) {
        time_count = 0;
        MYSERIAL2.printf("temp_watch_error_flag: %#x\r\n", thermalManager.temp_watch_error_flag);
        if (thermalManager.temp_watch_error_flag & 0x0020) {
          MYSERIAL2.printf("TempErrorCode:1012\r\n");
          MYSERIAL2.printf("Error:Demage:2 bed\r\n");
        }
        else if (thermalManager.temp_watch_error_flag & 0x0010) {
          MYSERIAL2.printf("TempErrorCode:1011\r\n");
          MYSERIAL2.printf("Error:Demage:1 bed\r\n");
        }
      }
    } break;

    default:
      bed_mos2_temp_watch_deal_step = 0;
      break;
  }
}

#define IS_NEW_MARLIN_NEW_NOZZLE  (hw_ver_read() && IS_new_nozzle_board())
#define IS_NEW_MARLIN_OLD_NOZZLE  (hw_ver_read() && !IS_new_nozzle_board())
#define IS_OLD_MARLIN_NEW_NOZZLE  (!hw_ver_read() && IS_new_nozzle_board())
#define IS_OLD_MARLIN_OLD_NOZZLE  (!hw_ver_read() && !IS_new_nozzle_board())

void Temperature::_temp_watch() {
  static uint8_t report_time_count = 0;
  static uint8_t report_count = 0;
  static millis_t timeout = millis();
  static uint8_t heater_bed_ctrl2_enable = 0;

  if ((millis() - timeout) > 1000) {
    timeout = millis();
    if (temp_watch_is_error()) {
      disable_all_heaters();
      #if PIN_EXISTS(HEATER_BED_CTRL_2)
        OUT_WRITE(HEATER_BED_CTRL_2_PIN, hw_ver_read() ? HEATER_BED_CTRL_2_INVERTING : !HEATER_BED_CTRL_2_INVERTING);
      #endif
      heater_bed_ctrl2_enable = 0;
      get_anker_nozzle_board_info()->power_off();
      report_time_count++;
      if (report_time_count >= 5) {
        report_time_count = 0;
        if (report_count < 10) {
          report_count++;
          MYSERIAL2.printf("temp_watch_error_flag: %#x\r\n", thermalManager.temp_watch_error_flag);
          if (IS_OLD_MARLIN_OLD_NOZZLE) {
            MYSERIAL2.printLine("OLD_MARLIN_OLD_NOZZLE\n");
            if (thermalManager.temp_watch_error_flag & 0x0002) {
              MYSERIAL2.printf("TempErrorCode:1001\r\n");
              MYSERIAL2.printf("Error:Demage:2 hotend\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0001) {
              MYSERIAL2.printf("TempErrorCode:1000\r\n");
              MYSERIAL2.printf("Error:Demage:1 hotend\r\n");
            }
            if (thermalManager.temp_watch_error_flag & 0x0020) {
              MYSERIAL2.printf("TempErrorCode:1012\r\n");
              MYSERIAL2.printf("Error:Demage:2 bed\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0010) {
              MYSERIAL2.printf("TempErrorCode:1011\r\n");
              MYSERIAL2.printf("Error:Demage:1 bed\r\n");
            }
          }
          else if (IS_OLD_MARLIN_NEW_NOZZLE) {
            MYSERIAL2.printLine("OLD_MARLIN_NEW_NOZZLE\n");
            if (thermalManager.temp_watch_error_flag & 0x0020) {
              MYSERIAL2.printf("TempErrorCode:1012\r\n");
              MYSERIAL2.printf("Error:Demage:2 bed\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0010) {
              MYSERIAL2.printf("TempErrorCode:1011\r\n");
              MYSERIAL2.printf("Error:Demage:1 bed\r\n");
            }
          }
          else if (IS_NEW_MARLIN_OLD_NOZZLE) {
            MYSERIAL2.printLine("NEW_MARLIN_OLD_NOZZLE\n");
            if (thermalManager.temp_watch_error_flag & 0x0002) {
              MYSERIAL2.printf("TempErrorCode:1001\r\n");
              MYSERIAL2.printf("Error:Demage:2 hotend\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0001) {
              MYSERIAL2.printf("TempErrorCode:1000\r\n");
              MYSERIAL2.printf("Error:Demage:1 hotend\r\n");
            }
            if (thermalManager.temp_watch_error_flag & 0x0002) {
              MYSERIAL2.printf("TempErrorCode:1001\r\n");
              MYSERIAL2.printf("Error:Demage:2 hotend\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0001) {
              MYSERIAL2.printf("TempErrorCode:1000\r\n");
              MYSERIAL2.printf("Error:Demage:1 hotend\r\n");
            }
            if (thermalManager.temp_watch_error_flag & 0x0020) {
              MYSERIAL2.printf("TempErrorCode:1012\r\n");
              MYSERIAL2.printf("Error:Demage:2 bed\r\n");
            }
            else if (thermalManager.temp_watch_error_flag & 0x0010) {
              MYSERIAL2.printf("TempErrorCode:1011\r\n");
              MYSERIAL2.printf("Error:Demage:1 bed\r\n");
            }
          }
        }
        else {
          MYSERIAL2.printf("heater off and nozzle power off!\r\n");
        }
      }
      if (IS_OLD_MARLIN_OLD_NOZZLE) {
        _hotend_mos2_temp_watch();
        _bed_mos2_temp_watch();
      }
      else if (IS_OLD_MARLIN_NEW_NOZZLE)
        _bed_mos2_temp_watch();
      else if (IS_NEW_MARLIN_OLD_NOZZLE)
        _hotend_mos2_temp_watch();
    }
    else {
      #if PIN_EXISTS(HEATER_BED_CTRL_2)
        if (heater_bed_ctrl2_enable == 0) {
          OUT_WRITE(HEATER_BED_CTRL_2_PIN, hw_ver_read() ? !HEATER_BED_CTRL_2_INVERTING : HEATER_BED_CTRL_2_INVERTING);
          heater_bed_ctrl2_enable = 1;
        }
      #endif
      if (IS_OLD_MARLIN_OLD_NOZZLE) {
        _hotend_temp_watch();
        _bed_temp_watch();
      }
      else if (IS_OLD_MARLIN_NEW_NOZZLE)
        _bed_temp_watch();
      else if (IS_NEW_MARLIN_OLD_NOZZLE)
        _hotend_temp_watch();

      report_time_count = 0;
      report_count = 0;
      hotend_mos2_temp_watch_deal_step = 0;
      bed_mos2_temp_watch_deal_step = 0;
    }
  }
}

#define CUR_AND_LAST_CELSIUS_DIFF_VALUE 5
#define CUR_CELSIUS_ERROR_CNT_MAX 20
void Temperature::hotend_temp_heating_process() {
  static celsius_float_t last_celsius = CUR_AND_LAST_CELSIUS_DIFF_VALUE;
  static uint8_t cur_celsius_error_cnt = 0;

  if ((temp_hotend[0].target > 0) && (temp_hotend[0].celsius < temp_hotend[0].target - TEMP_HYSTERESIS -1)) {
    if (temp_hotend[0].celsius < last_celsius - CUR_AND_LAST_CELSIUS_DIFF_VALUE) {
      cur_celsius_error_cnt++;
      MYSERIAL2.printf("hotend: cur_celsius error: cnt %d, cur %f, last %f\r\n", cur_celsius_error_cnt, temp_hotend[0].celsius, last_celsius);
      temp_hotend[0].celsius = last_celsius;
      if (cur_celsius_error_cnt >= CUR_CELSIUS_ERROR_CNT_MAX) {
        _temp_error((heater_id_t)0, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
      }
    }
    else {
      cur_celsius_error_cnt = 0;
      last_celsius = temp_hotend[0].celsius;
    }
  }
  else {
    cur_celsius_error_cnt = 0;
    last_celsius = CUR_AND_LAST_CELSIUS_DIFF_VALUE;
  }
}

void Temperature::bed_temp_heating_process() {
  static celsius_float_t last_celsius = CUR_AND_LAST_CELSIUS_DIFF_VALUE;
  static uint8_t cur_celsius_error_cnt = 0;

  if ((temp_bed.target > 0) && (temp_bed.celsius < temp_bed.target - TEMP_BED_HYSTERESIS -1)) {
    if (temp_bed.celsius < last_celsius - CUR_AND_LAST_CELSIUS_DIFF_VALUE) {
      cur_celsius_error_cnt++;
      MYSERIAL2.printf("bed: cur_celsius error: cnt %d, cur %f, last %f\r\n", cur_celsius_error_cnt, temp_bed.celsius, last_celsius);
      temp_bed.celsius = last_celsius;
      if (cur_celsius_error_cnt >= CUR_CELSIUS_ERROR_CNT_MAX)
        _temp_error(H_BED, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
    }
    else {
      cur_celsius_error_cnt = 0;
      last_celsius = temp_bed.celsius;
    }
  }
  else {
    cur_celsius_error_cnt = 0;
    last_celsius = CUR_AND_LAST_CELSIUS_DIFF_VALUE;
  }
}

#define HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE WATCH_TEMP_PERIOD
#define BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE WATCH_BED_TEMP_PERIOD
void Temperature::hotend_temp_slide_window_process() {
  static millis_t timer = millis();
  static uint8_t abnormal_temp_rise_cnt = 0;
  static uint8_t read_index=0, write_index=0, tmp_index=0, len=0;
  static celsius_float_t celsius_buf[HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE];

  if ((temp_hotend[0].target > 0) && (temp_hotend[0].celsius < temp_hotend[0].target - TEMP_HYSTERESIS -1)) {
    switch(hotend_temp_slide_window_process_step) {
      case 0: {
        len = 0;
        read_index = 0;
        write_index = 0;
        abnormal_temp_rise_cnt = 0;
        timer = millis() + SEC_TO_MS(1);
        hotend_temp_slide_window_process_step++;
      } break;

      case 1: {
        if (ELAPSED(millis(), timer)) {
          if (len < HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE) {
            celsius_buf[write_index] = temp_hotend[0].celsius;
            write_index = (write_index + 1) % HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
            len++;
            timer = millis() + SEC_TO_MS(1);
          }
          else
            hotend_temp_slide_window_process_step++;
        }
      } break;

      case 2: {
        tmp_index = (read_index + HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE - 1) % HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
        if (celsius_buf[tmp_index] - celsius_buf[read_index] >= WATCH_TEMP_INCREASE) {
          abnormal_temp_rise_cnt = 0;
        }
        else {
          abnormal_temp_rise_cnt++;
          MYSERIAL2.printf("hotend: slide_window error: cnt %d, pre %.2f, cur %.2f\r\n", abnormal_temp_rise_cnt, celsius_buf[read_index], celsius_buf[tmp_index]);
          if (abnormal_temp_rise_cnt >= WATCH_TEMP_PERIOD/4) {
            MYSERIAL2.printf("TempErrorCode:1008\r\n");
            _temp_error((heater_id_t)0, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
          }
        }
        read_index = (read_index + 1) % HOTEND_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
        if (len > 0) len--;
        timer = millis() + SEC_TO_MS(1);
        hotend_temp_slide_window_process_step = 1;
      } break;

      default:
        hotend_temp_slide_window_process_step = 0;
        break;
    }
  }
  else
    hotend_temp_slide_window_process_step = 0;
}

void Temperature::bed_temp_slide_window_process() {
  static millis_t timer = millis();
  static uint8_t abnormal_temp_rise_cnt = 0;
  static uint8_t read_index=0, write_index=0, tmp_index=0, len=0;
  static celsius_float_t celsius_buf[BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE];

  if ((temp_bed.target > 0) && (temp_bed.celsius < temp_bed.target - TEMP_BED_HYSTERESIS - 1)) {
    switch(bed_temp_slide_window_process_step) {
      case 0: {
        len = 0;
        read_index = 0;
        write_index = 0;
        abnormal_temp_rise_cnt = 0;
        timer = millis() + SEC_TO_MS(1);
        bed_temp_slide_window_process_step++;
      } break;

      case 1: {
        if (ELAPSED(millis(), timer)) {
          if (len < BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE) {
            celsius_buf[write_index] = temp_bed.celsius;
            write_index = (write_index + 1) % BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
            len++;
            timer = millis() + SEC_TO_MS(1);
          }
          else
            bed_temp_slide_window_process_step++;
        }
      } break;

      case 2: {
        tmp_index = (read_index+BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE-1)%BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
        if (celsius_buf[tmp_index] - celsius_buf[read_index] >= WATCH_BED_TEMP_INCREASE) {
          abnormal_temp_rise_cnt = 0;
        }
        else {
          abnormal_temp_rise_cnt++;
          MYSERIAL2.printf("bed: slide_window error: cnt %d, pre %.2f, cur %.2f\r\n", abnormal_temp_rise_cnt, celsius_buf[read_index], celsius_buf[tmp_index]);
          if (abnormal_temp_rise_cnt >= WATCH_BED_TEMP_PERIOD / 4) {
            MYSERIAL2.printf("TempErrorCode:1018\r\n");
            _temp_error(H_BED, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
          }
        }
        read_index = (read_index + 1) % BED_CELSIUS_SLIDE_WINDOW_BUF_SIZE;
        if (len > 0) len--;
        timer = millis() + SEC_TO_MS(1);
        bed_temp_slide_window_process_step = 1;
      } break;

      default:
        bed_temp_slide_window_process_step = 0;
        break;
    }
  }
  else
    bed_temp_slide_window_process_step = 0;
}

#define HOTEND_SEGMENTATION_INIT_TEMP                  100
#define HOTEND_SEGMENTATION_INTERVAL_TEMP                5
#define HOTEND_SEGMENTATION_INTERVAL_TIME                6
#define HOTEND_SEGMENTATION_CUR_CELSIUS_STABLE_CNT_MAX  20
#define HOTEND_SEGMENTATION_HEATING_ERROR_CNT_MAX        2
#define HOTEND_SEGMENTATION_TOTAL_TIME_MAX              80
void Temperature::hotend_segmentation_heating_process() {
  static uint8_t process_step = 0;
  static uint16_t seg_time_cnt = 0;
  static uint16_t total_time_cnt = 0;
  static millis_t timer = millis();
  static celsius_float_t last_celsius = 0.0;
  static uint8_t cur_celsius_stable_cnt = 0;
  static uint8_t seg_heating_err_cnt = 0;

  if ( (temp_hotend[0].target >= HOTEND_SEGMENTATION_INIT_TEMP) &&
       (temp_hotend[0].celsius >= HOTEND_SEGMENTATION_INIT_TEMP) &&
       (temp_hotend[0].celsius < temp_hotend[0].target - PID_FUNCTIONAL_RANGE)
  ) {
    switch (process_step) {
      case 0: {
        seg_time_cnt = 0;
        total_time_cnt = 0;
        cur_celsius_stable_cnt = 0;
        seg_heating_err_cnt = 0;
        process_step++;
        last_celsius = temp_hotend[0].celsius;
        break;
      }
      case 1: {
        if (temp_hotend[0].celsius > last_celsius + 3) {
          cur_celsius_stable_cnt++;
          if (cur_celsius_stable_cnt >= HOTEND_SEGMENTATION_CUR_CELSIUS_STABLE_CNT_MAX) {
            cur_celsius_stable_cnt = 0;
            timer = millis() + SEC_TO_MS(1);
            last_celsius = temp_hotend[0].celsius;
            process_step++;
          }
        }
        else
          cur_celsius_stable_cnt = 0;

      } break;

      case 2: {
        if (ELAPSED(millis(), timer)) {
          seg_time_cnt++;
          // MYSERIAL2.printf("hotend: segment: time_cnt %d, last %.2f, cur %.2f, tar %.2f, total_time %d\r\n",
          //                     seg_time_cnt, last_celsius, temp_hotend[0].celsius, (float)temp_hotend[0].target, total_time_cnt);
          if (temp_hotend[0].celsius - last_celsius >= HOTEND_SEGMENTATION_INTERVAL_TEMP) {
            total_time_cnt += seg_time_cnt;
            last_celsius = temp_hotend[0].celsius;
            seg_heating_err_cnt = 0;
            seg_time_cnt = 0;
          }
          else {
            if (seg_time_cnt >= HOTEND_SEGMENTATION_INTERVAL_TIME) {
              total_time_cnt += seg_time_cnt;
              seg_heating_err_cnt++;
              MYSERIAL2.printf("hotend: segment error: cnt %d, last %.2f, cur %.2f, tar %.2f, total_time %d\r\n",
                              seg_heating_err_cnt, last_celsius, temp_hotend[0].celsius, (float)temp_hotend[0].target, total_time_cnt);
              if (seg_heating_err_cnt >= HOTEND_SEGMENTATION_HEATING_ERROR_CNT_MAX) {
                MYSERIAL2.printf("TempErrorCode:1010\r\n");
                _temp_error((heater_id_t)0, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
              }
              last_celsius = temp_hotend[0].celsius;
              seg_time_cnt = 0;
            }
          }
          if (total_time_cnt > HOTEND_SEGMENTATION_TOTAL_TIME_MAX) {
            MYSERIAL2.printf("hotend: segment total_time error: total_time %d\r\n", total_time_cnt);
            MYSERIAL2.printf("TempErrorCode:1010\r\n");
            _temp_error((heater_id_t)0, str_t_heating_failed, GET_TEXT(MSG_HEATING_FAILED_LCD));
          }
          timer = millis() + SEC_TO_MS(1);
        }
      } break;

      default:
        process_step = 0;
        break;
    }
  }
  else
    process_step = 0;

}

#define TARGET_TEMP_STABLE_CNT_MAX 20
#define HOTEND_TEMP_SHOCK_VALUE    15
#define BED_TEMP_SHOCK_VALUE       10
#define TEMP_SHOCK_CNT_MAX         50
void Temperature::hotend_temp_shock_process() {
  static celsius_float_t last_celsius = 0.0;
  static uint8_t target_temp_stable_cnt = 0;
  static uint32_t temp_shock_cnt = 0;
  static celsius_t cur_target = 0;
  static bool target_change_flag = false;
  static uint8_t temp_error_count = 0;
  static millis_t timer = millis();
  static celsius_t last_target = 0;

  if (cur_target != temp_hotend[0].target) {
    cur_target = temp_hotend[0].target;
    target_change_flag = true;
  }

  switch (hotend_temp_shock_process_step) {
    case 0: {
      if ((temp_hotend[0].target > 0) &&
          (temp_hotend[0].celsius >= temp_hotend[0].target - TEMP_HYSTERESIS) &&
          (temp_hotend[0].celsius <= temp_hotend[0].target + TEMP_HYSTERESIS)
      ) {
        if (++target_temp_stable_cnt >= TARGET_TEMP_STABLE_CNT_MAX) {
          temp_shock_cnt = 0;
          target_temp_stable_cnt = 0;
          target_change_flag = false;
          last_celsius = temp_hotend[0].celsius;
          hotend_temp_shock_process_step++;
          MYSERIAL2.printf("hotend: shock start: cur %.2f, tar %.2f\r\n", temp_hotend[0].celsius, (float)temp_hotend[0].target);
        }
      }
      else
        target_temp_stable_cnt = 0;
    } break;

    case 1: {
      if (target_change_flag == false) {
        if ((temp_hotend[0].celsius < temp_hotend[0].target - HOTEND_TEMP_SHOCK_VALUE) ||
            (temp_hotend[0].celsius > temp_hotend[0].target + HOTEND_TEMP_SHOCK_VALUE)
        ) {
          temp_shock_cnt++;
          MYSERIAL2.printf("hotend: shock error: cnt %d, cur %.2f, tar %.2f, last %.2f\r\n", temp_shock_cnt, temp_hotend[0].celsius, (float)temp_hotend[0].target, last_celsius);
          if (temp_shock_cnt >= TEMP_SHOCK_CNT_MAX) {
            MYSERIAL2.printf("TempErrorCode:1009\r\n");
            temp_error_count++;
            MYSERIAL2.printf("hotend: shock error: temp_error_count %d\r\n", temp_error_count);
            if (temp_error_count >= 5)
              _temp_error((heater_id_t)0, str_t_thermal_runaway, GET_TEXT(MSG_THERMAL_RUNAWAY));
            else
              hotend_temp_shock_process_step++;
          }
          target_temp_stable_cnt = 0;
          // temp_hotend[0].celsius = last_celsius;
        }
        else {
          target_temp_stable_cnt++;
          if (target_temp_stable_cnt >= TARGET_TEMP_STABLE_CNT_MAX) {
            temp_shock_cnt = 0;
            target_temp_stable_cnt = 0;
            last_celsius = temp_hotend[0].celsius;
          }
        }
      }
      else {
        temp_shock_cnt = 0;
        target_temp_stable_cnt = 0;
        hotend_temp_shock_process_step = 0;
      }
    } break;

    case 2: {
      last_target = temp_hotend[0].target;
      setTargetHotend(0, 0);
      timer = millis() + SEC_TO_MS(3);
      hotend_temp_shock_process_step++;
    } break;

    case 3: {
      if (ELAPSED(millis(), timer)) {
        setTargetHotend(last_target, 0);
        hotend_temp_shock_process_step = 0;
      }
    } break;

    default:
      hotend_temp_shock_process_step = 0;
      break;
  }
}

void Temperature::bed_temp_shock_process() {
  static celsius_float_t last_celsius = 0.0;
  static uint8_t target_temp_stable_cnt = 0;
  static uint32_t temp_shock_cnt = 0;
  static celsius_t cur_target = 0;
  static bool target_change_flag = false;

  if (cur_target != temp_bed.target) {
    cur_target = temp_bed.target;
    target_change_flag = true;
  }

  switch (bed_temp_shock_process_step) {
    case 0: {
      if ((temp_bed.target > 0) &&
          (temp_bed.celsius >= temp_bed.target - TEMP_BED_HYSTERESIS) &&
          (temp_bed.celsius <= temp_bed.target + TEMP_BED_HYSTERESIS)
      ) {
        target_temp_stable_cnt++;
        if (target_temp_stable_cnt >= TARGET_TEMP_STABLE_CNT_MAX) {
          temp_shock_cnt = 0;
          target_temp_stable_cnt = 0;
          target_change_flag = false;
          last_celsius = temp_bed.celsius;
          bed_temp_shock_process_step++;
          MYSERIAL2.printf("bed: shock start: cur %.2f, tar %.2f\r\n", temp_bed.celsius, (float)temp_bed.target);
        }
      }
      else
        target_temp_stable_cnt = 0;

    } break;

    case 1: {
      if (target_change_flag == false) {
        if ((temp_bed.celsius < temp_bed.target - BED_TEMP_SHOCK_VALUE) ||
            (temp_bed.celsius > temp_bed.target + BED_TEMP_SHOCK_VALUE)
        ) {
          temp_shock_cnt++;
          MYSERIAL2.printf("bed: shock error: cnt %d, cur %.2f, tar %.2f, last %.2f\r\n", temp_shock_cnt, temp_bed.celsius, (float)temp_bed.target, last_celsius);
          // if (temp_shock_cnt >= TEMP_SHOCK_CNT_MAX)
          // {
          //   MYSERIAL2.printf("TempErrorCode:1019\r\n");
          //   _temp_error(H_BED, str_t_thermal_runaway, GET_TEXT(MSG_THERMAL_RUNAWAY));
          // }
          target_temp_stable_cnt = 0;
          // temp_bed.celsius = last_celsius;
        }
        else {
          target_temp_stable_cnt++;
          if (target_temp_stable_cnt >= TARGET_TEMP_STABLE_CNT_MAX) {
            target_temp_stable_cnt = 0;
            last_celsius = temp_bed.celsius;
          }
        }
      }
      else {
        temp_shock_cnt = 0;
        target_temp_stable_cnt = 0;
        bed_temp_shock_process_step = 0;
      }
    } break;

    default:
      bed_temp_shock_process_step = 0;
      break;
  }
}

void Temperature::temp_protect_process() {
  if (IS_old_nozzle_board()) {
    // hotend_temp_heating_process();
    hotend_temp_slide_window_process();
    hotend_segmentation_heating_process();
    hotend_temp_shock_process();
  }

  // bed_temp_heating_process();
  bed_temp_slide_window_process();
  bed_temp_shock_process();
}

#endif // ANKER_TEMP_WATCH
