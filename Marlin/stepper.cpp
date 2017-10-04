/*
  stepper.c - stepper motor driver: executes motion plans using stepper motors
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

/* The timer calculations of this module informed by the 'RepRap cartesian firmware' by Zack Smith
   and Philipp Tiefenbacher. */

#include "Marlin.h"
#include "stepper.h"
#include "planner.h"
#include "temperature.h"
#include "ultralcd.h"
#include "language.h"
#include "cardreader.h"
#include "speed_lookuptable.h"
#include "MarlinSerial.h"
#include <stdio.h>

//TRINAMIC Files
#include "TMC_SPI.h"
#include "TMC5130.h"


// Some useful constants
#define ENABLE_STEPPER_DRIVER_INTERRUPT()  TIMSK1 |= (1<<TOIE1 )
#define DISABLE_STEPPER_DRIVER_INTERRUPT() TIMSK1 &= ~(1<<TOIE1 )


block_t *current_block;  // A pointer to the block currently being traced

// Variables used by The Stepper Driver Interrupt
static unsigned char out_bits;        // The next stepping-bits to be output

// Additional motion execution queue for use with Trinamic TMC5130
#define MOTION_BUFFER_SIZE 16
typedef struct {
	unsigned long initial_speed_x, initial_speed_y, initial_speed_z, initial_speed_e;
	unsigned long nominal_speed_x, nominal_speed_y, nominal_speed_z, nominal_speed_e;
	unsigned long final_speed_x, final_speed_y, final_speed_z, final_speed_e;
	unsigned long accel_x, accel_y, accel_z, accel_e;
	long pos_x, pos_y, pos_z, pos_e;
	long nextTimerClk;
	bool calcready;
	bool pos_change_z;
} motion_block_t;
static motion_block_t motion_buffer[MOTION_BUFFER_SIZE];	// A ring buffer for motion movements
static motion_block_t motion_buffer_block_old;
static volatile unsigned char motion_buffer_head = 0;           // Index of the next block to be pushed
static volatile unsigned char motion_buffer_tail = 0;
static volatile bool motion_buffer_full = false;


// Variables for Trinamic Positioning Mode
static long pos_x, pos_y, pos_z, pos_e;
// Conversion of units between TMC5130 and Arduino
#define TMC5130clockFrequency (double) 16000000
#define TMC5130_a_divisor (long) 128
#define TMC5130_t_factor (double) 1.048576
static volatile long timerClk, nextTimerClk;
static volatile double TEMPtimerClk;

volatile long endstops_trigsteps[3]={0,0,0};
volatile long endstops_stepsTotal,endstops_stepsDone;

volatile long count_position[NUM_AXIS] = { 0, 0, 0, 0};
volatile signed char count_direction[NUM_AXIS] = { 1, 1, 1, 1};



/*******************************************************************
*                 private functions for motion queue
*******************************************************************/

/**
 * @brief Called when the current block is no longer needed. Discards the block and makes the memory
 * available for new blocks.)
 * @return	none
 *****************************************************************************/
FORCE_INLINE void discard_current_motion_block(void)
{
	motion_buffer_tail = (motion_buffer_tail + 1) & (MOTION_BUFFER_SIZE - 1);
	motion_buffer_full = false;
}


/**
 * @brief  Gets the current block. Returns NULL if buffer empty
 * @return pointer of the current block, null if empty
 *****************************************************************************/
FORCE_INLINE motion_block_t *get_current_motion_block(void)
{
  if ((motion_buffer_head == motion_buffer_tail) && (motion_buffer_full == false)) {
    return(NULL);
  }
  else
    return &motion_buffer[motion_buffer_tail];
}


/**
 * @brief  Returns true if the buffer has a queued block, false otherwise
 * @return	false if queue emty, else true
 *****************************************************************************/
FORCE_INLINE bool motion_blocks_queued(void)
{
	return ((motion_buffer_head != motion_buffer_tail) || motion_buffer_full);
}


