// MyAtmelBlink.c

// put LED on PD0

#ifndef F_CPU					
#define F_CPU 1000000UL			// 1 MHz unsigned long
#endif

#include <avr/io.h>				// this is always included in AVR programs
#include <util/delay.h>			// add this to use the delay function

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	
	DDRD |= (1 << PD0);			// set Port D pin PD0 for output
	
	while (1) {					// begin infinite loop
		PORTD ^= (1 << PD0);	// flip state of LED on PD0
		_delay_ms(500);			// delay 1/2 second
	}
	return(0);					// should never get here, this is to prevent a compiler warning
}

