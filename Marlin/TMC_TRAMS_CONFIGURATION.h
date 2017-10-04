/*
 * TMC_TRAMS_CONFIGURATION.h
 *
 *  Created on: 27.05.2016
 *      Author: tmctest
 */

#ifndef TMC_TRAMS_CONFIGURATION_H_
#define TMC_TRAMS_CONFIGURATION_H_

#include <stdbool.h>
#include "TMC5130.h"

/*******************************************************************
*                      Current configuration
*******************************************************************/
// only change if necessary
#define X_CURRENT_RUN	25	// Motor run current (0=1/32…31=32/32)
#define X_CURRENT_HOLD	8	// Standstill current (0=1/32…31=32/32)

#define Y_CURRENT_RUN	25	// Motor run current (0=1/32…31=32/32)
#define Y_CURRENT_HOLD	8	// Standstill current (0=1/32…31=32/32)

#define Z_CURRENT_RUN	23	// Motor run current (0=1/32…31=32/32)
#define Z_CURRENT_HOLD	8	// Standstill current (0=1/32…31=32/32)

#define E0_CURRENT_RUN	28	// Motor run current (0=1/32…31=32/32)
#define E0_CURRENT_HOLD	8	// Standstill current (0=1/32…31=32/32)




/*******************************************************************
*                      Reference switch configuration
*******************************************************************/
// x-axis
// switch position, select right or left reference switch (not both)
//#define SWITCH_POSITION_X	0x21		// left
#define SWITCH_POSITION_X	0x11		// right

// switch polarity, select high or low activ (not both)
#define SWITCH_POLARITY_X	REF_SW_LOW_ACTIV		// low activ
//#define SWITCH_POLARITY_X	REF_SW_HIGH_ACTIV		// high activ
//****************************************************************

// y-axis
// switch position, select right or left reference switch (not both)
#define SWITCH_POSITION_Y	0x21		// left
//#define SWITCH_POSITION_Y	0x11		// right

// switch polarity, select high or low activ (not both)
#define SWITCH_POLARITY_Y	REF_SW_LOW_ACTIV		// low activ
//#define SWITCH_POLARITY_Y	REF_SW_HIGH_ACTIV		// high activ
//****************************************************************

// z-axis
// switch position, select right or left reference switch (not both)
#define SWITCH_POSITION_Z	0x21		// left
//#define SWITCH_POSITION_Z	0x11		// right

// switch polarity, select high or low activ (not both)
#define SWITCH_POLARITY_Z	REF_SW_LOW_ACTIV		// low activ
//#define SWITCH_POLARITY_Z	REF_SW_HIGH_ACTIV		// high activ
//****************************************************************



/*******************************************************************
*               Stepper direction
*******************************************************************/
// x-axis
#define STEPPER_DIRECTION_X		INVERSE_MOTOR_DIRECTION
//#define STEPPER_DIRECTION_X		NORMAL_MOTOR_DIRECTION
//****************************************************************

// y-axis
//#define STEPPER_DIRECTION_Y		INVERSE_MOTOR_DIRECTION
#define STEPPER_DIRECTION_Y		NORMAL_MOTOR_DIRECTION
//****************************************************************

// z-axis
//#define STEPPER_DIRECTION_Z		INVERSE_MOTOR_DIRECTION
#define STEPPER_DIRECTION_Z		NORMAL_MOTOR_DIRECTION
//****************************************************************

// e0-axis
//#define STEPPER_DIRECTION_E0	INVERSE_MOTOR_DIRECTION
#define STEPPER_DIRECTION_E0	NORMAL_MOTOR_DIRECTION



/*******************************************************************
*               STALLGUARD
*******************************************************************/
// x-axis
#define STALLGUARD_X	// if selected, stallguard is active
#define STALLGUARDTHRESHOLD_X	0x08// range 0x00..0x7F

// y-axis
#define STALLGUARD_Y	// if selected, stallguard is active
#define STALLGUARDTHRESHOLD_Y	0x08// range 0x00..0x7F

// z-axis
//#define STALLGUARD_Z	// if selected, stallguard is active
#define STALLGUARDTHRESHOLD_Z	0x08// range 0x00..0x7F



#endif /* TMC_TRAMS_CONFIGURATION_H_ */