/**
 * @brief  Returns a pointer to the next free position in the motion buffer, if there is one
 * @return	pointer of next free block
 *****************************************************************************/
FORCE_INLINE motion_block_t *get_next_free_motion_block(void)
{
	if(motion_buffer_full)
		return NULL;
	else
		return &motion_buffer[motion_buffer_head];
}


/**
 * @brief  Increases the head pointer of the buffer. Don't use without proving
 * before that the queue is not full
 * @return	none
 *****************************************************************************/
FORCE_INLINE void append_motion_block(void) {
  CRITICAL_SECTION_START;
	motion_buffer_head += 1;
	if(motion_buffer_head == MOTION_BUFFER_SIZE)
		motion_buffer_head = 0;
  if(motion_buffer_head == motion_buffer_tail)
    motion_buffer_full = true;
  CRITICAL_SECTION_END;
}

/**
 * @brief  Returns the number of queued blocks
 * @return	number of queued blocks
 *****************************************************************************/
unsigned char blocks_in_motion_queue() {
  char temp = motion_buffer_head - motion_buffer_tail;
  if(temp < 0)
    temp += MOTION_BUFFER_SIZE;
  return temp;
}




/*******************************************************************
*                 public functions
*******************************************************************/


//         __________________________
//        /|                        |\     _________________         ^
//       / |                        | \   /|               |\        |
//      /  |                        |  \ / |               | \       s
//     /   |                        |   |  |               |  \      p
//    /    |                        |   |  |               |   \     e
//   +-----+------------------------+---+--+---------------+----+    e
//   |               BLOCK 1            |      BLOCK 2          |    d
//
//                           time ----->
//
//  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates
//  first block->accelerate_until step_events_completed, then keeps going at constant speed until
//  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
//  The slope of acceleration is calculated with the leib ramp alghorithm.

/**
 * @brief Enables timer interrupt
 * @return none
 *****************************************************************************/
void st_wake_up(void)
{
  //  TCNT1 = 0;
  ENABLE_STEPPER_DRIVER_INTERRUPT();
}


/**
 * @brief Pre-calculates the parameters for the current block, in order to save time. Hence, in the interrupt function
 * there is no processing, only sending the data via SPI
 * @return none
 *****************************************************************************/
