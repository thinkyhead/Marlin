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
 * configuration_store.cpp
 *
 * Configuration and EEPROM storage
 *
 * IMPORTANT:  Whenever there are changes made to the variables stored in EEPROM
 * in the functions below, also increment the version number. This makes sure that
 * the default values are used whenever there is a change to the data, to prevent
 * wrong data being written to the variables.
 *
 * ALSO: Variables in the Store and Retrieve sections must be in the same order.
 *       If a feature is disabled, some data must still be written that, when read,
 *       either sets a Sane Default, or results in No Change to the existing value.
 *
 */

#define EEPROM_VERSION "V80"

// Change EEPROM version if these are changed:
#define EEPROM_OFFSET 100

/**
 * V80 EEPROM Layout:
 *
 *  100  Version (char x4)
 *  104  EEPROM Checksum (uint16_t)
 *
 *  106  M92 XYZE  planner.axis_steps_per_mm (float x4)
 *  122  M203 XYZE planner.max_feedrate_mm_s (float x4)
 *  138  M201 XYZE planner.max_acceleration_mm_per_s2 (uint32_t x4)
 *  154  M204 P    planner.acceleration (float)
 *  158  M204 T    planner.travel_acceleration (float)
 *  162  M205 S    planner.min_feedrate_mm_s (float)
 *  166  M205 T    planner.min_travel_feedrate_mm_s (float)
 *  170  M205 B    planner.min_segment_time (ulong)
 *  174  M205 Z    planner.max_z_jerk (float)
 *  178  M206 ABCD home_offset (float x4)
 *
 *  194  This Slot is Available!
 *
 */
#include "Marlin.h"
#include "language.h"
#include "endstops.h"
#include "planner.h"
#include "configuration_store.h"

uint16_t eeprom_checksum;
const char version[4] = EEPROM_VERSION;

void _EEPROM_writeData(int &pos, uint8_t* value, uint8_t size) {
  uint8_t c;
  while (size--) {
    eeprom_write_byte((unsigned char*)pos, *value);
    c = eeprom_read_byte((unsigned char*)pos);
    if (c != *value) {
      SERIAL_ECHO_START;
      SERIAL_ECHOLNPGM(MSG_ERR_EEPROM_WRITE);
    }
    eeprom_checksum += c;
    pos++;
    value++;
  };
}
void _EEPROM_readData(int &pos, uint8_t* value, uint8_t size) {
  do {
    uint8_t c = eeprom_read_byte((unsigned char*)pos);
    *value = c;
    eeprom_checksum += c;
    pos++;
    value++;
  } while (--size);
}

/**
 * Post-process after Retrieve or Reset
 */
void Config_Postprocess() {
  // steps per s2 needs to be updated to agree with units per s2
  planner.reset_acceleration_rates();

  // Refresh steps_to_mm with the reciprocal of axis_steps_per_mm
  // and init stepper.count[], planner.position[] with current_position
  planner.refresh_positioning();
}

#if ENABLED(EEPROM_SETTINGS)

  #define DUMMY_PID_VALUE 3000.0f
  #define EEPROM_START() int eeprom_index = EEPROM_OFFSET
  #define EEPROM_SKIP(VAR) eeprom_index += sizeof(VAR)
  #define EEPROM_WRITE(VAR) _EEPROM_writeData(eeprom_index, (uint8_t*)&VAR, sizeof(VAR))
  #define EEPROM_READ(VAR) _EEPROM_readData(eeprom_index, (uint8_t*)&VAR, sizeof(VAR))

/**
 * M500 - Store Configuration
 */
