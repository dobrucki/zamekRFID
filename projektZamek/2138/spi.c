#include "spi.h"

void SPI_Master_Init() {
	PINSEL0 = PINSEL0 | 0x00001500
		; // Select P0.4, P0.5, P0.6, P0.7 as SCK0, MISO0, MOSI0 and GPIO
	IOSET |= (1 << 7)
		; // Make ouput High for P0.7
	IODIR |= (1 << 7)
		; // Config P0.7 as Ouput
	SPI_SPCR = 0x0020
		; // SPI Master mode, 8-bit data, SPI0 mode
	SPI_SPCCR = 0x08
		; // Even number, minimum value 8, pre scalar for SPI Clock
	IODIR |= (1 << 4 | 1 << 6)
		; // Set direction register for SCK and MOSI pin.
}


void SPI_Master_Write(tU8 data) {
	tU8 flush;
	SPI_SPDR = data
		; // Load data to be written into the data register
	while ( (SPI_SPSR & 0x80) == 0)
		; // Wait till data transmission is completed
	flush = SPI_SPDR;
}

tU8 SPI_Master_Read() {
	SPI_SPDR = 0xFF
		; // Transmit Flush byte
	while ( (SPI_SPSR & 0x80) == 0)
		; // Wait till data transmission is completed
	return SPI_SPDR
		; // Return the data received
}