void st_calculate(void)
{
	unsigned long scale_axis;	// scale factor
	motion_block_t *current_motion_block = get_next_free_motion_block();

	// block in motion_block available?
	if(current_motion_block == NULL)
		return;

	// If there is no current block, attempt to pop one from the buffer
	current_block = plan_get_current_block();
	// Anything in the buffer?
	if (current_block != NULL)
		current_block->busy = true; //Block being used
	else
		return;

	// Set directions
	out_bits = current_block->direction_bits;

	// Calculate ramp parameters of the current block
	// If there is any movement in the x-axis
	if(current_block->steps_x != 0)
	{
		//accel_x = current_block->steps_x * current_block->acceleration_st / current_block->step_event_count
		scale_axis = (((unsigned long)current_block->steps_x)<<16) / current_block->step_event_count;
		current_motion_block->accel_x = (scale_axis>>7) * current_block->acceleration_st;
		current_motion_block->accel_x = (current_motion_block->accel_x>>16);


		// Keep acceleration high to be able to brake
		if(current_motion_block->accel_x == 0)
			current_motion_block->accel_x = 1000;

		//initial_speed_x = current_block->steps_x * current_block->initial_rate / current_block->step_event_count;
		current_motion_block->initial_speed_x = scale_axis * current_block->initial_rate;
		current_motion_block->initial_speed_x = (current_motion_block->initial_speed_x >> 16);

		//nominal_speed_x = current_block->steps_x * current_block->nominal_rate / current_block->step_event_count;
		current_motion_block->nominal_speed_x = scale_axis * current_block->nominal_rate;
		current_motion_block->nominal_speed_x = (current_motion_block->nominal_speed_x>>16);

		if(current_motion_block->nominal_speed_x < current_motion_block->initial_speed_x)
			current_motion_block->initial_speed_x = current_motion_block->nominal_speed_x;

		//final_speed_x = current_block->steps_x * current_block->final_rate / current_block->step_event_count;
		current_motion_block->final_speed_x = scale_axis * current_block->final_rate;
		current_motion_block->final_speed_x = (current_motion_block->final_speed_x>>16);

		if(current_motion_block->final_speed_x < 10)
			current_motion_block->final_speed_x = 10;

		if((out_bits & (1<<X_AXIS))!=0)//speed_x = - speed_x;
		{
			pos_x -= current_block->steps_x;
			current_motion_block->pos_x = pos_x;
		}
		else
		{
			pos_x += current_block->steps_x;
			current_motion_block->pos_x = pos_x;
		}

		//store the calculate values in motion_buffer_block_old
		motion_buffer_block_old.accel_x = current_motion_block->accel_x;
		motion_buffer_block_old.initial_speed_x = current_motion_block->initial_speed_x;
		motion_buffer_block_old.nominal_speed_x = current_motion_block->nominal_speed_x;
		motion_buffer_block_old.final_speed_x = current_motion_block->final_speed_x;
		motion_buffer_block_old.pos_x = current_motion_block->pos_x;
	}
	else
	{
		//If there is no movement in the x-axis use the last calculate values
		current_motion_block->accel_x = motion_buffer_block_old.accel_x;
		current_motion_block->initial_speed_x = motion_buffer_block_old.initial_speed_x;
		current_motion_block->nominal_speed_x = motion_buffer_block_old.nominal_speed_x;
		current_motion_block->final_speed_x = motion_buffer_block_old.final_speed_x;
		current_motion_block->pos_x = motion_buffer_block_old.pos_x;
	}


	// If there is any movement in the y-axis
	if(current_block->steps_y != 0)
	{
		//accel_y = current_block->steps_y * current_block->acceleration_st / current_block->step_event_count;
		scale_axis = (((unsigned long)current_block->steps_y)<<16) / current_block->step_event_count;
		current_motion_block->accel_y = (scale_axis>>7) * current_block->acceleration_st;
		current_motion_block->accel_y = (current_motion_block->accel_y>>16);


		// Keep acceleration high to be able to brake
		if(current_motion_block->accel_y == 0)
			current_motion_block->accel_y = 1000;

		//initial_speed_y = current_block->steps_y * current_block->initial_rate / current_block->step_event_count;
		current_motion_block->initial_speed_y = scale_axis * current_block->initial_rate;
		current_motion_block->initial_speed_y = (current_motion_block->initial_speed_y>>16);

		//nominal_speed_y = current_block->steps_y * current_block->nominal_rate / current_block->step_event_count;
		current_motion_block->nominal_speed_y = scale_axis * current_block->nominal_rate;
		current_motion_block->nominal_speed_y = (current_motion_block->nominal_speed_y>>16);

		if(current_motion_block->nominal_speed_y < current_motion_block->initial_speed_y)
			current_motion_block->initial_speed_y = current_motion_block->nominal_speed_y;

		//final_speed_y = current_block->steps_y * current_block->final_rate / current_block->step_event_count;
		current_motion_block->final_speed_y = scale_axis * current_block->final_rate;
		current_motion_block->final_speed_y = (current_motion_block->final_speed_y>>16);

		if(current_motion_block->final_speed_y < 10)
			current_motion_block->final_speed_y = 10;

		if((out_bits & (1<<Y_AXIS))!=0)//speed_y = - speed_y;
		{
			pos_y -= current_block->steps_y;
			current_motion_block->pos_y = pos_y;
		}
		else
		{
			pos_y += current_block->steps_y;
			current_motion_block->pos_y = pos_y;
		}

		//store the calculate values in motion_buffer_block_old
		motion_buffer_block_old.accel_y = current_motion_block->accel_y;
		motion_buffer_block_old.initial_speed_y = current_motion_block->initial_speed_y;
		motion_buffer_block_old.nominal_speed_y = current_motion_block->nominal_speed_y;
		motion_buffer_block_old.final_speed_y = current_motion_block->final_speed_y;
		motion_buffer_block_old.pos_y = current_motion_block->pos_y;
	}
	else
	{
		//If there is no movement in the y-axis use the last calculate values
		current_motion_block->accel_y = motion_buffer_block_old.accel_y;
		current_motion_block->initial_speed_y = motion_buffer_block_old.initial_speed_y;
		current_motion_block->nominal_speed_y = motion_buffer_block_old.nominal_speed_y;
		current_motion_block->final_speed_y = motion_buffer_block_old.final_speed_y;
		current_motion_block->pos_y = motion_buffer_block_old.pos_y;
	}

	// If there is any movement in the z-axis
	if(current_block->steps_z != 0)
	{
		//accel_z = current_block->steps_z * current_block->acceleration_st / current_block->step_event_count;
		scale_axis = (((unsigned long)current_block->steps_z)<<16) / current_block->step_event_count;
		current_motion_block->accel_z = (scale_axis>>7) * current_block->acceleration_st;
		current_motion_block->accel_z = (current_motion_block->accel_z>>16);


		// Keep acceleration high to be able to brake
		if(current_motion_block->accel_z == 0)
			current_motion_block->accel_z = 1000;

		//initial_speed_z = current_block->steps_z * current_block->initial_rate / current_block->step_event_count;
		current_motion_block->initial_speed_z = scale_axis * current_block->initial_rate;
		current_motion_block->initial_speed_z = (current_motion_block->initial_speed_z>>16);

		//nominal_speed_z = current_block->steps_z * current_block->nominal_rate / current_block->step_event_count;
		current_motion_block->nominal_speed_z = scale_axis * current_block->nominal_rate;
		current_motion_block->nominal_speed_z = (current_motion_block->nominal_speed_z>>16);

		if(current_motion_block->nominal_speed_z < current_motion_block->initial_speed_z)
			current_motion_block->initial_speed_z = current_motion_block->nominal_speed_z;

		//final_speed_z = current_block->steps_z * current_block->final_rate / current_block->step_event_count;
		current_motion_block->final_speed_z = scale_axis * current_block->final_rate;
		current_motion_block->final_speed_z = (current_motion_block->final_speed_z>>16);

		if(current_motion_block->final_speed_z < 10)
			current_motion_block->final_speed_z = 10;
		if(current_motion_block->final_speed_z > 800)
			current_motion_block->final_speed_z = 800;

		if((out_bits & (1<<Z_AXIS))!=0)//speed_z = - speed_z;
		{
			pos_z -= current_block->steps_z;
			current_motion_block->pos_z = pos_z;
		}
		else
		{
			pos_z += current_block->steps_z;
			current_motion_block->pos_z = pos_z;
		}

		current_motion_block->pos_change_z = true;
	}
	else
	{
		current_motion_block->pos_change_z = false;
	}

	// If there is any movement in the e-axis
	if(current_block->steps_e != 0)
	{
		//accel_e = current_block->steps_e * current_block->acceleration_st / current_block->step_event_count;
		scale_axis = (((unsigned long)current_block->steps_e)<<16) / current_block->step_event_count;
		current_motion_block->accel_e = (scale_axis>>7) * current_block->acceleration_st;
		current_motion_block->accel_e = (current_motion_block->accel_e>>16);




		// Keep acceleration high to be able to brake
		if(current_motion_block->accel_e == 0)
			current_motion_block->accel_e = 1000;

		//initial_speed_e = current_block->steps_e * current_block->initial_rate / current_block->step_event_count;
		current_motion_block->initial_speed_e = scale_axis * current_block->initial_rate;
		current_motion_block->initial_speed_e = (current_motion_block->initial_speed_e>>16);


		//nominal_speed_e = current_block->steps_e * current_block->nominal_rate / current_block->step_event_count;
		current_motion_block->nominal_speed_e = scale_axis * current_block->nominal_rate;
		current_motion_block->nominal_speed_e = (current_motion_block->nominal_speed_e>>16);

		if(current_motion_block->nominal_speed_e < current_motion_block->initial_speed_e)
			current_motion_block->initial_speed_e = current_motion_block->nominal_speed_e;

		//final_speed_e = current_block->steps_e * current_block->final_rate / current_block->step_event_count;
		current_motion_block->final_speed_e = scale_axis * current_block->final_rate;
		current_motion_block->final_speed_e = (current_motion_block->final_speed_e>>16);

		if(current_motion_block->final_speed_e < 10)
			current_motion_block->final_speed_e = 10;
		if(current_motion_block->final_speed_e > 800)
			current_motion_block->final_speed_e = 800;

		if((out_bits & (1<<E_AXIS))!=0)//speed_e = - speed_e;
		{
			pos_e -= current_block->steps_e;
			current_motion_block->pos_e = pos_e;
		}
		else
		{
			pos_e += current_block->steps_e;
			current_motion_block->pos_e = pos_e;
		}

		//store the calculate values in motion_buffer_block_old
		motion_buffer_block_old.accel_e = current_motion_block->accel_e;
		motion_buffer_block_old.initial_speed_e = current_motion_block->initial_speed_e;
		motion_buffer_block_old.nominal_speed_e = current_motion_block->nominal_speed_e;
		motion_buffer_block_old.final_speed_e = current_motion_block->final_speed_e;
		motion_buffer_block_old.pos_e = current_motion_block->pos_e;
	}
	else
	{
		//If there is no movement in the e-axis use the last calculate values
		current_motion_block->accel_e = motion_buffer_block_old.accel_e;
		current_motion_block->initial_speed_e = motion_buffer_block_old.initial_speed_e;
		current_motion_block->nominal_speed_e = motion_buffer_block_old.nominal_speed_e;
		current_motion_block->final_speed_e = motion_buffer_block_old.final_speed_e;
		current_motion_block->pos_e = motion_buffer_block_old.pos_e;
	}

	// Calculate duration of the movement
	// I - Acceleration phase
	double temp;
	//timerClk = (current_block->nominal_rate - current_block->initial_rate) / current_block->acceleration_st;
	nextTimerClk = current_block->nominal_rate - current_block->initial_rate;
	temp = (double) nextTimerClk /  current_block->acceleration_st;
	nextTimerClk = temp * 2000000;

	// II - Plateau / Constant speed phase (if applies)
	if(current_block->decelerate_after > current_block->accelerate_until)
	{	//timer += phase2duration;
		//timerClk = timerClk + (2000 *1000 * (current_block->decelerate_after - current_block->accelerate_until) / current_block->nominal_rate);
		TEMPtimerClk = current_block->decelerate_after - current_block->accelerate_until;
		TEMPtimerClk = TEMPtimerClk / current_block->nominal_rate;
		TEMPtimerClk = TEMPtimerClk * 2000000;
		nextTimerClk = nextTimerClk + TEMPtimerClk;
	}

	// III - Deceleration phase
	//timerClk = 2000 * 1000 * (current_block->nominal_rate - current_block->final_rate) / current_block->acceleration_st;
	TEMPtimerClk = current_block->nominal_rate - current_block->final_rate;
	TEMPtimerClk = TEMPtimerClk / current_block->acceleration_st;
	TEMPtimerClk = TEMPtimerClk * 2000000;
	TEMPtimerClk = nextTimerClk + TEMPtimerClk;

	// Adjust value for the internal units of the chip (we changed units of the acceleration, but speed has also internal units)
	nextTimerClk = TEMPtimerClk * TMC5130_t_factor;

	current_motion_block->nextTimerClk = nextTimerClk;
	motion_buffer_block_old.nextTimerClk = current_motion_block->nextTimerClk;



	// Calculations finished: Let the interrupt send the information
	current_motion_block->calcready = true;
	motion_buffer_block_old.calcready = current_motion_block->calcready;
	append_motion_block();
	current_block = NULL;
	plan_discard_current_block();
}


