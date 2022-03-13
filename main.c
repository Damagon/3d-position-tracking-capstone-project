/*
 * TWI_Code2.c
 *
 * Created: 3/12/2022 7:29:21 PM
 * Author : bcoutts
 */ 

#include <avr/io.h>

#include "twimaster.h"


int main(void)
{
    i2c_init();
	
	
	i2c_start(0x78); //writing
	
	
	i2c_write(0x30);
	i2c_write(0x0A);
	
	i2c_start(0x79); //reading
	uint8_t data = i2c_readNak();
	
}

