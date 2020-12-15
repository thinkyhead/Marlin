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
 * dgus_lcd.cpp
 *
 * DGUS implementation for Marlin by coldtobi, Feb-May 2019
 */

#include "../../inc/MarlinConfigPre.h"

#if HAS_DGUS_LCD

#include "ui_api.h"
#include "lib/dgus/DGUSDisplayDef.h"
#include "lib/dgus/DGUSDisplay.h"

#if ENABLED(DWIN_CREALITY_TOUCHLCD)
  #include "lib/dgus_creality/DGUSScreenHandler.h"
#else
  #include "lib/dgus/DGUSScreenHandler.h"
#endif

extern const char NUL_STR[];

namespace ExtUI {

  void onStartup() {
    dgusdisplay.InitDisplay();
    ScreenHandler.UpdateScreenVPData();
  }

  void onIdle() { ScreenHandler.loop(); }

  void onPrinterKilled(PGM_P const error, PGM_P const component) {
    DGUSLCD_Screens screen = DGUSLCD_SCREEN_KILL;
    #if ENABLED(DWIN_CREALITY_TOUCHLCD)
      if (strcmp_P(error, GET_TEXT(MSG_ERR_MAXTEMP)) == 0 || strcmp_P(error, GET_TEXT(MSG_THERMAL_RUNAWAY)) == 0)
        screen = DGUSLCD_SCREEN_THERMAL_RUNAWAY;
      else if (strcmp_P(error, GET_TEXT(MSG_HEATING_FAILED_LCD)) == 0)
        screen = DGUSLCD_SCREEN_HEATING_FAILED;
      else if (strcmp_P(error, GET_TEXT(MSG_ERR_MINTEMP)) == 0)
        screen = DGUSLCD_SCREEN_THERMISTOR_ERROR;
      ScreenHandler.sendinfoscreen(GET_TEXT(MSG_HALTED), error, GET_TEXT(MSG_PLEASE_RESET), NUL_STR, true, true, true, true);
    #else
      ScreenHandler.sendinfoscreen(GET_TEXT(MSG_HALTED), error, NUL_STR, GET_TEXT(MSG_PLEASE_RESET), true, true, true, true);
    #endif

    ScreenHandler.GotoScreen(screen);
    while (!ScreenHandler.loop());  // Wait while anything is left to be sent
  }

  void onMediaInserted() { TERN_(SDSUPPORT, ScreenHandler.SDCardInserted()); }
  void onMediaError()    { TERN_(SDSUPPORT, ScreenHandler.SDCardError()); }
  void onMediaRemoved()  { TERN_(SDSUPPORT, ScreenHandler.SDCardRemoved()); }