/**
 * @brief "The Stepper Driver Interrupt" - This timer interrupt is the workhorse.
 * It has been divided into st_calculate and this function. In this part we send the information of the current
 * block, once the calculations of st_calculate are ready (current_block->ready) and set the timer for the next
 * wake-up
 * @return none
 *****************************************************************************/
ISR(TIMER1_OVF_vect)
{
	unsigned long temp_timer;
	motion_block_t *current_motion_block;

	if(timerClk == 0)
	{
		current_motion_block =  get_current_motion_block();
		if((current_motion_block->calcready == true) && (current_motion_block != NULL))
  		{
  			// Send accel and nominal speed SPI datagrams
			spi_writeRegister(VMAX, current_motion_block->nominal_speed_x, XAXIS);//Velocity of X
			spi_writeRegister(AMAX, current_motion_block->accel_x, XAXIS);//ACC of X
			spi_writeRegister(DMAX, current_motion_block->accel_x, XAXIS);//DEC of X
			spi_writeRegister(VMAX, current_motion_block->nominal_speed_y, YAXIS);//Velocity of Y
			spi_writeRegister(AMAX, current_motion_block->accel_y, YAXIS);//ACC of Y
			spi_writeRegister(DMAX, current_motion_block->accel_y, YAXIS);//DEC of Y

  			// only send if there is any movement in the z-axis
  			if(current_motion_block->pos_change_z == true){
  				spi_writeRegister(VMAX, current_motion_block->nominal_speed_z, ZAXIS);//Velocity of ZAXIS
				spi_writeRegister(AMAX, current_motion_block->accel_z, ZAXIS);//ACC of ZAXIS
				spi_writeRegister(DMAX, current_motion_block->accel_z, ZAXIS);//DEC of ZAXIS
  			}

  			//check if new data receive
  			st_check_UART_rx();

  			spi_writeRegister(VMAX, current_motion_block->nominal_speed_e, E0AXIS);//Velocity of E0AXIS
  			spi_writeRegister(AMAX, current_motion_block->accel_e, E0AXIS);//ACC of E0AXIS
  			spi_writeRegister(DMAX, current_motion_block->accel_e, E0AXIS);//DEC of E0AXIS

  			// Send initial and final speeed SPI datagrams
  			spi_writeRegister(VSTART, current_motion_block->initial_speed_x, XAXIS);//Initial vel of X
  			spi_writeRegister(VSTOP, current_motion_block->final_speed_x, XAXIS);//Final vel of X
  			spi_writeRegister(VSTART, current_motion_block->initial_speed_y, YAXIS);//Initial vel of Y
  			spi_writeRegister(VSTOP, current_motion_block->final_speed_y, YAXIS);//Final vel of Y

  			// only send if there is any movement in the z-axis
  			if(current_motion_block->pos_change_z == true){
  				spi_writeRegister(VSTART, current_motion_block->initial_speed_z, ZAXIS);//Initial vel of Z
  				spi_writeRegister(VSTOP, current_motion_block->final_speed_z, ZAXIS);//Final vel of Z
  			}

  			//check if new data receive
  			st_check_UART_rx();

  			spi_writeRegister(VSTART, current_motion_block->initial_speed_e, E0AXIS);//Initial vel of E0
  			spi_writeRegister(VSTOP, current_motion_block->final_speed_e, E0AXIS);//Final vel of E0

  			// Send target positions, movement starts immediately
  			// only send if there is any movement in the z-axis
  			if(current_motion_block->pos_change_z == true)
  				spi_writeRegister(XTARGET, current_motion_block->pos_z, ZAXIS);// target

  			spi_writeRegister(XTARGET, current_motion_block->pos_x, XAXIS);// target
  			spi_writeRegister(XTARGET, current_motion_block->pos_y, YAXIS);// target
  			spi_writeRegister(XTARGET, current_motion_block->pos_e, E0AXIS);// target

  			// Let's wake up back when this movement is over
  			timerClk = current_motion_block->nextTimerClk;

  			// Discard the current_block and let st_calculate start with the next one
  			current_motion_block->calcready = false;
  			discard_current_motion_block();
		}
		else
		{
			timerClk = 2000; // Check in a short time, if a new block is calculated
		}
	}

	// Update 32-bit software-extended timer
	if(timerClk >= 65535)
	{
		timerClk = timerClk - 65535;
		TCNT1 = 1;
	}
	else
	{
		temp_timer = TCNT1;

		if(timerClk > (temp_timer + 100))
		{
			TCNT1 = 65535 - timerClk + temp_timer;
			timerClk = 0;
		}
		else
		{
			TCNT1 = 65436;
			timerClk = 0;
		}
	}
}


