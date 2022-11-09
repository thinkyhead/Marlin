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

#include "../../inc/MarlinConfig.h"

#if ENABLED(PROBE_CONTROL)

  #include "../../feature/anker/anker_homing.h"
  #include "../../feature/anker/anker_z_sensorless.h"
  #include "../../feature/anker/board_configure.h"

  #include "../gcode.h"
  #include "../queue.h"         // for getting the command port
  #include "../../module/planner.h"
  #include "../../module/motion.h"
  #include "../../module/stepper.h"
  #include "../../module/endstops.h"

  void GcodeSuite::M2000() {
    if (parser.seen('S')) {
      if (parser.value_bool()) {
        WRITE(PROBE_CONTROL_PIN, PROBE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROBE_CONTROL_PIN:OPEN!");
      }
      else {
        WRITE(PROBE_CONTROL_PIN, !PROBE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROBE_CONTROL_PIN:CLOSE!");
      }
    }
    else {
      if (READ(PROBE_CONTROL_PIN) == PROBE_CONTROL_STATE)
        SERIAL_ECHOLNPGM("PROBE_CONTROL is OPEN!");
      else
        SERIAL_ECHOLNPGM("PROBE_CONTROL is CLOSE!");
    }
  }

#endif // PROBE_CONTROL

#if ENABLED(READ_BUFFER_SIZE)

  void GcodeSuite::M2001() {
    SERIAL_ECHOLNPGM(
      "BLOCK_BUFFER_SIZE: ", BLOCK_BUFFER_SIZE
      , "\nMAX_CMD_SIZE: ", MAX_CMD_SIZE
      , "\nBUFSIZE: ", BUFSIZE
      , "\nSLOWDOWN_DIVISOR: ", SLOWDOWN_DIVISOR
      #ifdef XY_FREQUENCY_LIMIT
        , "\nXY_FREQUENCY_LIMIT: ", planner.xy_freq_limit_hz
        , "\nXY_FREQUENCY_MIN_PERCENT: ", planner.xy_freq_min_speed_factor
      #endif
    );
    SERIAL_EOL();
  }

#endif

#if ENABLED(BOARD_CONFIGURE)

  void GcodeSuite::M2002() {
    SERIAL_ECHOLNPGM("Board check");
    SERIAL_ECHOLNPGM("adc1: ", board_configure.adc1);
    SERIAL_ECHOLNPGM("adc2: ", board_configure.adc2);
  }

#endif

#if ENABLED(USE_Z_SENSORLESS)

  void GcodeSuite::M2003() {
    bool report = true;

    const uint8_t index = parser.byteval('I');
    LOOP_LINEAR_AXES(i) if (parser.seen(AXIS_CHAR(i))) {
      const int16_t value = parser.value_int();
      report = false;
      if (i == Z_AXIS) {
        if (index == 0 || index == 1) use_z_sensorless.set_z1_value(value);
        #ifdef ANKER_Z2_STALL_SENSITIVITY
          if (index == 0 || index == 2) use_z_sensorless.set_z2_value(value);
        #endif
      }
    }

    if (report) use_z_sensorless.report();
  }

#endif // USE_Z_SENSORLESS

#if ENABLED(EVT_HOMING_5X)

  void GcodeSuite::M2004() {
    #if ENABLED(NO_MOTION_BEFORE_HOMING)
      if (parser.seen('S')) {
        if (parser.value_bool()) {
          anker_homing.no_check_all_axis = true;
          SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!");
        }
        else {
          anker_homing.no_check_all_axis = false;
          SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!");
        }
      }
      else {
        if (anker_homing.no_check_all_axis)
          SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!");
        else
          SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!");
      }
    #endif
  }

#endif

#if ENABLED(USE_Z_SENSORLESS)

  void GcodeSuite::M2005() {
    switch (anker_homing.get_first_end_z_axis()) {
      case Z_AXIS_IS_Z1: SERIAL_ECHOLNPGM("first_end_z_axis:Z1!"); break;
      case Z_AXIS_IS_Z2: SERIAL_ECHOLNPGM("first_end_z_axis:Z2!"); break;
      case Z_AXIS_IDLE:  SERIAL_ECHOLNPGM("first_end_z_axis:IDLE!"); break;
    }
  }

  void GcodeSuite::M2006() { // Auto-Proof Threshold
    use_z_sensorless.set_z1_value(80); // Set an insensitive threshold so that it must return to zero
    use_z_sensorless.set_z2_value(80);
    gcode.process_subcommands_now_P(PSTR("G28Z"));
  }

#endif

#if ENABLED(TMC_AUTO_CONFIG)

  void GcodeSuite::M2007() {
    stepperX.rms_current(800, 1);
    stepperY.rms_current(800, 1);
    stepperX.set_pwm_thrs(301);
    stepperY.set_pwm_thrs(301);
    TMC2208_n::PWMCONF_t pwmconf{0};
    pwmconf.pwm_lim       = 12;
    pwmconf.pwm_reg       = 8;
    pwmconf.pwm_autograd  = true;
    pwmconf.pwm_autoscale = true;
    pwmconf.pwm_freq      = 0b01;
    pwmconf.pwm_grad      = 14;
    pwmconf.pwm_ofs       = 36;
    stepperX.PWMCONF(pwmconf.sr);
    ENABLE_AXIS_X();
    ENABLE_AXIS_Y();
    _delay_ms(200);

    SERIAL_ECHOLNPGM("PER-XPWM_SCALE_AUTO: ", stepperX.pwm_scale_auto());
    SERIAL_ECHOLNPGM("PER-YPWM_SCALE_AUTO: ", stepperY.pwm_scale_auto());
    gcode.process_subcommands_now_P(PSTR("M204 P100 R100 T100\nG1 X10 F60\nG1 X20 F7500"));
    SERIAL_ECHOLNPGM("XPWM_SCALE_AUTO: ", stepperX.pwm_scale_auto());
    SERIAL_ECHOLNPGM("YPWM_SCALE_AUTO: ", stepperY.pwm_scale_auto());
  }

#endif

#if ENABLED(ANKER_EXTRUDERS_RECEIVE)

  void GcodeSuite::M2008() {
    if (!parser.seen('S')) return;
    gcode.process_subcommands_now_P(parser.value_bool() ? PSTR("G1X17") : PSTR("G1X0"));
  }

#endif
