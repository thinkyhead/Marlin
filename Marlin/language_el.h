/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * Greek
 *
 * LCD Menu Messages
 * See also https://github.com/MarlinFirmware/Marlin/wiki/LCD-Language
 *
 */
#ifndef LANGUAGE_EL_H
#define LANGUAGE_EL_H

// Define SIMULATE_ROMFONT to see what is seen on the character based display defined in Configuration.h
//#define SIMULATE_ROMFONT

//#define MAPPER_CECF
//#define DISPLAY_CHARSET_ISO10646_GREEK

#define WELCOME_MSG                         MACHINE_NAME " έτοιμο."
#define MSG_SD_INSERTED                     "Τοποθετήθηκε η κάρτα SD"
#define MSG_SD_REMOVED                      "Η κάρτα SD αφαιρέθηκε"
#define MSG_LCD_ENDSTOPS                    "Endstops" // Max length 8 characters
#define MSG_MAIN                            "Βασική Οθόνη"
#define MSG_AUTOSTART                       "Αυτοματή Εκκίνηση"
#define MSG_DISABLE_STEPPERS                "Απενεργοποίησε τα steppers"
#define MSG_AUTO_HOME                       "Auto home"
#define MSG_AUTO_HOME_X                     "Home X"
#define MSG_AUTO_HOME_Y                     "Home Y"
#define MSG_AUTO_HOME_Z                     "Home Z"
#define MSG_LEVEL_BED_HOMING                "Homing XYZ"
#define MSG_LEVEL_BED_WAITING               "Κάνε κλικ για Εκκίνηση"
#define MSG_LEVEL_BED_NEXT_POINT            "Επόμενο Σημείο"
#define MSG_LEVEL_BED_DONE                  "Ολοκλήρωση Επιπεδοποίησης!"
#define MSG_LEVEL_BED_CANCEL                "Ακύρωση"
#define MSG_SET_HOME_OFFSETS                "Set home Μετατόπιση"
#define MSG_HOME_OFFSETS_APPLIED            "Offsets applied"
#define MSG_SET_ORIGIN                      "Set origin"
#define MSG_PREHEAT_PLA                     "Preheat PLA"
#define MSG_PREHEAT_PLA_N                   MSG_PREHEAT_PLA " "
#define MSG_PREHEAT_PLA_ALL                 MSG_PREHEAT_PLA " All"
#define MSG_PREHEAT_PLA_BEDONLY             MSG_PREHEAT_PLA " Bed"
#define MSG_PREHEAT_PLA_SETTINGS            MSG_PREHEAT_PLA " conf"
#define MSG_PREHEAT_ABS                     "Preheat ABS"
#define MSG_PREHEAT_ABS_N                   MSG_PREHEAT_ABS " "
#define MSG_PREHEAT_ABS_ALL                 MSG_PREHEAT_ABS " All"
#define MSG_PREHEAT_ABS_BEDONLY             MSG_PREHEAT_ABS " Bed"
#define MSG_PREHEAT_ABS_SETTINGS            MSG_PREHEAT_ABS " conf"
#define MSG_H1                              "1"
#define MSG_H2                              "2"
#define MSG_H3                              "3"
#define MSG_H4                              "4"
#define MSG_COOLDOWN                        "Μειωση θερμοκρασιας"
#define MSG_SWITCH_PS_ON                    "Ενεργοποιήσετε την Τροφοδοσία"
#define MSG_SWITCH_PS_OFF                   "Απενεργοποιήσετε την Τροφοδοσία"
#define MSG_EXTRUDE                         "Extrude"
#define MSG_RETRACT                         "Retract"
#define MSG_MOVE_AXIS                       "Move axis"
#define MSG_LEVEL_BED                       "Level bed"
#define MSG_MOVE_X                          "Mετακινήστε X"
#define MSG_MOVE_Y                          "Mετακινήστε Y"
#define MSG_MOVE_Z                          "Mετακινήστε Z"
#define MSG_MOVE_E                          "Extruder"
#define MSG_MOVE_E1                         "1"
#define MSG_MOVE_E2                         "2"
#define MSG_MOVE_E3                         "3"
#define MSG_MOVE_E4                         "4"
#define MSG_MOVE_01MM                       "Mετακινήστε 0.1mm"
#define MSG_MOVE_1MM                        "Mετακινήστε 1mm"
#define MSG_MOVE_10MM                       "Mετακινήστε 10mm"
#define MSG_SPEED                           "Ταχύτητα"
#define MSG_BED_Z                           "Bed Z"
#define MSG_NOZZLE                          "Nozzle"
#define MSG_N1                              " 1"
#define MSG_N2                              " 2"
#define MSG_N3                              " 3"
#define MSG_N4                              " 4"
#define MSG_BED                             "Bed"
#define MSG_FAN_SPEED                       "Fan speed"
#define MSG_FLOW                            "Flow"
#define MSG_CONTROL                         "Έλεγχος"
#define MSG_MIN                             " " LCD_STR_THERMOMETER " Min"
#define MSG_MAX                             " " LCD_STR_THERMOMETER " Max"
#define MSG_FACTOR                          " " LCD_STR_THERMOMETER " Fact"
#define MSG_AUTOTEMP                        "Autotemp"
#define MSG_ON                              "Ενεργοποιημένο"
#define MSG_OFF                             "Απενεργοποιημένο"
#define MSG_PID_P                           "PID-P"
#define MSG_PID_I                           "PID-I"
#define MSG_PID_D                           "PID-D"
#define MSG_PID_C                           "PID-C"
#define MSG_E1                              " E1"
#define MSG_E2                              " E2"
#define MSG_E3                              " E3"
#define MSG_E4                              " E4"
#define MSG_ACC                             "Accel"
#define MSG_VXY_JERK                        "Vxy-jerk"
#define MSG_VZ_JERK                         "Vz-jerk"
#define MSG_VE_JERK                         "Ve-jerk"
#define MSG_VMAX                            "Vmax "
#define MSG_X                               "X"
#define MSG_Y                               "Y"
#define MSG_Z                               "Z"
#define MSG_E                               "E"
#define MSG_VMIN                            "Vmin"
#define MSG_VTRAV_MIN                       "VTrav min"
#define MSG_AMAX                            "Amax "
#define MSG_A_RETRACT                       "A-retract"
#define MSG_A_TRAVEL                        "A-travel"
#define MSG_XSTEPS                          "Xsteps/mm"
#define MSG_YSTEPS                          "Ysteps/mm"
#define MSG_ZSTEPS                          "Zsteps/mm"
#define MSG_ESTEPS                          "Esteps/mm"
#define MSG_TEMPERATURE                     "Θερμοκρασία"
#define MSG_MOTION                          "Motion"
#define MSG_VOLUMETRIC                      "Filament"
#define MSG_VOLUMETRIC_ENABLED              "E in mm3"
#define MSG_FILAMENT_DIAM                   "Fil. Dia."
#define MSG_DIAM_E1                         " 1"
#define MSG_DIAM_E2                         " 2"
#define MSG_DIAM_E3                         " 3"
#define MSG_DIAM_E4                         " 4"
#define MSG_CONTRAST                        "LCD contrast"
#define MSG_STORE_EPROM                     "Store memory"
#define MSG_LOAD_EPROM                      "Load memory"
#define MSG_RESTORE_FAILSAFE                "Restore failsafe"
#define MSG_REFRESH                         "Refresh"
#define MSG_WATCH                           "Info screen"
#define MSG_PREPARE                         "Prepare"
#define MSG_TUNE                            "Tune"
#define MSG_PAUSE_PRINT                     "Pause print"
#define MSG_RESUME_PRINT                    "Resume print"
#define MSG_STOP_PRINT                      "Stop print"
#define MSG_CARD_MENU                       "Print from SD"
#define MSG_NO_CARD                         "No SD card"
#define MSG_DWELL                           "Sleep..."
#define MSG_USERWAIT                        "Wait for user..."
#define MSG_RESUMING                        "Resuming print"
#define MSG_PRINT_ABORTED                   "Print aborted"
#define MSG_NO_MOVE                         "No move."
#define MSG_KILLED                          "KILLED. "
#define MSG_STOPPED                         "STOPPED. "
#define MSG_CONTROL_RETRACT                 "Retract mm"
#define MSG_CONTROL_RETRACT_SWAP            "Swap Re.mm"
#define MSG_CONTROL_RETRACTF                "Retract  V"
#define MSG_CONTROL_RETRACT_ZLIFT           "Hop mm"
#define MSG_CONTROL_RETRACT_RECOVER         "UnRet +mm"
#define MSG_CONTROL_RETRACT_RECOVER_SWAP    "S UnRet+mm"
#define MSG_CONTROL_RETRACT_RECOVERF        "UnRet  V"
#define MSG_AUTORETRACT                     "AutoRetr."
#define MSG_FILAMENTCHANGE                  "Change filament"
#define MSG_INIT_SDCARD                     "Init. SD card"
#define MSG_CNG_SDCARD                      "Change SD card"
#define MSG_ZPROBE_OUT                      "Z probe out. bed"
#define MSG_YX_UNHOMED                      "Home X/Y before Z"
#define MSG_XYZ_UNHOMED                     "Home XYZ first"
#define MSG_ZPROBE_ZOFFSET                  "Z Offset"
#define MSG_BABYSTEP_X                      "Babystep X"
#define MSG_BABYSTEP_Y                      "Babystep Y"
#define MSG_BABYSTEP_Z                      "Babystep Z"
#define MSG_ENDSTOP_ABORT                   "Endstop abort"
#define MSG_HEATING_FAILED_LCD              "Heating failed"
#define MSG_ERR_REDUNDANT_TEMP              "Err: REDUNDANT TEMP"
#define MSG_THERMAL_RUNAWAY                 "THERMAL RUNAWAY"
#define MSG_ERR_MAXTEMP                     "Err: MAXTEMP"
#define MSG_ERR_MINTEMP                     "Err: MINTEMP"
#define MSG_ERR_MAXTEMP_BED                 "Err: MAXTEMP BED"
#define MSG_ERR_MINTEMP_BED                 "Err: MINTEMP BED"
#define MSG_END_HOUR                        "hours"
#define MSG_END_MINUTE                      "minutes"
#define MSG_HEATING                         "Heating..."
#define MSG_HEATING_COMPLETE                "Heating done."
#define MSG_BED_HEATING                     "Bed Heating."
#define MSG_BED_DONE                        "Bed done."
#define MSG_DELTA_CALIBRATE                 "Delta Calibration"
#define MSG_DELTA_CALIBRATE_X               "Calibrate X"
#define MSG_DELTA_CALIBRATE_Y               "Calibrate Y"
#define MSG_DELTA_CALIBRATE_Z               "Calibrate Z"
#define MSG_DELTA_CALIBRATE_CENTER          "Calibrate Center"

#endif // LANGUAGE_EL_H