/**
 * @brief Initialize the endstops, pullups and timer configurations.
 * @return none
 *****************************************************************************/
void st_init(void)
{

  //endstops and pullups
  #if defined(X_MIN_PIN) && X_MIN_PIN > -1
    SET_INPUT(X_MIN_PIN);
    #ifdef ENDSTOPPULLUP_XMIN
      WRITE(X_MIN_PIN,HIGH);
    #endif
  #endif

  #if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
    SET_INPUT(Y_MIN_PIN);
    #ifdef ENDSTOPPULLUP_YMIN
      WRITE(Y_MIN_PIN,HIGH);
    #endif
  #endif

  #if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
    SET_INPUT(Z_MIN_PIN);
    #ifdef ENDSTOPPULLUP_ZMIN
      WRITE(Z_MIN_PIN,HIGH);
    #endif
  #endif

  #if defined(X_MAX_PIN) && X_MAX_PIN > -1
    SET_INPUT(X_MAX_PIN);
    #ifdef ENDSTOPPULLUP_XMAX
      WRITE(X_MAX_PIN,HIGH);
    #endif
  #endif

  #if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
    SET_INPUT(Y_MAX_PIN);
    #ifdef ENDSTOPPULLUP_YMAX
      WRITE(Y_MAX_PIN,HIGH);
    #endif
  #endif

  #if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
    SET_INPUT(Z_MAX_PIN);
    #ifdef ENDSTOPPULLUP_ZMAX
      WRITE(Z_MAX_PIN,HIGH);
    #endif
  #endif


    //Trinamic timer
    // waveform generation = 0000 = NORMAL
    TCCR1B &= ~(1<<WGM13);
    TCCR1B &= ~(1<<WGM12);
    TCCR1A &= ~(1<<WGM11);
    TCCR1A &= ~(1<<WGM10);

    // output mode = 00 (disconnected)
    TCCR1A &= ~(3<<COM1A0);
    TCCR1A &= ~(3<<COM1B0);

    // Set the timer pre-scaler
    // Generally we use a divider of 8, resulting in a 2MHz timer
    // frequency on a 16MHz MCU. If you are going to change this, be
    // sure to regenerate speed_lookuptable.h with
    // create_speed_lookuptable.py
    TCCR1B = (TCCR1B & ~(0x07<<CS10)) | (2<<CS10);

    TCNT1 = 1;
    ENABLE_STEPPER_DRIVER_INTERRUPT();

    sei();	// global interrupt enable
}


