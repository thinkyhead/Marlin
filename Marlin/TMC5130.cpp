/*
 * TMC5130.cpp
 *
 *  Created on: 03.06.2016
 *      Author: tmctest
 */

#include <stdio.h>

// TMC Files
#include "TMC5130.h"
#include "TMC_TRAMS.h"
#include "TMC_SPI.h"
#include "TMC_TRAMS_CONFIGURATION.h"

//
//
///*******************************************************************
//*                      public functions
//*******************************************************************/

/**
 * @brief Initialize the Trinamic Drivers(TMC5130)
 * @param	csPin				chip select for spi (XAXIS,YAXIS,ZAXIS,E0AXIS)
 * @param	irun				Motor run current (0..31)
 * @param	ihold				Standstill current (0..31)
 * @param	stepper_direction	inverse/ not inverse
 * @return	none
 *****************************************************************************/
void TMC5130_init(uint8_t csPin, uint8_t irun, uint8_t ihold, uint8_t stepper_direction)
{
	uint32_t value;
	value = SET_IHOLD(ihold) | SET_IRUN(irun) | SET_IHOLDDELAY(7);
	spi_writeRegister(IHOLD_IRUN, value, csPin);		//IHOLD and IRUN current
	spi_writeRegister(RAMPMODE, 0x0, csPin);			//select position mode
	spi_writeRegister(V_1, 0x0, csPin);					//Disables A1 and D1 in position mode, amax and vmax only
	spi_writeRegister(D_1, 0x10, csPin);				//D1 not zero
	spi_writeRegister(AMAX, 0xFFFF, csPin);				//Acceleration
	spi_writeRegister(VMAX, 0xFFFF, csPin);				//Velocity
	spi_writeRegister(CHOPCONF, 0x140101D5, csPin);		//Chopper Configuration
	spi_writeRegister(GCONF, 0x1084 | stepper_direction, csPin);	//General Configuration

	// initialize enable pin for given axis
	// set as output
	// default disable, low activ
	switch(csPin){
		case XAXIS_CS:	DRV_EN_X_DDR	|= (1<<DRV_EN_X);
						TMC5130_disableDriver(X_AXIS);
						break;
		case YAXIS_CS:	DRV_EN_Y_DDR	|= (1<<DRV_EN_Y);
						TMC5130_disableDriver(Y_AXIS);
						break;
		case ZAXIS_CS:	DRV_EN_Z_DDR	|= (1<<DRV_EN_Z);
						TMC5130_disableDriver(Z_AXIS);
						break;
		case E0AXIS_CS:	DRV_EN_E0_DDR	|= (1<<DRV_EN_E0);
						TMC5130_disableDriver(E_AXIS);
						break;
	}
}


/**
 * @brief Activates the driver stage for the given axis
 * @param	axis		axis
 * @return	none
 *****************************************************************************/
void TMC5130_enableDriver(uint8_t axis){

	// low activ
	switch(axis){
		case X_AXIS:	DRV_EN_X_PORT	&= ~(1<<DRV_EN_X);
						break;
		case Y_AXIS:	DRV_EN_Y_PORT	&= ~(1<<DRV_EN_Y);
						break;
		case Z_AXIS:	DRV_EN_Z_PORT	&= ~(1<<DRV_EN_Z);
						break;
		case E_AXIS:	DRV_EN_E0_PORT	&= ~(1<<DRV_EN_E0);
						break;
		default:
					break;
	}

	return;
}


/**
 * @brief Deactivates the driver stage for the given axis
 * @param	axis		axis
 * @return	none
 *****************************************************************************/
void TMC5130_disableDriver(uint8_t axis){

	// low activ
	switch(axis){
		case X_AXIS:	DRV_EN_X_PORT	|= (1<<DRV_EN_X);
						break;
		case Y_AXIS:	DRV_EN_Y_PORT	|= (1<<DRV_EN_Y);
						break;
		case Z_AXIS:	DRV_EN_Z_PORT	|= (1<<DRV_EN_Z);
						break;
		case E_AXIS:	DRV_EN_E0_PORT	|= (1<<DRV_EN_E0);
						break;
		default:
					break;
	}

	return;
}


/**
 * @brief Performs a homing for the given axis
 * @param	axis		axis to home
 * @return	none
 *****************************************************************************/
