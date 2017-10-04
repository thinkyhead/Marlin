#include "TMC_SPI.h"
#include "TMC_TRAMS.h"
#include "TMC5130.h"



/*******************************************************************
 *                      private functions
 *******************************************************************/

/**
 * @brief Send a byte via SPI and read the received
 * @param	data		to transmitted byte
 * @return	received byte
 *****************************************************************************/
 uint8_t spi_readWriteByte(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF))); // polling the SPI Interrupt Flag
	return SPDR;	// return the received byte
}


 /**
  * @brief Send a byte via SPI
  * @param		data		to be transmitted byte
  *****************************************************************************/
  void spi_writeByte(uint8_t data)
 {
 	SPDR = data;
 	while(!(SPSR & (1<<SPIF)));
 }



/*******************************************************************
*                      public functions
*******************************************************************/

/**
 * @brief Initialize the SPI
 * SPI Master
 * 4Mhz(CPU_CLOCK / 4)
 * CPOL = 0
 * CPHA = 0
 * no interrupt
 *****************************************************************************/
void spi_init(void)
{
	//Initialize the SPI interface
	//outputs
	DDR_SPI |= ((1<<SPI_MOSI) | (1<<SPI_SCK) | (1<<SPI_CS));
	//inputs
	DDR_SPI &= ~(1<<SPI_MISO);

	//Initialize chip select pins
	SPI_CS_DDR	|= (1<<XAXIS_CS);
	SPI_CS_DDR	|= (1<<YAXIS_CS);
	SPI_CS_DDR	|= (1<<ZAXIS_CS);
	SPI_CS_DDR	|= (1<<E0AXIS_CS);

	//all cs high
	SPI_CS_PORT	|= (1<<XAXIS_CS);
	SPI_CS_PORT	|= (1<<YAXIS_CS);
	SPI_CS_PORT	|= (1<<ZAXIS_CS);
	SPI_CS_PORT	|= (1<<E0AXIS_CS);


	SPCR = ((1<<SPE)|               // SPI Enable
			(0<<SPIE)|              // SPI Interupt Enable
			(0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
			(1<<MSTR)|              // Master/Slave select
			(0<<SPR1)|(0<<SPR0)|    // SPI Clock Rate(fcpu/4 = 4Mhz)
			(0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
			(0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)
}


/**
 * @brief Reads four byte via SPI
 * @param		address		register address
 * @param		csPin		chip select
 * @return		status
 *****************************************************************************/
uint32_t spi_readRegister(uint8_t address, uint8_t slave)
 {
		uint8_t buf[4];
		uint32_t register_value = 0;

		SPI_CS_PORT &= ~(1 << slave); 	// enable slave, low activ

		// first read cycle to address the register
		spi_readWriteByte(address);
		spi_readWriteByte(0x00);
		spi_readWriteByte(0x00);
		spi_readWriteByte(0x00);
		spi_readWriteByte(0x00);

		PORTL |= (1 << slave);			// disable slave, low activ

		nop();
		nop();
		nop();

		SPI_CS_PORT &= ~(1 << slave); 	// select slave, low activ

		// second read cycle to get the register value
		spi_readWriteByte(address);
		buf[3] = spi_readWriteByte(0x00);
		buf[2] = spi_readWriteByte(0x00);
		buf[1] = spi_readWriteByte(0x00);
		buf[0] = spi_readWriteByte(0x00);

		SPI_CS_PORT |= (1 << slave);	// disable slave, low activ


		register_value |= buf[3];
		register_value = register_value << 8;
		register_value |= buf[2];
		register_value = register_value << 8;
		register_value |= buf[1];
		register_value = register_value << 8;
		register_value |= buf[0];

		return register_value;
 }



/**
 * @brief Send five byte via SPI and received the status
 * @param		address		register address
 * @param		data		to transmitted data
 * @param		csPin		chip select
 * @return	status
 *****************************************************************************/
uint8_t spi_writeRegister(uint8_t address, uint32_t data, uint8_t slave)
{
	uint8_t buf[4];
	uint8_t status = 0;

	buf[0] = data & 0xFF;
	buf[1] = (data & 0xFF00) >> 8;
	buf[2] = (data & 0xFF0000) >> 16;
	buf[3] = (data & 0xFF000000) >> 24;

	SPI_CS_PORT &= ~(1 << slave); 	// enable slave, low activ

	// address register
	spi_writeByte(address | READ_ACCESS);
	// send new register value
	spi_writeByte(buf[3]);
	spi_writeByte(buf[2]);
	spi_writeByte(buf[1]);
	spi_writeByte(buf[0]);

	SPI_CS_PORT |= (1 << slave);	// disable slave, low activ

	return status;
 }


/**
 * @brief Reads the status from the MAMC
 * @param		csPin		chip select
 * @return	status
 *****************************************************************************/
uint8_t spi_readStatus(uint8_t slave)
{
	uint8_t status;

	SPI_CS_PORT &= ~(1 << slave); 	// enable slave, low activ

 	// send adress and read the status from FPGA
 	status = spi_readWriteByte(GCONF);	// addressing any register int the tmc5130
 	// send data, msb first
 	spi_readWriteByte(0x00);
 	spi_readWriteByte(0x00);
 	spi_readWriteByte(0x00);
 	spi_readWriteByte(0x00);

 	SPI_CS_PORT |= (1 << slave);	// disable slave, low activ

	return status;
}