  void onPlayTone(const uint16_t frequency, const uint16_t duration) {
    TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.Buzzer(frequency, duration));
  }

  void onPrintTimerStarted() {}

  void onPrintTimerPaused() {
    #if ENABLED(DWIN_CREALITY_TOUCHLCD)
      // Handle M28 Pause SD print - But only if we're not waiting on a user
      if (ExtUI::isPrintingFromMediaPaused() && ScreenHandler.getCurrentScreen() == DGUSLCD_SCREEN_PRINT_RUNNING)
        ScreenHandler.GotoScreen(DGUSLCD_SCREEN_PRINT_PAUSED);
    #endif
  }

  void onPrintTimerStopped() {}

  void onFilamentRunout(const extruder_t extruder) { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.FilamentRunout()); }

  inline void onUserConfirmed() {
    ScreenHandler.SetupConfirmAction(nullptr);
    #if ENABLED(DWIN_CREALITY_TOUCHLCD)
      //SERIAL_ECHOLNPGM("User confirmation invoked");
      ExtUI::setUserConfirmed();
    #else
      ScreenHandler.PopToOldScreen();
    #endif
  }

  void onUserConfirmRequired(const char * const msg) {
    if (msg) {
      #if ENABLED(DWIN_CREALITY_TOUCHLCD)
        //SERIAL_ECHOLNPAIR("User confirmation requested: ", msg);
        ScreenHandler.setstatusmessagePGM(msg);
        ScreenHandler.SetupConfirmAction(onUserConfirmed);
        ScreenHandler.sendinfoscreen(PSTR("Confirmation required"), msg, NUL_STR, NUL_STR, true, true, false, true);
      #else
        ScreenHandler.sendinfoscreen(PSTR("Please confirm."), nullptr, msg, nullptr, true, true, false, true);
        ScreenHandler.SetupConfirmAction(ExtUI::setUserConfirmed);
      #endif
      ScreenHandler.GotoScreen(DGUSLCD_SCREEN_POPUP);
    }
    else if (ScreenHandler.getCurrentScreen() == DGUSLCD_SCREEN_POPUP ) {
      #if ENABLED(DWIN_CREALITY_TOUCHLCD)
        //SERIAL_ECHOLNPGM("User confirmation canceled");
        //ScreenHandler.SetupConfirmAction(nullptr);
        ScreenHandler.setstatusmessagePGM(nullptr);
      #else
        ScreenHandler.SetupConfirmAction(nullptr);
      #endif
      ScreenHandler.PopToOldScreen();
    }
  }

  void onStatusChanged(const char * const msg) { ScreenHandler.setstatusmessage(msg); }

  void onFactoryReset()   { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnFactoryReset()); }

  void onHomingStart()    { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnHomingStart()); }
  void onHomingComplete() { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnHomingComplete()); }

  void onPrintFinished()  { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnPrintFinished()); }

  void onStoreSettings(char *buff) {
    // Called when saving to EEPROM (i.e. M500). If the ExtUI needs
    // permanent data to be stored, it can write up to eeprom_data_size bytes
    // into buff.

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(buff, &myDataStruct, sizeof(myDataStruct));
  }

  void onLoadSettings(const char *buff) {
    // Called while loading settings from EEPROM. If the ExtUI
    // needs to retrieve data, it should copy up to eeprom_data_size bytes
    // from buff

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(&myDataStruct, buff, sizeof(myDataStruct));
  }

  void onConfigurationStoreWritten(bool success) {
    // Called after the entire EEPROM has been written,
    // whether successful or not.
  }

  void onConfigurationStoreRead(bool success) {
    // Called after the entire EEPROM has been read,
    // whether successful or not.
  }

  #if HAS_MESH
    void onMeshLevelingStart() { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnMeshLevelingStart()); }

    void onMeshUpdate(const int8_t xpos, const int8_t ypos, const float zval) {
      // Called when any mesh points are updated
      TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.OnMeshLevelingUpdate(xpos, ypos));
    }
    void onMeshUpdate(const int8_t xpos, const int8_t ypos, const ExtUI::probe_state_t state) {
      // Called to indicate a special condition
      #if ENABLED(DWIN_CREALITY_TOUCHLCD)
        // Only called for UBL
        if (state == MESH_START) ScreenHandler.OnMeshLevelingStart();
        ScreenHandler.OnMeshLevelingUpdate(xpos, ypos);
      #endif
    }
  #endif

  #if ENABLED(POWER_LOSS_RECOVERY)
    void onPowerLossResume() {
      // Called on resume from power-loss
      ScreenHandler.GotoScreen(DGUSLCD_SCREEN_POWER_LOSS);
    }
  #endif


  #if HAS_PID_HEATING
    void onPidTuning(const result_t rst) {
      #if DISABLED(DWIN_CREALITY_TOUCHLCD)
        // Called for temperature PID tuning result
        switch (rst) {
          case PID_BAD_EXTRUDER_NUM:
            ScreenHandler.setstatusmessagePGM(GET_TEXT(MSG_PID_BAD_EXTRUDER_NUM));
            break;
          case PID_TEMP_TOO_HIGH:
            ScreenHandler.setstatusmessagePGM(GET_TEXT(MSG_PID_TEMP_TOO_HIGH));
            break;
          case PID_TUNING_TIMEOUT:
            ScreenHandler.setstatusmessagePGM(GET_TEXT(MSG_PID_TIMEOUT));
            break;
          case PID_DONE:
            ScreenHandler.setstatusmessagePGM(GET_TEXT(MSG_PID_AUTOTUNE_DONE));
            break;
        }
        ScreenHandler.GotoScreen(DGUSLCD_SCREEN_MAIN);
      #endif
    }
  #endif

  void onSteppersDisabled() { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.HandleStepperState(false)); }
  void onSteppersEnabled()  { TERN_(DWIN_CREALITY_TOUCHLCD, ScreenHandler.HandleStepperState(true)); }

}

#endif // HAS_DGUS_LCD
