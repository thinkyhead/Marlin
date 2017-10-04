/*
 * TMC5130.h
 *
 *  Created on: 27.05.2016
 *      Author: tmctest
 */

#ifndef TMC5130_H_
#define TMC5130_H_


// Marlin Files
#include <stdbool.h>
#include "Marlin.h"

// TRINAMIC TMC5130 Register Address Defines
#define GCONF				0x00 	//Global configuration flags
#define X_COMPARE 			0x05	//Position  comparison  register
#define IHOLD_IRUN			0x10	//Driver current control
#define TCOOLTHRS			0x14	//This is the lower threshold velocity for switching on smart energy coolStep and stallGuard feature.
#define RAMPMODE			0x20	//Driving mode (Velocity, Positioning, Hold)
#define XACTUAL				0x21	//Actual motor position
#define VACTUAL 			0x22	//Actual  motor  velocity  from  ramp  generator
#define VSTART				0x23	//Motor start velocity
#define A_1					0x24	//First  acceleration  between  VSTART  and  V1
#define V_1					0x25	//First  acceleration  /  deceleration  phase  target velocity
#define AMAX				0x26	//Second  acceleration  between  V1  and  VMAX
#define VMAX 				0x27	//This is the target velocity in velocity mode. It can be changed any time during a motion.
#define DMAX				0x28	//Deceleration between VMAX and V1
#define D_1					0x2A 	//Deceleration  between  V1  and  VSTOP
									//Attention:  Do  not  set  0  in  positioning  mode, even if V1=0!
#define VSTOP				0x2B	//Motor stop velocity (unsigned)
									//Attention: Set VSTOP > VSTART!
									//Attention:  Do  not  set  0  in  positioning  mode, minimum 10 recommend!
#define TZEROWAIT			0x2C	//Defines  the  waiting  time  after  ramping  down
									//to  zero  velocity  before  next  movement  or
									//direction  inversion  can  start.  Time  range  is about 0 to 2 seconds.
#define XTARGET				0x2D	//Target position for ramp mode
#define SW_MODE 			0x34	//Switch mode configuration
#define RAMP_STAT			0x35	//Ramp status and switch event status
#define XLATCH				0x36	//Latches  XACTUAL  upon  a programmable switch event
#define CHOPCONF			0x6C	//Chopper and driver configuration
#define COOLCONF			0x6D	//coolStep smart current control register and stallGuard2 configuration
#define DRV_STATUS 			0x6F	//stallGuard2 value and driver error flags

#define SET_IHOLD(a)		((a & 0x1F)<<0)
#define SET_IRUN(a)			((a & 0x1F)<<8)
#define SET_IHOLDDELAY(a)	((a & 0xF)<<16)


#define READ_ACCESS			0x80	// Read access for spi communication

// Polarity for reference switch
#define REF_SW_HIGH_ACTIV	0x00 	// non-inverted, high active: a high level on REFL stops the motor
#define REF_SW_LOW_ACTIV	0x0C	// inverted, low active: a low level on REFL stops the motor

// Motor direction
#define NORMAL_MOTOR_DIRECTION	0x00	// Normal motor direction
#define INVERSE_MOTOR_DIRECTION	0x10	// Inverse motor direction

// Modes for RAMPMODE register
#define POSITIONING_MODE	0x00		// using all A, D and V parameters)
#define VELOCITY_MODE_POS	0x01		// positiv VMAX, using AMAX acceleration
#define VELOCITY_MODE_NEG	0x02		// negativ VMAX, using AMAX acceleration
#define HOLD_MODE			0x03		// velocity remains unchanged, unless stop event occurs

#define VZERO				0x400		// flag in RAMP_STAT, 1: signals that the actual velocity is 0.


/**
 * @brief Initialize the Trinamic Drivers(TMC5130)
 * @param	csPin				chip select for spi (XAXIS,YAXIS,ZAXIS,E0AXIS)
 * @param	irun				Motor run current (0..31)
 * @param	ihold				Standstill current (0..31)
 * @param	stepper_direction	inverse/ not inverse
 * @return	none
 *****************************************************************************/
void TMC5130_init(uint8_t csPin, uint8_t irun, uint8_t ihold, uint8_t stepper_direction);


/**
 * @brief Activates the driver stage for the given axis
 * @param	axis		axis
 * @return	none
 *****************************************************************************/
void TMC5130_enableDriver(uint8_t axis);


/**
 * @brief Deactivates the driver stage for the given axis
 * @param	axis		axis
 * @return	none
 *****************************************************************************/
void TMC5130_disableDriver(uint8_t axis);


/**
 * @brief Performs a homing for the given axis
 * @param	axis		axis to home
 * @return	none
 *****************************************************************************/
void TMC5130_homing(int axis);



#endif /* TMC5130_H_ */