void Config_StoreSettings()  {
  float dummy = 0.0f;
  char ver[4] = "000";

  EEPROM_START();

  EEPROM_WRITE(ver);     // invalidate data first
  EEPROM_SKIP(eeprom_checksum); // Skip the checksum slot

  eeprom_checksum = 0; // clear before first "real data"

  EEPROM_WRITE(planner.axis_steps_per_mm);
  EEPROM_WRITE(planner.max_feedrate_mm_s);
  EEPROM_WRITE(planner.max_acceleration_mm_per_s2);
  EEPROM_WRITE(planner.acceleration);
  EEPROM_WRITE(planner.travel_acceleration);
  EEPROM_WRITE(planner.min_feedrate_mm_s);
  EEPROM_WRITE(planner.min_travel_feedrate_mm_s);
  EEPROM_WRITE(planner.min_segment_time);
  EEPROM_WRITE(planner.max_z_jerk);
  EEPROM_WRITE(home_offset);

  uint16_t final_checksum = eeprom_checksum,
           eeprom_size = eeprom_index;

  eeprom_index = EEPROM_OFFSET;
  EEPROM_WRITE(version);
  EEPROM_WRITE(final_checksum);

  // Report storage size
  SERIAL_ECHO_START;
  SERIAL_ECHOPAIR("Settings Stored (", eeprom_size);
  SERIAL_ECHOLNPGM(" bytes)");
}

/**
 * M501 - Retrieve Configuration
 */
void Config_RetrieveSettings() {

  EEPROM_START();

  char stored_ver[4];
  EEPROM_READ(stored_ver);

  uint16_t stored_checksum;
  EEPROM_READ(stored_checksum);

  //  SERIAL_ECHOPAIR("Version: [", ver);
  //  SERIAL_ECHOPAIR("] Stored version: [", stored_ver);
  //  SERIAL_ECHOLNPGM("]");

  if (strncmp(version, stored_ver, 3) != 0) {
    Config_ResetDefault();
  }
  else {
    float dummy = 0;

    eeprom_checksum = 0; // clear before reading first "real data"

    // version number match
    EEPROM_READ(planner.axis_steps_per_mm);
    EEPROM_READ(planner.max_feedrate_mm_s);
    EEPROM_READ(planner.max_acceleration_mm_per_s2);

    EEPROM_READ(planner.acceleration);
    EEPROM_READ(planner.travel_acceleration);
    EEPROM_READ(planner.min_feedrate_mm_s);
    EEPROM_READ(planner.min_travel_feedrate_mm_s);
    EEPROM_READ(planner.min_segment_time);
    EEPROM_READ(planner.max_z_jerk);
    EEPROM_READ(home_offset);

    if (eeprom_checksum == stored_checksum) {
      Config_Postprocess();
      SERIAL_ECHO_START;
      SERIAL_ECHO(version);
      SERIAL_ECHOPAIR(" stored settings retrieved (", eeprom_index);
      SERIAL_ECHOLNPGM(" bytes)");
    }
    else {
      SERIAL_ERROR_START;
      SERIAL_ERRORLNPGM("EEPROM checksum mismatch");
      Config_ResetDefault();
    }
 }

  #if ENABLED(EEPROM_CHITCHAT)
    Config_PrintSettings();
  #endif
}

#endif // EEPROM_SETTINGS

/**
 * M502 - Reset Configuration
 */
void Config_ResetDefault() {
  float tmp1[] = DEFAULT_AXIS_STEPS_PER_UNIT;
  float tmp2[] = DEFAULT_MAX_FEEDRATE;
  long tmp3[] = DEFAULT_MAX_ACCELERATION;
  LOOP_ABCD(i) {
    planner.axis_steps_per_mm[i] = tmp1[i];
    planner.max_feedrate_mm_s[i] = tmp2[i];
    planner.max_acceleration_mm_per_s2[i] = tmp3[i];
  }

  planner.acceleration = DEFAULT_ACCELERATION;
  planner.travel_acceleration = DEFAULT_TRAVEL_ACCELERATION;
  planner.min_feedrate_mm_s = DEFAULT_MINIMUMFEEDRATE;
  planner.min_segment_time = DEFAULT_MINSEGMENTTIME;
  planner.min_travel_feedrate_mm_s = DEFAULT_MINTRAVELFEEDRATE;
  planner.max_z_jerk = DEFAULT_ZJERK;
  home_offset[A_AXIS] = home_offset[B_AXIS] = home_offset[C_AXIS] = 0;

  endstops.enable_globally(false);

  Config_Postprocess();

  SERIAL_ECHO_START;
  SERIAL_ECHOLNPGM("Hardcoded Default Settings Loaded");
}

