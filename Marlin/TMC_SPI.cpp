#include "TMC_SPI.h"
#include "TMC_TRAMS.h"
#include "TMC5130.h"

#include "macros.h"

/*******************************************************************
 *                      private functions
 *******************************************************************/

/**
 * @brief Send a byte via SPI and read the received
 * @param data    to transmitted byte
 * @return  received byte
 *****************************************************************************/
uint8_t spi_readWriteByte(uint8_t data) {
  SPDR = data;
  while(!TEST(SPSR, SPIF)); // polling the SPI Interrupt Flag
  return SPDR;  // return the received byte
}


/**
 * @brief Send a byte via SPI
 * @param    data    to be transmitted byte
 *****************************************************************************/
void spi_writeByte(uint8_t data) {
  SPDR = data;
  while(!TEST(SPSR, SPIF));
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
void spi_init(void) {
  //Initialize the SPI interface
  //outputs
  DDR_SPI |= _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_CS);
  //inputs
  CBI(DDR_SPI, SPI_MISO);

  // Init chip select pins
  SBI(SPI_CS_DDR, XAXIS_CS);
  SBI(SPI_CS_DDR, YAXIS_CS);
  SBI(SPI_CS_DDR, ZAXIS_CS);
  SBI(SPI_CS_DDR, E0AXIS_CS);

  // All cs high
  SBI(SPI_CS_PORT, XAXIS_CS);
  SBI(SPI_CS_PORT, YAXIS_CS);
  SBI(SPI_CS_PORT, ZAXIS_CS);
  SBI(SPI_CS_PORT, E0AXIS_CS);

  SPCR = (_BV(SPE)            // SPI Enable
       | (0<<SPIE)            // SPI Interupt Enable
       | (0<<DORD)            // Data Order (0:MSB first / 1:LSB first)
       | (1<<MSTR)            // Master/Slave select
       | (0<<SPR1)|(0<<SPR0)  // SPI Clock Rate(fcpu/4 = 4Mhz)
       | (0<<CPOL)            // Clock Polarity (0:SCK low / 1:SCK hi when idle)
       | (0<<CPHA));          // Clock Phase (0:leading / 1:trailing edge sampling)
}

/**
 * @brief Reads four byte via SPI
 * @param   address   register address
 * @param   csPin   chip select
 * @return    status
 *****************************************************************************/
uint32_t spi_readRegister(uint8_t address, uint8_t slave) {
    uint8_t buf[4];

    CBI(SPI_CS_PORT, slave);   // enable slave, active low

    // first read cycle to address the register
    spi_readWriteByte(address);
    spi_readWriteByte(0x00);
    spi_readWriteByte(0x00);
    spi_readWriteByte(0x00);
    spi_readWriteByte(0x00);

    SBI(PORTL, slave);      // disable slave, active low

    nop();
    nop();
    nop();

    CBI(SPI_CS_PORT, slave);   // select slave, active low

    // second read cycle to get the register value
    spi_readWriteByte(address);
    buf[3] = spi_readWriteByte(0x00);
    buf[2] = spi_readWriteByte(0x00);
    buf[1] = spi_readWriteByte(0x00);
    buf[0] = spi_readWriteByte(0x00);

    SBI(SPI_CS_PORT, slave);  // disable slave, active low

    uint32_t register_value = (buf[3] << 8) | buf[2];
    register_value <<= 8; register_value |= buf[1];
    register_value <<= 8; register_value |= buf[0];

    return register_value;
 }

/**
 * @brief Send five byte via SPI and received the status
 * @param   address   register address
 * @param   data    to transmitted data
 * @param   csPin   chip select
 * @return  status
 *****************************************************************************/
uint8_t spi_writeRegister(uint8_t address, uint32_t data, uint8_t slave) {
  uint8_t status = 0;
  uint8_t buf[4] = {
     data        & 0xFF,
    (data >>  8) & 0xFF,
    (data >> 16) & 0xFF,
    (data >> 24) & 0xFF
  };

  CBI(SPI_CS_PORT, slave);   // enable slave, active low

  // address register
  spi_writeByte(address | READ_ACCESS);
  // send new register value
  spi_writeByte(buf[3]);
  spi_writeByte(buf[2]);
  spi_writeByte(buf[1]);
  spi_writeByte(buf[0]);

  SBI(SPI_CS_PORT, slave);  // disable slave, active low

  return status;
 }

/**
 * @brief Reads the status from the MAMC
 * @param   csPin   chip select
 * @return  status
 *****************************************************************************/
uint8_t spi_readStatus(const uint8_t slave) {
  uint8_t status;

  CBI(SPI_CS_PORT, slave);   // enable slave, active low

  // send adress and read the status from FPGA
  status = spi_readWriteByte(GCONF);  // addressing any register int the tmc5130
  // send data, msb first
  spi_readWriteByte(0x00);
  spi_readWriteByte(0x00);
  spi_readWriteByte(0x00);
  spi_readWriteByte(0x00);

  SBI(SPI_CS_PORT, slave);  // disable slave, active low

  return status;
}
