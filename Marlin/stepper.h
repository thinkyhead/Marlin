/*
  stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef stepper_h
#define stepper_h

#include "planner.h"

#if EXTRUDERS > 2
  #define WRITE_E_STEP(v) { if(current_block->active_extruder == 2) { WRITE(E2_STEP_PIN, v); } else { if(current_block->active_extruder == 1) { WRITE(E1_STEP_PIN, v); } else { WRITE(E0_STEP_PIN, v); }}}
  #define NORM_E_DIR() { if(current_block->active_extruder == 2) { WRITE(E2_DIR_PIN, !INVERT_E2_DIR); } else { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); }}}
  #define REV_E_DIR() { if(current_block->active_extruder == 2) { WRITE(E2_DIR_PIN, INVERT_E2_DIR); } else { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, INVERT_E0_DIR); }}}
#elif EXTRUDERS > 1
  #ifndef DUAL_X_CARRIAGE
    #define WRITE_E_STEP(v) { if(current_block->active_extruder == 1) { WRITE(E1_STEP_PIN, v); } else { WRITE(E0_STEP_PIN, v); }}
    #define NORM_E_DIR() { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); }}
    #define REV_E_DIR() { if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, INVERT_E0_DIR); }}
  #else
    extern bool extruder_duplication_enabled;
    #define WRITE_E_STEP(v) { if(extruder_duplication_enabled) { WRITE(E0_STEP_PIN, v); WRITE(E1_STEP_PIN, v); } else if(current_block->active_extruder == 1) { WRITE(E1_STEP_PIN, v); } else { WRITE(E0_STEP_PIN, v); }}
    #define NORM_E_DIR() { if(extruder_duplication_enabled) { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, !INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, !INVERT_E0_DIR); }}
    #define REV_E_DIR() { if(extruder_duplication_enabled) { WRITE(E0_DIR_PIN, INVERT_E0_DIR); WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else if(current_block->active_extruder == 1) { WRITE(E1_DIR_PIN, INVERT_E1_DIR); } else { WRITE(E0_DIR_PIN, INVERT_E0_DIR); }}
  #endif
#else
  #define WRITE_E_STEP(v) WRITE(E0_STEP_PIN, v)
  #define NORM_E_DIR() WRITE(E0_DIR_PIN, !INVERT_E0_DIR)
  #define REV_E_DIR() WRITE(E0_DIR_PIN, INVERT_E0_DIR)
#endif


extern block_t *current_block;  // A pointer to the block currently being traced


/**
 * @brief Initialize the endstops, pullups and timer configurations.
 * @return none
 *****************************************************************************/
void st_init(void);


/**
 * @brief Block until all buffered steps are executed
 * @return none
 *****************************************************************************/
void st_synchronize(void);

/**
 * @brief Pre-calculates the parameters for the current block, in order to save time. Hence, in the interrupt function
 * there is no processing, only sending the data via SPI
 * @return none
 *****************************************************************************/
void st_calculate(void);


/**
 * @brief Set current position in steps
 * @param &x		position for x axis
 * @param &y		position for y axis
 * @param &z		position for z axis
 * @param &e		position for e axis
 * @return none
 *****************************************************************************/
void st_set_position(const long &x, const long &y, const long &z, const long &e);


/**
 * @brief Set current position in steps
 * @param &e		position for e axis
 * @return none
 *****************************************************************************/
void st_set_e_position(const long &e);


/**
 * @brief Get current position in steps
 * @param axis	 axis
 * @return current position in steps
 *****************************************************************************/
long st_get_position(uint8_t axis);


/**
 * @brief Enables timer interrupt
 * @return none
 *****************************************************************************/
void st_wake_up(void);


/**
 * @brief Disable all drivers
 * @return none
 *****************************************************************************/
void finishAndDisableSteppers(void);


/**
 * @brief  Returns the number of queued blocks
 * @return	number of queued blocks
 *****************************************************************************/
unsigned char blocks_in_motion_queue();

#endif