void TMC5130_homing(int axis)
{
	unsigned int sw_register;
	uint8_t motor_direction;
	uint32_t stallguardthreshold;
	int homing_retract;
	int homing_speed;
	int axis_to_home;
	bool sg_active = false;
	uint32_t stall_speed;

	float STEPS_PER_UNIT_TMP[]=DEFAULT_AXIS_STEPS_PER_UNIT; // configuraton.h
	float HOMING_FEEDRATE_TMP[]=HOMING_FEEDRATE;			// configuraton.h

	switch(axis)
	{
		case X_AXIS:
			axis_to_home = XAXIS_CS;
			homing_retract = X_HOME_RETRACT_MM * STEPS_PER_UNIT_TMP[0]; // configuraton_adv.h
			homing_speed = HOMING_FEEDRATE_TMP[0];
			sw_register = SWITCH_POSITION_X | SWITCH_POLARITY_X; // TMC_TRAMS_CONFIGURATION

			#ifdef STALLGUARD_X
				sg_active = true;
				stallguardthreshold = STALLGUARDTHRESHOLD_X;
				motor_direction = STEPPER_DIRECTION_X;
			#endif

			break;
		case Y_AXIS:
			axis_to_home = YAXIS_CS;
			homing_retract = Y_HOME_RETRACT_MM * STEPS_PER_UNIT_TMP[1];
			homing_speed = HOMING_FEEDRATE_TMP[1];
			sw_register = SWITCH_POSITION_Y | SWITCH_POLARITY_Y;

			#ifdef STALLGUARD_Y
				sg_active = true;
				stallguardthreshold = STALLGUARDTHRESHOLD_Y;
				motor_direction = STEPPER_DIRECTION_Y;
			#endif

			break;
		case Z_AXIS:
			axis_to_home = ZAXIS_CS;
			homing_retract = Z_HOME_RETRACT_MM * STEPS_PER_UNIT_TMP[2];
			homing_speed = HOMING_FEEDRATE_TMP[2];
			sw_register = SWITCH_POSITION_Z | SWITCH_POLARITY_Z;

			#ifdef STALLGUARD_Z
				sg_active = true;
				stallguardthreshold = STALLGUARDTHRESHOLD_Z;
				motor_direction = STEPPER_DIRECTION_Z;
			#endif

			break;
		default:
			return;
	}


	//Retract axis before homing so it doesn't crash into the printing bed
	if(axis == Z_AXIS)
	{
		spi_writeRegister(RAMPMODE, VELOCITY_MODE_POS, axis_to_home);	//VELOCITY MODE positive Direction
		spi_writeRegister(VMAX, homing_speed, axis_to_home);			//Homing Speed in VMAX
		_delay_ms(3000);
	}

	// Homing Procedure:
	// Enable Trinamic Drivers to start homing movement


	if(sg_active == true)
	{
		spi_writeRegister(SW_MODE, 0x00, axis_to_home);	//SWITCH REGISTER

		stall_speed = 16777216 / homing_speed;
		stall_speed = stall_speed / 16;  // match homing speed to actual microstep speed (at 1/16 microstep)
		stall_speed = stall_speed * 1.10; // Activate stallGuard sligthly below desired homing velocity (provide 10% tolerance)

		spi_writeRegister(GCONF, 0x1080 | motor_direction, axis_to_home);	//stealthchop off for stallguard homing
		spi_writeRegister(COOLCONF, ((stallguardthreshold & 0x7F)<<16),axis_to_home);//sgt <-- Entry the value determined for SGT: lower value=higher sensitivity (lower force for stall detection)
		spi_writeRegister(TCOOLTHRS, stall_speed  ,axis_to_home);//TCOOLTHRS
		spi_writeRegister(SW_MODE, 0x400, axis_to_home);	//SWITCH REGISTER
		spi_writeRegister(AMAX, 100, axis_to_home);	//AMAX for stallGuard homing shall be significantly lower than AMAX for printing

		// Set velocity mode in direction to the endstop
		spi_writeRegister(RAMPMODE, VELOCITY_MODE_NEG, axis_to_home);	//VELOCITY MODE, direction to the endstop
		spi_writeRegister(VMAX, homing_speed, axis_to_home);	//Homing Speed in VMAX

		_delay_ms(20);

		//While motor is still moving (vzero != 1)
		while((spi_readRegister(RAMP_STAT, axis_to_home) & VZERO) != VZERO);

		// Endstop reached. Reset and retract
		spi_writeRegister(RAMPMODE, HOLD_MODE, axis_to_home);		//HOLD Mode
		spi_writeRegister(XACTUAL, 0x0, axis_to_home);				//XACTUAL = 0
		spi_writeRegister(XTARGET, 0x0, axis_to_home);				//XTARGET = 0
		spi_writeRegister(SW_MODE, 0x0, axis_to_home);				//SWITCH REGISTER
		spi_writeRegister(RAMPMODE, POSITIONING_MODE, axis_to_home);//Position MODE
		spi_writeRegister(VMAX, homing_speed, axis_to_home);	//Homing Speed in VMAX
		spi_writeRegister(DMAX, 0xFFFF, axis_to_home);				//DMAX
		spi_writeRegister(XTARGET, homing_retract, axis_to_home);	//XTARGET = homing_retract

		_delay_ms(20);

		//While motor is still moving (vzero != 1)
		while((spi_readRegister(RAMP_STAT, axis_to_home) & VZERO) != VZERO);

		// Endstop reached. Reset and retract
		spi_writeRegister(SW_MODE, 0x0, axis_to_home);	//SWITCH REGISTER
		spi_writeRegister(RAMPMODE, 0x3, axis_to_home);	//HOLD Mode
		spi_writeRegister(GCONF, 0x1080 | motor_direction, axis_to_home);//Turn on stealthchop again
		spi_writeRegister(XACTUAL, 0x0, axis_to_home);	//XACTUAL = 0
		spi_writeRegister(XTARGET, 0x0, axis_to_home);	//XTARGET = 0
		spi_writeRegister(RAMPMODE, 0x0, axis_to_home);	//Position MODE
		_delay_ms(200);
	}
	else
	{
		TMC5130_enableDriver(axis);


		// Set velocity mode in direction to the endstop
		spi_writeRegister(RAMPMODE, VELOCITY_MODE_NEG, axis_to_home);	//VELOCITY MODE negative Direction
		spi_writeRegister(VMAX, homing_speed, axis_to_home);			//Homing Speed in VMAX

		//Config switch register of TMC5130
		spi_writeRegister(SW_MODE, sw_register, axis_to_home);		//SWITCH REGISTER

		//While motor is still moving (vzero != 1)
		while((spi_readRegister(RAMP_STAT, axis_to_home) & VZERO) != VZERO);

		// Endstop reached. Reset and retract
		spi_writeRegister(RAMPMODE, HOLD_MODE, axis_to_home);		//HOLD Mode
		spi_writeRegister(XACTUAL, 0x0, axis_to_home);				//XACTUAL = 0
		spi_writeRegister(XTARGET, 0x0, axis_to_home);				//XTARGET = 0
		spi_writeRegister(SW_MODE, 0x0, axis_to_home);				//SWITCH REGISTER
		spi_writeRegister(RAMPMODE, POSITIONING_MODE, axis_to_home);//Position MODE
		spi_writeRegister(VMAX, homing_speed, axis_to_home);		//Homing Speed in VMAX
		spi_writeRegister(DMAX, 0xFFFF, axis_to_home);				//DMAX
		spi_writeRegister(XTARGET, homing_retract, axis_to_home);	//XTARGET = homing_retract

		_delay_ms(200);

		//While motor is still moving (vzero != 1)
		while((spi_readRegister(RAMP_STAT, axis_to_home) & VZERO) != VZERO);

		//Retract finished
		spi_writeRegister(SW_MODE, sw_register, axis_to_home);		//SWITCH REGISTER
		spi_writeRegister(RAMPMODE, HOLD_MODE, axis_to_home);		//HOLD Mode
		spi_writeRegister(XACTUAL, 0x0, axis_to_home);				//XACTUAL = 0
		spi_writeRegister(XTARGET, 0x0, axis_to_home);				//XTARGET = 0
		spi_writeRegister(RAMPMODE, POSITIONING_MODE, axis_to_home);//Position MODE
	}
}