#if DISABLED(DISABLE_M503)

#define CONFIG_ECHO_START do{ if (!forReplay) SERIAL_ECHO_START; }while(0)

/**
 * M503 - Print Configuration
 */
void Config_PrintSettings(bool forReplay) {
  // Always have this function, even with EEPROM_SETTINGS disabled, the current values will be shown

  CONFIG_ECHO_START;

  if (!forReplay) {
    SERIAL_ECHOLNPGM("Steps per unit:");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M92 X", planner.axis_steps_per_mm[A_AXIS]);
  SERIAL_ECHOPAIR(" Y", planner.axis_steps_per_mm[B_AXIS]);
  SERIAL_ECHOPAIR(" Z", planner.axis_steps_per_mm[C_AXIS]);
  SERIAL_ECHOPAIR(" E", planner.axis_steps_per_mm[D_AXIS]);
  SERIAL_EOL;

  CONFIG_ECHO_START;

  if (!forReplay) {
    SERIAL_ECHOLNPGM("Maximum feedrates (mm/s):");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M203 X", planner.max_feedrate_mm_s[A_AXIS]);
  SERIAL_ECHOPAIR(" Y", planner.max_feedrate_mm_s[B_AXIS]);
  SERIAL_ECHOPAIR(" Z", planner.max_feedrate_mm_s[C_AXIS]);
  SERIAL_ECHOPAIR(" E", planner.max_feedrate_mm_s[D_AXIS]);
  SERIAL_EOL;

  CONFIG_ECHO_START;
  if (!forReplay) {
    SERIAL_ECHOLNPGM("Maximum Acceleration (mm/s2):");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M201 X", planner.max_acceleration_mm_per_s2[A_AXIS]);
  SERIAL_ECHOPAIR(" Y", planner.max_acceleration_mm_per_s2[B_AXIS]);
  SERIAL_ECHOPAIR(" Z", planner.max_acceleration_mm_per_s2[C_AXIS]);
  SERIAL_ECHOPAIR(" E", planner.max_acceleration_mm_per_s2[D_AXIS]);
  SERIAL_EOL;
  CONFIG_ECHO_START;
  if (!forReplay) {
    SERIAL_ECHOLNPGM("Accelerations: P=printing, R=retract and T=travel");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M204 P", planner.acceleration);
  SERIAL_ECHOPAIR(" T", planner.travel_acceleration);
  SERIAL_EOL;

  CONFIG_ECHO_START;
  if (!forReplay) {
    SERIAL_ECHOLNPGM("Advanced variables: S=Min feedrate (mm/s), T=Min travel feedrate (mm/s), B=minimum segment time (ms), X=maximum XY jerk (mm/s),  Z=maximum Z jerk (mm/s),  E=maximum E jerk (mm/s)");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M205 S", planner.min_feedrate_mm_s);
  SERIAL_ECHOPAIR(" T", planner.min_travel_feedrate_mm_s);
  SERIAL_ECHOPAIR(" B", planner.min_segment_time);
  SERIAL_ECHOPAIR(" Z", planner.max_z_jerk);
  SERIAL_EOL;

  CONFIG_ECHO_START;
  if (!forReplay) {
    SERIAL_ECHOLNPGM("Home offset (mm)");
    CONFIG_ECHO_START;
  }
  SERIAL_ECHOPAIR("  M206 X", home_offset[A_AXIS]);
  SERIAL_ECHOPAIR(" Y", home_offset[B_AXIS]);
  SERIAL_ECHOPAIR(" Z", home_offset[C_AXIS]);
  SERIAL_EOL;
}

#endif // !DISABLE_M503