/**
 * @brief Block until all buffered steps are executed
 * @return none
 *****************************************************************************/
void st_synchronize(void)
{
	while(blocks_queued()){
		manage_heater();
		manage_inactivity();
		lcd_update();
		//Trinamic: calculate data for next blocks while waiting
		st_calculate();
	}

	while(motion_blocks_queued()){
		manage_heater();
		manage_inactivity();
		lcd_update();
		//process_commands();
	}
}


/**
 * @brief Set current position in steps
 * @param &x		position for x axis
 * @param &y		position for y axis
 * @param &z		position for z axis
 * @param &e		position for e axis
 * @return none
 *****************************************************************************/
void st_set_position(const long &x, const long &y, const long &z, const long &e)
{

  CRITICAL_SECTION_START;
  if(pos_x != x)
  {
	  count_position[X_AXIS] = pos_x = x;
	  motion_buffer_block_old.pos_x = x;
	  // Update positino in the driver
	  spi_writeRegister(RAMPMODE, HOLD_MODE, XAXIS);	//HOLD Mode
	  spi_writeRegister(XTARGET, pos_x, XAXIS);
	  spi_writeRegister(XACTUAL, pos_x, XAXIS);
	  spi_writeRegister(RAMPMODE, POSITIONING_MODE, XAXIS);	//Position MODE
  }
  if(pos_y != y)
  {
	  count_position[Y_AXIS] = pos_y = y;
	  motion_buffer_block_old.pos_y = y;
	  // Update positino in the driver
	  spi_writeRegister(RAMPMODE, HOLD_MODE, YAXIS);	//HOLD Mode
	  spi_writeRegister(XTARGET, pos_y, YAXIS);
	  spi_writeRegister(XACTUAL, pos_y, YAXIS);
	  spi_writeRegister(RAMPMODE, POSITIONING_MODE, YAXIS);	//Position MODE
  }
  if(pos_z != z)
  {
	  count_position[Z_AXIS] = pos_z = z;
	  motion_buffer_block_old.pos_z = z;
	  // Update positino in the driver
	  spi_writeRegister(RAMPMODE, HOLD_MODE, ZAXIS);	//HOLD Mode
	  spi_writeRegister(XTARGET, pos_z, ZAXIS);
	  spi_writeRegister(XACTUAL, pos_z, ZAXIS);
	  spi_writeRegister(RAMPMODE, POSITIONING_MODE, ZAXIS);	//Position MODE
  }
  if(pos_e != e)
  {
	  count_position[E_AXIS] = pos_e = e;
	  motion_buffer_block_old.pos_e = e;
	  // Update positino in the driver
	  spi_writeRegister(RAMPMODE, HOLD_MODE, E0AXIS);	//HOLD Mode
	  spi_writeRegister(XTARGET, pos_e, E0AXIS);
	  spi_writeRegister(XACTUAL, pos_e, E0AXIS);
	  spi_writeRegister(RAMPMODE, POSITIONING_MODE, E0AXIS);	//Position MODE
  }
  CRITICAL_SECTION_END;
}


