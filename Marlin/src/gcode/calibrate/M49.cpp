/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2025 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#if ENABLED(STALLGUARD_TUNING)

#include "../gcode.h"
#include "../../module/motion.h"
#include "../../module/endstops.h"
#include "../../lcd/marlinui.h"
#include "../../feature/bedlevel/bedlevel.h" // to disable it
#include "../../module/stepper/indirection.h" // for restore_stepper_drivers

/**
 * M49: Sensorless and StallGuard measurement function.
 *      Use a bisect method to find the feedrate, current, speed, and acceleration that works for each axis.
 *      The aim is to find the balance of most reliable and highest homing speed.
 *
 * Parameters:
 *     H<count>     Do 'count' homings each of X of Y, then one homing of Z. If just 'H' is given, do three attempts.
 *
 *     X<linear>    Initial X move (else +20mm)
 *     Y<linear>    Initial Y move (else +20mm)
 *     Z<linear>    Initial Z raise (else Z_CLEARANCE_FOR_HOMING)
 *
 *     P<linear>    Length of the X axis test (else 20mm)
 *     Q<linear>    Length of the Y axis test (else 20mm)
 *
 *     F<mm/s>      Proposed (Starting) Feedrate (else 20mm/s)
 *     G<mm/s>      Span of Feedrates to test (else 20mm/s)
 *
 *     L<current>   Current range lower limit
 *     O<current>   Current range upper limit
 *
 *     S<steps>     Number of feedrate tests
 *     C<steps>     Number of current tests
 *
 *     V<int>       Verbose level (may discard)
 *
 * How This Works:
 *  - The initial homing does the individual X and Y axes with a few tries using the existing settings.
 *  - We can do an initial move to clear space, allowing for some grinding.
 *  - Do a series of moves starting at the given feedrate and looking for stall signals.
 *  - If we do a short move where we know there is no endstop and get a "hit" we know to reduce sensitivity. We do need to find that initial point.
 *    - So, moving only in clear space, map out the acceleration, speed, and current where false positives appear and disappear.
 *    - The feedrate/acceleration where false positives appear give the max rate that is "safe" to use for homing. But, still go lower.
 *    - The feedrate where false positives never occur, but where real hits are always detected, is the ideal rate to use for homing.
 *  - Once we have tested for false positives then we have to bump the real end to find the values that fail to trigger, false negatives, and aim higher.
 *  - The ideal rate/accel/current is around halfway between the false negative and the false positive.
 *  - The ideal combination of rate/accel/current is where this range is widest.
 *
 *  - For initial development just write code to do a series of moves and log the results to see what works and what doesn't.
 *  - For example, do endstop moves in free space in a loop varying the trio, just logging whether or not they give false positive.
 *  - After each step, take the stalled position and trust it to move to the corrected position.
 *
 * Sensorless Notes:
 *  - Instead of responding to triggered endstops in the stepper ISR based on the global flag, set an "interruptible" flag in the block. This would prevent
 *    previous moves made without endstops enabled from being interrupted by endstops being enabled globally for a future planned move.
 *  - Lower acceleration is safer, but we could also ignore the endstop for a short period after the move starts, the most likely moment of a false positive.
 *  - The purpose of reduced current is to reduce motor feedback and reduce wear on printer parts, so the lowest current that works is good.
 *    But lower current then requires lower acceleration and/or top speed.
 *  - Once we find a range to move freely in the initial homing shimmy, with (paused) direction change, without false positive, then we want
 *    to set the threshold slightly less sensitive.
 */

#define TEST_SPAN 40
#define TEST_SPAN_X (-TEST_SPAN * X_HOME_DIR)
#define TEST_SPAN_Y (-TEST_SPAN * Y_HOME_DIR)

