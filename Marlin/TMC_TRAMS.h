/*
 * TMC_TRAMS.h
 *
 *  Created on: 27.05.2016
 *      Author: tmctest
 */

#ifndef TMC_TRAMS_H_
#define TMC_TRAMS_H_

#include <avr/io.h>

// SPI communication
#define SPI_MISO	PB3		// master in slave out
#define SPI_MOSI	PB2		// master out slave in
#define SPI_SCK		PB1		// serial clock
#define SPI_CS		PB0		// chip select not use, only use for spi initialize
#define PORT_SPI    PORTB	// port-register for spi interface
#define DDR_SPI     DDRB	// data-direction-register for spi interface

#define XAXIS_CS		PL3		// chip select x
#define YAXIS_CS		PL0		// chip select y
#define ZAXIS_CS		PL1		// chip select z
#define E0AXIS_CS		PL2		// chip select e0
#define SPI_CS_PORT		PORTL	// Port for chip select signals
#define SPI_CS_DDR		DDRL	// DDR for chip select signals


// TMC5130 driver enable
// x-axis
#define	DRV_EN_X		PD7		// driver enable x
#define DRV_EN_X_PORT	PORTD	// port-register for driver enable x
#define DRV_EN_X_DDR	DDRD	// data-direction-register for driver enable x

// y-axis
#define	DRV_EN_Y		PK0		// driver enable y
#define DRV_EN_Y_PORT	PORTK	// port-register for driver enable y
#define DRV_EN_Y_DDR	DDRK	// data-direction-register for driver enable y

// z-axis
#define	DRV_EN_Z		PF2		// driver enable z
#define DRV_EN_Z_PORT	PORTF	// port-register for driver enable z
#define DRV_EN_Z_DDR	DDRF	// data-direction-register for driver enable z

// eo-axis
#define	DRV_EN_E0		PA2		// driver enable eo
#define DRV_EN_E0_PORT	PORTA	// port-register for driver enable e0
#define DRV_EN_E0_DDR	DDRA	// data-direction-register for driver enable e0




#endif /* TMC_TRAMS_H_ */