/**
 * @brief Set current position in steps
 * @param &e		position for e axis
 * @return none
 *****************************************************************************/
void st_set_e_position(const long &e)
{
  CRITICAL_SECTION_START;
  if(pos_e != e)
  {
	  count_position[E_AXIS] = pos_e = e;
	  motion_buffer_block_old.pos_e = e;
	  // Update positino in the driver
	  spi_writeRegister(RAMPMODE, HOLD_MODE, E0AXIS);	//HOLD Mode
	  spi_writeRegister(XTARGET, pos_e, E0AXIS);
	  spi_writeRegister(XACTUAL, pos_e, E0AXIS);
	  spi_writeRegister(RAMPMODE, POSITIONING_MODE, E0AXIS);	//Position MODE
  }
  CRITICAL_SECTION_END;
}


/**
 * @brief Get current position in steps
 * @param axis	 axis
 * @return current position in steps
 *****************************************************************************/
long st_get_position(uint8_t axis)
{
  long count_pos;
  CRITICAL_SECTION_START;
  count_pos = count_position[axis];
  CRITICAL_SECTION_END;
  return count_pos;
}


/**
 * @brief Disable all drivers
 * @return none
 *****************************************************************************/
void finishAndDisableSteppers(void)
{
  st_synchronize();
  disable_x();
  disable_y();
  disable_z();
  disable_e0();
  disable_e1();
  disable_e2();

  //Trinamic Drivers -> Disable
  TMC5130_disableDriver(X_AXIS);
  TMC5130_disableDriver(Y_AXIS);
  TMC5130_disableDriver(Z_AXIS);
  TMC5130_disableDriver(E_AXIS);

}


