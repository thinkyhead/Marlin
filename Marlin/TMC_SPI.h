#ifndef TMC_SPI_H
#define TMC_SPI_H

//#include "Marlin.h"
#include <stdio.h>
#define nop() asm volatile("nop")


/**
 * @brief Initialize the SPI
 * SPI Master
 * 4Mhz(CPU_CLOCK / 4)
 * CPOL = 0
 * CPHA = 0
 * no interrupt
 *****************************************************************************/
void spi_init(void);


/**
 * @brief Reads four byte via SPI
 * @param		address		register address
 * @param		csPin		chip select
 * @return		status
 *****************************************************************************/
uint32_t spi_readRegister(uint8_t address, uint8_t slave);


/**
 * @brief Send five byte via SPI and received the status
 * @param		address		register address
 * @param		data		to transmitted data
 * @param		csPin		chip select
 * @return	status
 *****************************************************************************/
uint8_t spi_writeRegister(uint8_t address, uint32_t data, uint8_t slave);


/**
 * @brief Reads the status from the MAMC
 * @param		csPin		chip select
 * @return	status
 *****************************************************************************/
uint8_t spi_readStatus(uint8_t slave);



#endif //TMC_SPI_H
