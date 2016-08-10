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
 * planner.cpp
 *
 * Buffer movement commands and manage the acceleration profile plan
 *
 * Derived from Grbl
 * Copyright (c) 2009-2011 Simen Svale Skogsrud
 *
 * The ring buffer implementation gleaned from the wiring_serial library by David A. Mellis.
 *
 *
 * Reasoning behind the mathematics in this module (in the key of 'Mathematica'):
 *
 * s == speed, a == acceleration, t == time, d == distance
 *
 * Basic definitions:
 *   Speed[s_, a_, t_] := s + (a*t)
 *   Travel[s_, a_, t_] := Integrate[Speed[s, a, t], t]
 *
 * Distance to reach a specific speed with a constant acceleration:
 *   Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, d, t]
 *   d -> (m^2 - s^2)/(2 a) --> estimate_acceleration_distance()
 *
 * Speed after a given distance of travel with constant acceleration:
 *   Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, m, t]
 *   m -> Sqrt[2 a d + s^2]
 *
 * DestinationSpeed[s_, a_, d_] := Sqrt[2 a d + s^2]
 *
 * When to start braking (di) to reach a specified destination speed (s2) after accelerating
 * from initial speed s1 without ever stopping at a plateau:
 *   Solve[{DestinationSpeed[s1, a, di] == DestinationSpeed[s2, a, d - di]}, di]
 *   di -> (2 a d - s1^2 + s2^2)/(4 a) --> intersection_distance()
 *
 * IntersectionDistance[s1_, s2_, a_, d_] := (2 a d - s1^2 + s2^2)/(4 a)
 *
 */

#include "planner.h"
#include "stepper.h"
#include "language.h"

#include "Marlin.h"

Planner planner;

  // public:

/**
 * A ring buffer of moves described in steps
 */
block_t Planner::block_buffer[BLOCK_BUFFER_SIZE];
volatile uint8_t Planner::block_buffer_head = 0;           // Index of the next block to be pushed
volatile uint8_t Planner::block_buffer_tail = 0;

float Planner::max_feedrate_mm_s[NUM_AXIS], // Max speeds in mm per second
      Planner::axis_steps_per_mm[NUM_AXIS],
      Planner::steps_to_mm[NUM_AXIS];

unsigned long Planner::max_acceleration_steps_per_s2[NUM_AXIS],
              Planner::max_acceleration_mm_per_s2[NUM_AXIS]; // Use M201 to override by software

millis_t Planner::min_segment_time;
float Planner::min_feedrate_mm_s,
      Planner::acceleration,         // Normal acceleration mm/s^2  DEFAULT ACCELERATION for all printing moves. M204 SXXXX
      Planner::travel_acceleration,  // Travel acceleration mm/s^2  DEFAULT ACCELERATION for all NON printing moves. M204 MXXXX
      Planner::max_z_jerk,           // The largest speed change requiring no acceleration
      Planner::min_travel_feedrate_mm_s;

// private:

long Planner::position[NUM_AXIS] = { 0 };

float Planner::previous_speed[NUM_AXIS],
      Planner::previous_nominal_speed;

#ifdef XY_FREQUENCY_LIMIT
  // Old direction bits. Used for speed calculations
  unsigned char Planner::old_direction_bits = 0;
  // Segment times (in µs). Used for speed calculations
  long Planner::axis_segment_time[2][3] = { {MAX_FREQ_TIME + 1, 0, 0}, {MAX_FREQ_TIME + 1, 0, 0} };
#endif

/**
 * Class and Instance Methods
 */

Planner::Planner() { init(); }

void Planner::init() {
  block_buffer_head = block_buffer_tail = 0;
  memset(position, 0, sizeof(position)); // clear position
  LOOP_ABCD(i) previous_speed[i] = 0.0;
  previous_nominal_speed = 0.0;
}

/**
 * Calculate trapezoid parameters, multiplying the entry- and exit-speeds
 * by the provided factors.
 */
