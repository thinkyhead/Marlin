/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime :
 * @Description  :
 */
#include "anker_homing.h"

#if ENABLED(EVT_HOMING_5X)

#include "../bedlevel/bedlevel.h"

#if ADAPT_DETACHED_NOZZLE
  #include "../interactive/uart_nozzle_tx.h"
  #include "../interactive/uart_nozzle_rx.h"
#endif

Anker_Homing anker_homing;
#if ENABLED(NO_MOTION_BEFORE_HOMING)
  void Anker_Homing::anker_disable_motton_before_init() {
    SET_INPUT_PULLUP(DISABLED_MOTION_BEFORE_PIN);
  }
  void Anker_Homing::anker_disable_motton_before_check() {
    anker_homing.no_check_all_axis = (READ(DISABLED_MOTION_BEFORE_PIN) == DISABLED_MOTION_BEFORE_STATE);
  }
#endif

#if ENABLED(USE_Z_SENSORLESS)

  #include "../../module/motion.h"
  #include "../../module/probe.h"
  #include "anker_align.h"
  #include "anker_nozzle_board.h"

  void Anker_Homing::set_first_end_z_axis(ANKER_Z_AXIS z) {
    first_end_z_axis = z;
  }

  ANKER_Z_AXIS Anker_Homing::get_first_end_z_axis() { return first_end_z_axis; }

  #define FIRST_END_Z1_POS_X              235
  #define FIRST_END_Z1_POS_Y              237
  #define FIRST_END_Z2_POS_X              0
  #define FIRST_END_Z2_POS_Y              237
  #define DUAL_Z_NOZZLE_BELOW_BED_MM      4     //The height of the nozzle below the heated bed when the double Z is locked and reset to zero
  #define DUAL_Z_NOZZLE_BELOW_BED_STEP_MM 0.1
  #define HOMING_RUN_ALIGN_MM             20
  #define HOMING_RUN_ALIGN_STEP_MM        0.1

  void Anker_Homing::anker_dual_z_run_align() {
    bool run_align = true;
    xyz_pos_t first_z1_pos = { FIRST_END_Z1_POS_X, FIRST_END_Z1_POS_Y, current_position.z };
    xyz_pos_t first_z2_pos = { FIRST_END_Z2_POS_X, FIRST_END_Z2_POS_Y, current_position.z };
    if (first_end_z_axis == Z_AXIS_IS_Z1)
      do_blocking_move_to(first_z1_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S));
    else if (first_end_z_axis == Z_AXIS_IS_Z2)
      do_blocking_move_to(first_z2_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S));

    if (nozzle_board_type == NOZZLE_TYPE_OLD)
      set_anker_z_sensorless_probe_value(800);
    else
      uart_nozzle_tx_probe_val(800);

    #if ENABLED(PROBE_CONTROL)
      if (IS_old_nozzle_board())
        WRITE(PROBE_CONTROL_PIN, !PROBE_CONTROL_STATE);
      probe.anker_level_set_probing_paused(true,ANKER_LEVELING_DELAY_BEFORE_PROBING);
      if (IS_old_nozzle_board())
        WRITE(PROBE_CONTROL_PIN, PROBE_CONTROL_STATE);
    #endif
    for (uint16_t i = 0; i < (DUAL_Z_NOZZLE_BELOW_BED_MM / DUAL_Z_NOZZLE_BELOW_BED_STEP_MM); i++) {
      do_blocking_move_to_z(current_position.z-DUAL_Z_NOZZLE_BELOW_BED_STEP_MM);
      if (PROBE_TRIGGERED()) {
        SERIAL_ECHOLNPGM(" probe first triggered!");
        run_align = false;
        break;
      }
    }

    if (run_align) {
      run_align = false;
      if (first_end_z_axis == Z_AXIS_IS_Z1) {
        for (uint16_t i = 0; i < (HOMING_RUN_ALIGN_MM / HOMING_RUN_ALIGN_STEP_MM); i++) {
          anker_align.add_z2_value(-HOMING_RUN_ALIGN_STEP_MM);
          if (PROBE_TRIGGERED()) {
            SERIAL_ECHO("right probe triggered!\r\n");
            break;
          }
        }
      }
      else if (first_end_z_axis == Z_AXIS_IS_Z2) {
        for (uint16_t i = 0; i < (HOMING_RUN_ALIGN_MM / HOMING_RUN_ALIGN_STEP_MM); i++) {
          anker_align.add_z1_value(-HOMING_RUN_ALIGN_STEP_MM);
          if (PROBE_TRIGGERED()) {
            SERIAL_ECHO("lift probe triggered!\r\n");
            break;
          }
        }
      }
    }

    #if ENABLED(PROBE_CONTROL)
      if (IS_old_nozzle_board())
        WRITE(PROBE_CONTROL_PIN, !PROBE_CONTROL_STATE);
    #endif
    reset_anker_z_sensorless_probe_value();
  }

  void Anker_Homing::set_triger_per_ms() {
    anker_homing.trigger_per_ms = millis();
  }

  void Anker_Homing::set_triger_ms() {
    anker_homing.trigger_ms = millis();
    SERIAL_ECHOLNPGM(" trigger!!");
  }

  bool Anker_Homing::is_z_top_triger() {
    SERIAL_ECHOLNPAIR(" trigger_per_ms:", anker_homing.trigger_per_ms, "\n trigger_ms:", anker_homing.trigger_ms);
    if (anker_homing.trigger_ms<(anker_homing.trigger_per_ms+ANTHER_TIME_Z_MIN_LIMIT))
    return true;
    else return false;
  }

  bool Anker_Homing::is_anthor_z_no_triger() {
    SERIAL_ECHOLNPAIR(" anthor_z trigger_per_ms:", anker_homing.trigger_per_ms, "\n anthor_z trigger_ms:", anker_homing.trigger_ms);
    if (anker_homing.trigger_ms>(anker_homing.trigger_per_ms+ANTHER_TIME_ANTHOR_Z_MAX_LIMIT))
    return true;
    else return false;
  }

  void Anker_Homing::set_probe_triger_ms() {
    anker_homing.trigger_ms = millis();
    SERIAL_ECHOLNPGM(" probe trigger!!");
  }

  bool Anker_Homing::is_z_probe_no_triger() { // If the probe does not trigger for a long time, reset Z to zero
    return ELAPSED(millis(), (anker_homing.trigger_per_ms + ANKER_PROBE_TIMEOUT));
  }

#endif // USE_Z_SENSORLESS

// Determine whether the center point needs to be zeroed
bool Anker_Homing::is_center_home() {
  return anker_homing.anker_z_homing_options && TERN1(USE_Z_SENSORLESS, !anker_homing.is_again_probe_homing);
}

#endif // EVT_HOMING_5X