void GcodeSuite::M49() {

  const int8_t verbose_level = parser.byteval('V', 1);
  if (!WITHIN(verbose_level, 0, 2)) {
    SERIAL_ECHOLNPGM(GCODE_ERR_MSG("(V)erbose level implausible (0-2)."));
    return;
  }

  if (verbose_level > 0)
    SERIAL_ECHOLNPGM("M49 Stall Threshold Test");

  // Always disable Bed Level correction before this test...
  TERN_(HAS_LEVELING, TEMPORARY_BED_LEVELING_STATE(false));

  // Work with unscaled feedrates
  remember_feedrate_scaling_off();

  // Clear the endstops by 40mm in both X and Y
  // We have to be able to do short homing moves so testing moves in the range of 20mm is fine
  const xy_pos_t workspot = { X_HOME_POS + TEST_SPAN_X / 2, Y_HOME_POS + TEST_SPAN_Y / 2 };
  const xy_pos_t bumpspotx = { X_HOME_POS + X_HOME_DIR * 2, Y_HOME_POS + TEST_SPAN_Y / 2 };
  const xy_pos_t bumpspoty = { X_HOME_POS + TEST_SPAN_X / 2, Y_HOME_POS + Y_HOME_DIR * 2 };

  // If 'H' was given then home X and Y carefully. We could use a lowered rate and acceleration for extra safety.
  if (parser.seen_test('H')) {
    const uint8_t tries = parser.byteval('H', 3);
    for (uint8_t i = tries; i--;) homeaxis(X_AXIS);
    for (uint8_t i = tries; i--;) homeaxis(Y_AXIS);
    homeaxis(Z_AXIS);
    do_blocking_move_to_xy_z(workspot, Z_CLEARANCE_FOR_HOMING);
  }

  set_homing_current(X_AXIS); // Set homing current for X axis
  const bool x_stealth = tmc_enable_stallguard(stepperX);

  const auto oldthrs = stepperX.homing_threshold();

  wait_for_user = true;

  bool test_complete = false;
  feedRate_t test_feedrate_mm_s = 20; // 0.9° (6400SPR) Stepper can home half as fast as 1.8° (3200SPR) stepper. For 160 steps/mm, 20mm/s = 3200 steps/s, or 1/2 RPS.
  do {
    // For each move attempt to move 20mm with endstops on
    endstops.enable(true);

    //endstops.hit_on_purpose(); // Clear hit state

    restore_stepper_drivers();    // Reapply TMCStepper shadow states

    // Turn on TMC Debug Reporting
    gcode.process_subcommands_now(F("M122 X S1 P250")); // Full Report at a fast 4Hz

    // Test a range of thresholds at the test feedrate.
    // Later we'll do a range of feedrates around the starting value.

    int16_t test_thrs = stepperX.sgt_min;
    while (test_thrs <= stepperX.sgt_max) {
      if (!wait_for_user) { test_complete = true; break; }

      stepperX.homing_threshold(test_thrs);

      // Do some X moves and observe the output
      for (uint8_t attempt = 1; attempt--;) {
        if (!wait_for_user) { test_complete = true; break; }
        // Move right 20mm
        safe_delay(350); do_blocking_move_to_x(workspot.x + TEST_SPAN_X * 7/16, test_feedrate_mm_s);
        safe_delay(350); do_blocking_move_to_x(workspot.x + TEST_SPAN_X * 1/16, test_feedrate_mm_s);
        safe_delay(350); do_blocking_move_to_x(workspot.x + TEST_SPAN_X * 0/16, test_feedrate_mm_s);
        safe_delay(350); do_blocking_move_to_x(workspot.x - TEST_SPAN_X * 1/16, test_feedrate_mm_s);
        safe_delay(350); do_blocking_move_to_x(workspot.x - TEST_SPAN_X * 7/16, test_feedrate_mm_s);
      }
      test_thrs += 1;
    }

    test_complete = true; // For now just do one set of moves

    // Turn off TMC Debug Reporting
    gcode.process_subcommands_now(F("M122 S0"));
    endstops.enable(false);   // Disable for now
  } while (!test_complete);

  // Use non-homing SG thrs
  stepperX.homing_threshold(oldthrs);

  tmc_disable_stallguard(stepperX, x_stealth);
  restore_homing_current(X_AXIS); // Set homing current for X axis

  endstops.not_homing();      // Restore endstop global state
  restore_feedrate_and_scaling();
  report_current_position();
}

#endif // STALLGUARD_TUNING
