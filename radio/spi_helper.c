/*
 *  ======= spi_helper.c =======
 *
 *	Includes the low-level helper functions for the CC1101 library that allow
 *	the library to communicate with the CC1101 via SPI.
 *
 *	Author: Tyler Horton (tch0014@auburn.edu)
 *  Created on: 6/27/16
 *  Last modified: 8/16/16
 */


#include <radio/radio_driver.h>

#define SPI_CSN_PORT	GPIO_PORTP_BASE
#define SPI_CSN_PIN     GPIO_PIN_5
#define SPI_MISO_PORT   GPIO_PORTQ_BASE
#define SPI_MISO_PIN	GPIO_PIN_3


/*
 *  ======= SpiInit =======
 *
 *  Initializes the SPI interface that communicates with the CC1101. Also, the
 *  pin used for the CC1101's CSN line is initialized and pulled high (CSN is
 *  active low).
 *
 *  Params: none
 *  Return: none
 */
void SpiInit(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
	GPIOPinTypeGPIOOutput(SPI_CSN_PORT, SPI_CSN_PIN);
	GPIOPinWrite(SPI_CSN_PORT, SPI_CSN_PIN, 0xFF);
	Board_initSPI();	// This function has been modified for the needed configuration
}

/*
 *  ======= SpiWrite =======
 *
 *  Writes data to the CC1101 through the SPI interface using TI's SSI module.
 *
 *  Params:
 *  	unsigned char - address to write to in the CC1101
 *  	const unsigned char - pointer to a buffer with data to write
 *  	unsigned char - number of bytes to write (should correspond with buffer
 *  					size)
 *  Return: none
 */
void SpiWrite(unsigned char address, const unsigned char *pBuffer, unsigned char count)
{
	uint8_t i;
	uint32_t dummy;

	/* All transactions start with a header byte containing a R/W bit, a burst access bit,
	   and a 6-bit address */

	GPIOPinWrite(SPI_CSN_PORT, SPI_CSN_PIN, 0x00);			// Pull CSn low
	while(GPIOPinRead(SPI_MISO_PORT, SPI_MISO_PIN));		// Wait until the MISO pin goes low

	while(SSIBusy(SSI3_BASE));								// Wait for SSI to not be busy
	SSIDataPut(SSI3_BASE, (uint32_t)address);				// Write header for writing data to MOSI Pin
	SSIDataGet(SSI3_BASE, &dummy);							// Read dummy values

	// Write data
	for(i=0; i<count; i++) {
		while(SSIBusy(SSI3_BASE));							// Wait for SSI to not be busy
		SSIDataPut(SSI3_BASE, (uint32_t)(*(pBuffer+i)));	// Write data to MOSI Pin
		SSIDataGet(SSI3_BASE, &dummy);						// Read dummy value
	}

	while(SSIBusy(SSI3_BASE));
	GPIOPinWrite(SPI_CSN_PORT, SPI_CSN_PIN, 0xFF);			// Release CSn back to high
}

/*
 *  ======= SpiRead =======
 *
 *  Reads data to the CC1101 through the SPI interface using TI's SSI module.
 *
 *  Params:
 *  	unsigned char - address to read from in the CC1101
 *  	const unsigned char - pointer to a buffer where data can be read to
 *  	unsigned char - number of bytes to read (should correspond with buffer
 *  					size)
 *  Return: none
 */
void SpiRead(unsigned char address, unsigned char *pBuffer, unsigned char count)
{
	uint8_t i;
	uint32_t dummy;
	uint32_t rxdata;

	GPIOPinWrite(SPI_CSN_PORT, SPI_CSN_PIN, 0x00);		// Pull CSn low
	while(GPIOPinRead(SPI_MISO_PORT, SPI_MISO_PIN));	// Wait until the MISO pin goes low

	while(SSIBusy(SSI3_BASE));							// Wait for SSI to not be busy
	SSIDataPut(SSI3_BASE, address);						// Write header for reading data from MISO Pin
	SSIDataGet(SSI3_BASE, &dummy);						// Read dummy value

	for(i=0; i<count; i++) {
		while(SSIBusy(SSI3_BASE));						// Wait for SSI to be ready
		SSIDataPut(SSI3_BASE, 0x00);					// Write dummy value
		SSIDataGet(SSI3_BASE, &rxdata);					// Read value from MISO pin
		*(pBuffer+i) = (uint8_t)(rxdata & 0x00FF);		// Mask unnecessary bits
	}

	while(SSIBusy(SSI3_BASE));
	GPIOPinWrite(SPI_CSN_PORT, SPI_CSN_PIN, 0xFF);		// Release CSn back to high
}