void Planner::calculate_trapezoid_for_block(block_t* block, float entry_factor, float exit_factor) {
  unsigned long initial_rate = ceil(block->nominal_rate * entry_factor),
                final_rate = ceil(block->nominal_rate * exit_factor); // (steps per second)

  // Limit minimal step rate (Otherwise the timer will overflow.)
  NOLESS(initial_rate, 120);
  NOLESS(final_rate, 120);

  long accel = block->acceleration_steps_per_s2;
  int32_t accelerate_steps = ceil(estimate_acceleration_distance(initial_rate, block->nominal_rate, accel));
  int32_t decelerate_steps = floor(estimate_acceleration_distance(block->nominal_rate, final_rate, -accel));

  // Calculate the size of Plateau of Nominal Rate.
  int32_t plateau_steps = block->step_event_count - accelerate_steps - decelerate_steps;

  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort accel and start braking
  // in order to reach the final_rate exactly at the end of this block.
  if (plateau_steps < 0) {
    accelerate_steps = ceil(intersection_distance(initial_rate, final_rate, accel, block->step_event_count));
    accelerate_steps = max(accelerate_steps, 0); // Check limits due to numerical round-off
    accelerate_steps = min((uint32_t)accelerate_steps, block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    plateau_steps = 0;
  }

  // block->accelerate_until = accelerate_steps;
  // block->decelerate_after = accelerate_steps+plateau_steps;
  CRITICAL_SECTION_START;  // Fill variables used by the stepper in a critical section
  if (!block->busy) { // Don't update variables if block is busy.
    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps + plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
  }
  CRITICAL_SECTION_END;
}

// "Junction jerk" in this context is the immediate change in speed at the junction of two blocks.
// This method will calculate the junction jerk as the euclidean distance between the nominal
// velocities of the respective blocks.
//inline float junction_jerk(block_t *before, block_t *after) {
//  return sqrt(
//    pow((before->speed_x-after->speed_x), 2)+pow((before->speed_y-after->speed_y), 2));
//}


// The kernel called by recalculate() when scanning the plan from last to first entry.
void Planner::reverse_pass_kernel(block_t* previous, block_t* current, block_t* next) {
  if (!current) return;
  UNUSED(previous);

  if (next) {
    // If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
    // If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
    // check for maximum allowable speed reductions to ensure maximum possible planned speed.
    float max_entry_speed = current->max_entry_speed;
    if (current->entry_speed != max_entry_speed) {

      // If nominal length true, max junction speed is guaranteed to be reached. Only compute
      // for max allowable speed if block is decelerating and nominal length is false.
      if (!current->nominal_length_flag && max_entry_speed > next->entry_speed) {
        current->entry_speed = min(max_entry_speed,
                                   max_allowable_speed(-current->acceleration, next->entry_speed, current->millimeters));
      }
      else {
        current->entry_speed = max_entry_speed;
      }
      current->recalculate_flag = true;

    }
  } // Skip last block. Already initialized and set for recalculation.
}

/**
 * recalculate() needs to go over the current plan twice.
 * Once in reverse and once forward. This implements the reverse pass.
 */
void Planner::reverse_pass() {

  if (movesplanned() > 3) {

    block_t* block[3] = { NULL, NULL, NULL };

    // Make a local copy of block_buffer_tail, because the interrupt can alter it
    CRITICAL_SECTION_START;
      uint8_t tail = block_buffer_tail;
    CRITICAL_SECTION_END

    uint8_t b = BLOCK_MOD(block_buffer_head - 3);
    while (b != tail) {
      b = prev_block_index(b);
      block[2] = block[1];
      block[1] = block[0];
      block[0] = &block_buffer[b];
      reverse_pass_kernel(block[0], block[1], block[2]);
    }
  }
}

// The kernel called by recalculate() when scanning the plan from first to last entry.
void Planner::forward_pass_kernel(block_t* previous, block_t* current, block_t* next) {
  if (!previous) return;
  UNUSED(next);

  // If the previous block is an acceleration block, but it is not long enough to complete the
  // full speed change within the block, we need to adjust the entry speed accordingly. Entry
  // speeds have already been reset, maximized, and reverse planned by reverse planner.
  // If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
  if (!previous->nominal_length_flag) {
    if (previous->entry_speed < current->entry_speed) {
      double entry_speed = min(current->entry_speed,
                               max_allowable_speed(-previous->acceleration, previous->entry_speed, previous->millimeters));
      // Check for junction speed change
      if (current->entry_speed != entry_speed) {
        current->entry_speed = entry_speed;
        current->recalculate_flag = true;
      }
    }
  }
}

/**
 * recalculate() needs to go over the current plan twice.
 * Once in reverse and once forward. This implements the forward pass.
 */
void Planner::forward_pass() {
  block_t* block[3] = { NULL, NULL, NULL };

  for (uint8_t b = block_buffer_tail; b != block_buffer_head; b = next_block_index(b)) {
    block[0] = block[1];
    block[1] = block[2];
    block[2] = &block_buffer[b];
    forward_pass_kernel(block[0], block[1], block[2]);
  }
  forward_pass_kernel(block[1], block[2], NULL);
}

/**
 * Recalculate the trapezoid speed profiles for all blocks in the plan
 * according to the entry_factor for each junction. Must be called by
 * recalculate() after updating the blocks.
 */
void Planner::recalculate_trapezoids() {
  int8_t block_index = block_buffer_tail;
  block_t* current;
  block_t* next = NULL;

  while (block_index != block_buffer_head) {
    current = next;
    next = &block_buffer[block_index];
    if (current) {
      // Recalculate if current block entry or exit junction speed has changed.
      if (current->recalculate_flag || next->recalculate_flag) {
        // NOTE: Entry and exit factors always > 0 by all previous logic operations.
        float nom = current->nominal_speed;
        calculate_trapezoid_for_block(current, current->entry_speed / nom, next->entry_speed / nom);
        current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
      }
    }
    block_index = next_block_index(block_index);
  }
  // Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
  if (next) {
    float nom = next->nominal_speed;
    calculate_trapezoid_for_block(next, next->entry_speed / nom, (MINIMUM_PLANNER_SPEED) / nom);
    next->recalculate_flag = false;
  }
}

/*
 * Recalculate the motion plan according to the following algorithm:
 *
 *   1. Go over every block in reverse order...
 *
 *      Calculate a junction speed reduction (block_t.entry_factor) so:
 *
 *      a. The junction jerk is within the set limit, and
 *
 *      b. No speed reduction within one block requires faster
 *         deceleration than the one, true constant acceleration.
 *
 *   2. Go over every block in chronological order...
 *
 *      Dial down junction speed reduction values if:
 *      a. The speed increase within one block would require faster
 *         acceleration than the one, true constant acceleration.
 *
 * After that, all blocks will have an entry_factor allowing all speed changes to
 * be performed using only the one, true constant acceleration, and where no junction
 * jerk is jerkier than the set limit, Jerky. Finally it will:
 *
 *   3. Recalculate "trapezoids" for all blocks.
 */
void Planner::recalculate() {
  reverse_pass();
  forward_pass();
  recalculate_trapezoids();
}

/**
 * Maintain fans, paste extruder pressure,
 */
void Planner::check_axes_activity() {
  bool axis_active[NUM_AXIS] = { false };

  if (blocks_queued()) {
    block_t* block;
    for (uint8_t b = block_buffer_tail; b != block_buffer_head; b = next_block_index(b)) {
      block = &block_buffer[b];
      LOOP_ABCD(i) if (block->steps[i]) axis_active[i] = true;
    }
  }
  #if ENABLED(DISABLE_A)
    if (!axis_active[A_AXIS]) disable_a();
  #endif
  #if ENABLED(DISABLE_B)
    if (!axis_active[B_AXIS]) disable_b();
  #endif
  #if ENABLED(DISABLE_C)
    if (!axis_active[C_AXIS]) disable_c();
  #endif
  #if ENABLED(DISABLE_D)
    if (!axis_active[D_AXIS]) disable_d();
  #endif
}

/**
 * Planner::buffer_line
 *
 * Add a new linear movement to the buffer.
 *
 *  x,y,z,e   - target position in mm
 *  fr_mm_s   - (target) speed of the move
 *  extruder  - target extruder
 */

void Planner::buffer_line(const float& a, const float& b, const float& c, const float& d, float fr_mm_s) {
  // Calculate the buffer head after we push this byte
  int next_buffer_head = next_block_index(block_buffer_head);

  // If the buffer is full: good! That means we are well ahead of the robot.
  // Rest here until there is room in the buffer.
  while (block_buffer_tail == next_buffer_head) idle();

  // The target position of the tool in absolute steps
  // Calculate target position in absolute steps
  //this should be done after the wait, because otherwise a M92 code within the gcode disrupts this calculation somehow
  long target[NUM_AXIS] = {
    lround(a * axis_steps_per_mm[A_AXIS]),
    lround(b * axis_steps_per_mm[B_AXIS]),
    lround(c * axis_steps_per_mm[C_AXIS]),
    lround(d * axis_steps_per_mm[D_AXIS])
  };

  long da = target[A_AXIS] - position[A_AXIS],
       db = target[B_AXIS] - position[B_AXIS],
       dc = target[C_AXIS] - position[C_AXIS],
       dd = target[D_AXIS] - position[D_AXIS];

  // Prepare to set up new block
  block_t* block = &block_buffer[block_buffer_head];

  // Mark block as not busy (Not executed by the stepper interrupt)
  block->busy = false;

  // Number of steps for each axis
  block->steps[A_AXIS] = labs(da);
  block->steps[B_AXIS] = labs(db);
  block->steps[C_AXIS] = labs(dc);
  block->steps[D_AXIS] = labs(dd);
  block->step_event_count = max(block->steps[A_AXIS], max(block->steps[B_AXIS], max(block->steps[C_AXIS], block->steps[D_AXIS])));

  // Bail if this is a zero-length block
  if (block->step_event_count <= dropsegments) return;

  // Compute direction bits for this block
  uint8_t dirs = 0;
  if (da < 0) SBI(dirs, A_AXIS);
  if (db < 0) SBI(dirs, B_AXIS);
  if (dc < 0) SBI(dirs, C_AXIS);
  if (dd < 0) SBI(dirs, D_AXIS);
  block->direction_bits = dirs;

  // Enable active axes
  if (block->steps[A_AXIS]) enable_a();
  if (block->steps[B_AXIS]) enable_b();
  if (block->steps[C_AXIS]) enable_c();
  if (block->steps[D_AXIS]) enable_d();

  NOLESS(fr_mm_s, min_travel_feedrate_mm_s);

  /**
   * Calculate the total length of the movement
   */

  float delta_mm[4];
  delta_mm[A_AXIS] = da * steps_to_mm[A_AXIS];
  delta_mm[B_AXIS] = db * steps_to_mm[B_AXIS];
  delta_mm[C_AXIS] = dc * steps_to_mm[C_AXIS];
  delta_mm[D_AXIS] = dd * steps_to_mm[D_AXIS];

  #define DROP(N) (block->steps[N ##_AXIS] <= dropsegments)

  if (DROP(A) && DROP(B) && DROP(C) && DROP(D))
    block->millimeters = fabs(delta_mm[D_AXIS]);
  else
    block->millimeters = sqrt(sq(delta_mm[A_AXIS]) + sq(delta_mm[B_AXIS]) + sq(delta_mm[C_AXIS]) + sq(delta_mm[D_AXIS]));

  float inverse_millimeters = 1.0 / block->millimeters;  // Inverse millimeters to remove multiple divides

  // Calculate moves/second for this move. No divide by zero due to previous checks.
  float inverse_mm_s = fr_mm_s * inverse_millimeters;

  int moves_queued = movesplanned();

  block->nominal_speed = block->millimeters * inverse_mm_s; // (mm/sec) Always > 0
  block->nominal_rate = ceil(block->step_event_count * inverse_mm_s); // (step/sec) Always > 0

  // Calculate and limit speed in mm/sec for each axis
  float current_speed[NUM_AXIS];
  float speed_factor = 1.0; //factor <=1 do decrease speed
  LOOP_ABCD(i) {
    current_speed[i] = delta_mm[i] * inverse_mm_s;
    float cs = fabs(current_speed[i]), mf = max_feedrate_mm_s[i];
    if (cs > mf) speed_factor = min(speed_factor, mf / cs);
  }

  // Correct the speed
  if (speed_factor < 1.0) {
    LOOP_ABCD(i) current_speed[i] *= speed_factor;
    block->nominal_speed *= speed_factor;
    block->nominal_rate *= speed_factor;
  }

  // Compute and limit the acceleration rate for the trapezoid generator.
  float steps_per_mm = block->step_event_count / block->millimeters;
  // Limit acceleration per axis
  block->acceleration_steps_per_s2 = ceil((block->steps[D_AXIS] ? acceleration : travel_acceleration) * steps_per_mm);
  if (max_acceleration_steps_per_s2[A_AXIS] < (block->acceleration_steps_per_s2 * block->steps[A_AXIS]) / block->step_event_count)
    block->acceleration_steps_per_s2 = (max_acceleration_steps_per_s2[A_AXIS] * block->step_event_count) / block->steps[A_AXIS];
  if (max_acceleration_steps_per_s2[B_AXIS] < (block->acceleration_steps_per_s2 * block->steps[B_AXIS]) / block->step_event_count)
    block->acceleration_steps_per_s2 = (max_acceleration_steps_per_s2[B_AXIS] * block->step_event_count) / block->steps[B_AXIS];
  if (max_acceleration_steps_per_s2[C_AXIS] < (block->acceleration_steps_per_s2 * block->steps[C_AXIS]) / block->step_event_count)
    block->acceleration_steps_per_s2 = (max_acceleration_steps_per_s2[C_AXIS] * block->step_event_count) / block->steps[C_AXIS];
  if (max_acceleration_steps_per_s2[D_AXIS] < (block->acceleration_steps_per_s2 * block->steps[D_AXIS]) / block->step_event_count)
    block->acceleration_steps_per_s2 = (max_acceleration_steps_per_s2[D_AXIS] * block->step_event_count) / block->steps[D_AXIS];
  block->acceleration = block->acceleration_steps_per_s2 / steps_per_mm;
  block->acceleration_rate = (long)(block->acceleration_steps_per_s2 * 16777216.0 / ((F_CPU) * 0.125));

  // Start with a safe speed
  float vmax_junction = min(block->nominal_speed, max_z_jerk * 0.5),
        safe_speed = vmax_junction,
        vmax_junction_factor = 1.0;

  if ((moves_queued > 1) && (previous_nominal_speed > 0.0001)) {
    float dsa = fabs(current_speed[A_AXIS] - previous_speed[A_AXIS]),
          dsb = fabs(current_speed[B_AXIS] - previous_speed[B_AXIS]),
          dsc = fabs(current_speed[C_AXIS] - previous_speed[C_AXIS]),
          dsd = fabs(current_speed[D_AXIS] - previous_speed[D_AXIS]);

    if (dsa > max_z_jerk) vmax_junction_factor = min(vmax_junction_factor, max_z_jerk / dsa);
    if (dsb > max_z_jerk) vmax_junction_factor = min(vmax_junction_factor, max_z_jerk / dsb);
    if (dsc > max_z_jerk) vmax_junction_factor = min(vmax_junction_factor, max_z_jerk / dsc);
    if (dsd > max_z_jerk) vmax_junction_factor = min(vmax_junction_factor, max_z_jerk / dsd);

    vmax_junction = block->nominal_speed;
    vmax_junction = min(previous_nominal_speed, vmax_junction * vmax_junction_factor); // Limit speed to max previous speed
  }
  block->max_entry_speed = vmax_junction;

  // Initialize block entry speed. Compute based on deceleration to user-defined MINIMUM_PLANNER_SPEED.
  double v_allowable = max_allowable_speed(-block->acceleration, MINIMUM_PLANNER_SPEED, block->millimeters);
  block->entry_speed = min(vmax_junction, v_allowable);

  // Initialize planner efficiency flags
  // Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
  // If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
  // the current block and next block junction speeds are guaranteed to always be at their maximum
  // junction speeds in deceleration and acceleration, respectively. This is due to how the current
  // block nominal speed limits both the current and next maximum junction speeds. Hence, in both
  // the reverse and forward planners, the corresponding block junction speed will always be at the
  // the maximum junction speed and may always be ignored for any speed reduction checks.
  block->nominal_length_flag = (block->nominal_speed <= v_allowable);
  block->recalculate_flag = true; // Always calculate trapezoid for new block

  // Update previous path unit_vector and nominal speed
  LOOP_ABCD(i) previous_speed[i] = current_speed[i];
  previous_nominal_speed = block->nominal_speed;

  calculate_trapezoid_for_block(block, block->entry_speed / block->nominal_speed, safe_speed / block->nominal_speed);

  // Move buffer head
  block_buffer_head = next_buffer_head;

  // Update position
  LOOP_ABCD(i) position[i] = target[i];

  recalculate();

  stepper.wake_up();

} // buffer_line()

/**
 * Directly set the planner XYZ position (hence the stepper positions).
 *
 * On CORE machines stepper ABC will be translated from the given XYZ.
 */
void Planner::set_position_mm(const float& x, const float& y, const float& z, const float& e) {
  long nx = position[A_AXIS] = lround(x * axis_steps_per_mm[A_AXIS]),
       ny = position[B_AXIS] = lround(y * axis_steps_per_mm[B_AXIS]),
       nz = position[C_AXIS] = lround(z * axis_steps_per_mm[C_AXIS]),
       ne = position[D_AXIS] = lround(e * axis_steps_per_mm[D_AXIS]);
  stepper.set_position(nx, ny, nz, ne);
  previous_nominal_speed = 0.0; // Resets planner junction speeds. Assumes start from rest.

  LOOP_ABCD(i) previous_speed[i] = 0.0;
}

// Recalculate the steps/s^2 acceleration rates, based on the mm/s^2
void Planner::reset_acceleration_rates() {
  LOOP_ABCD(i)
    max_acceleration_steps_per_s2[i] = max_acceleration_mm_per_s2[i] * axis_steps_per_mm[i];
}

// Recalculate position, steps_to_mm if axis_steps_per_mm changes!
void Planner::refresh_positioning() {
  LOOP_ABCD(i) steps_to_mm[i] = 1.0 / axis_steps_per_mm[i];
  set_position_mm(current_position[A_AXIS], current_position[B_AXIS], current_position[C_AXIS], current_position[D_AXIS]);
  reset_acceleration_rates();
}
