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

/**
 * DWIN by Creality3D
 */

#include "../dwin_lcd.h"
#include "rotary_encoder.h"
#include "../../../libs/BL24CXX.h"

#include <stdint.h>

#include "../../../inc/MarlinConfig.h"

enum processID {
  // Process ID
  MainMenu,
  SelectFile,
  Prepare,
  Control,
  Leveling,
  PrintProcess,
  AxisMove,
  TemperatureID,
  Motion,
  Info,
  Tune,
  #if HAS_HOTEND
    PLAPreheat,
    ABSPreheat,
  #endif
  MaxSpeed,
  MaxSpeed_value,
  MaxAcceleration,
  MaxAcceleration_value,
  MaxCorner,
  MaxCorner_value,
  Step,
  Step_value,

  // Last Process ID
  Last_Prepare,

  // Back Process ID
  Back_Main,
  Back_Print,

  // Date variable ID
  Move_X,
  Move_Y,
  Move_Z,
  #if HAS_HOTEND
    Extruder,
    ETemp,
  #endif
  Homeoffset,
  #if HAS_HEATED_BED
    BedTemp,
  #endif
  #if HAS_FAN
    FanSpeed,
  #endif
  PrintSpeed,

  // Window ID
  Print_window,
  Popup_Window
};

extern int checkkey, last_checkkey;
extern float zprobe_zoffset;
extern char print_filename[16];

extern millis_t dwin_heat_time;

typedef struct {
  TERN_(HAS_HOTEND,     int16_t E_Temp    = 0);
  TERN_(HAS_HEATED_BED, int16_t Bed_Temp  = 0);
  TERN_(HAS_FAN,        int16_t Fan_speed = 0);
  int16_t print_speed     = 100;
  float Max_Feedspeed     = 0;
  float Max_Acceleration  = 0;
  float Max_Corner        = 0;
  float Max_Step          = 0;
  float Move_X_scale      = 0;
  float Move_Y_scale      = 0;
  float Move_Z_scale      = 0;
  #if EXTRUDERS
    float Move_E_scale    = 0;
  #endif
  float offset_value      = 0;
  char show_mode          = 0;    // -1: Temperature control    0: Printing temperature
} HMI_value_t;

typedef struct {
  bool language_chinese;  // 0: EN, 1: CN
  bool pause_flag:1;
  bool print_finish:1;
  bool confirm_flag:1;
  bool select_flag:1;
  bool home_flag:1;
  bool heat_flag:1;  // 0: heating done  1: during heating
  #if HAS_HOTEND
    bool ETempTooLow_flag:1;
  #endif
  #if HAS_LEVELING
    bool leveling_offset_flag:1;
  #endif
  #if HAS_FAN
    char feedspeed_flag;
  #endif
  char acc_flag;
  char corner_flag;
  char step_flag;
} HMI_Flag;

extern HMI_value_t HMI_ValueStruct;
extern HMI_Flag    HMI_flag;

// Language
void HMI_SetLanguage(void);
void HMI_SetAndSaveLanguageWestern(void);
void HMI_SetAndSaveLanguageChinese(void);

// Show ICO
void ICON_Print(bool show);
void ICON_Prepare(bool show);
void ICON_Control(bool show);
void ICON_Leveling(bool show);
void ICON_StartInfo(bool show);

void ICON_Setting(bool show);
void ICON_Pause(bool show);
void ICON_Continue(bool show);
void ICON_Stop(bool show);

// Popup window tips
#if HAS_HOTEND
  void Popup_Window_Temperature(const bool toohigh);
  void Popup_Window_ETempTooLow(void);
#endif

void Popup_Window_Resume(void);
void Popup_Window_Home(void);
void Popup_Window_Leveling(void);

void Goto_PrintProcess(void);
void Goto_MainMenu(void);

// Variable control
void HMI_Move_X(void);
void HMI_Move_Y(void);
void HMI_Move_Z(void);
void HMI_Move_E(void);

void HMI_Zoffset(void);

TERN_(HAS_HOTEND,     void HMI_ETemp(void));
TERN_(HAS_HEATED_BED, void HMI_BedTemp(void));
TERN_(HAS_FAN,        void HMI_FanSpeed(void));

void HMI_PrintSpeed(void);

void HMI_MaxFeedspeedXYZE(void);
void HMI_MaxAccelerationXYZE(void);
void HMI_MaxCornerXYZE(void);
void HMI_StepXYZE(void);

void update_variable(void);
void show_plus_or_minus(uint8_t size, uint16_t bColor, uint8_t iNum, uint8_t fNum, uint16_t x, uint16_t y, long value);

// SD Card
void HMI_SDCardInit(void);
void HMI_SDCardUpdate(void);

// Main Process
void Icon_print(bool value);
void Icon_control(bool value);
void Icon_temperature(bool value);
void Icon_leveling(bool value);

// Other
bool Pause_HeatStatus();
void HMI_StartFrame(const bool with_update); // startup screen
void HMI_MainMenu(void);          // main process screen
void HMI_SelectFile(void);        // file page
void HMI_Printing(void);          // print page
void HMI_Prepare(void);           // prepare page
void HMI_Control(void);           // control page
void HMI_Leveling(void);          // Level the page
void HMI_AxisMove(void);          // Axis movement menu
void HMI_Temperature(void);       // Temperature menu
void HMI_Motion(void);            // Sports menu
void HMI_Info(void);              // Information menu
void HMI_Tune(void);              // Adjust the menu

#if HAS_HOTEND
  void HMI_PLAPreheatSetting(void); // PLA warm-up setting
  void HMI_ABSPreheatSetting(void); // ABS warm-up setting
#endif

void HMI_MaxSpeed(void);          // Maximum speed submenu
void HMI_MaxAcceleration(void);   // Maximum acceleration submenu
void HMI_MaxCorner(void);         // Maximum corner speed submenu
void HMI_Step(void);              // transmission ratio

void HMI_Init(void);
void DWIN_Update(void);
void EachMomentUpdate(void);
void DWIN_HandleScreen(void);

void DWIN_CompletedHoming(void);
void DWIN_CompletedLeveling(void);
