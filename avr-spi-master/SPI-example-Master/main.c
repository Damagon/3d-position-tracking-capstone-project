/************************************************************************
Title:    Example SPI-master
Brief:	  This example shows how to receive and send data with SPI
Author:   Julien Delvaux
Software: Atmel Studio 7
Hardware: AVR 8-Bits, tested with ATmega1284P and ATmega88PA-PU
License:  GNU General Public License 3


LICENSE:
	Copyright (C) 2015 Julien Delvaux

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
    
************************************************************************/
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CS_CAM_1        2        //ArduCam 1
#include "SPI/SPI.h"


uint8_t readSPI(uint8_t addr) {
	
	// Send the register address
	PORTB&= ~(1<<CS_CAM_1);
	SPDR = addr;

	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));

	// Receive the value from the ArduCAM
	SPDR = 0x00; //Command

	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));
	uint8_t data = SPDR;

	PORTB|= (1<<CS_CAM_1);
	
	return data;
}

void writeSPI(uint8_t addr, uint8_t data) {
	
	PORTB&= ~(1<<CS_CAM_1);
	SPDR = addr;

	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));

	SPDR = data;

	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));

	PORTB|= (1<<CS_CAM_1);
	
}

int main(void)
{
	DDRD |= (1 << PD0);
	
	PORTD &= ~(1 << PD0);
	
    spi_master_init(0,SPI_CLOCK_DIV2);
	// Load data into the buffer
	
	writeSPI(0x84, 0x97);
	
	
	uint8_t data = readSPI(0x00);
	
	if (data==0x97) {
		PORTD|= (1 << PD0);
	}
}
 
   

