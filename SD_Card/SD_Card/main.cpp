#include <avr/io.h>
#include "timeout.h"
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "sdcard/ff.h"
#include "sdcard/integer.h"

FATFS FatFs;	// FatFs work area 
FIL *fp;		// fpe object 


/*---------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */
/* Used to provide a Timestamp for SDCard files and folders*/
DWORD get_fattime (void)
{
	// Returns current time packed into a DWORD variable 
	return	  ((DWORD)(2013 - 1980) << 25)	// Year 2013 
	| ((DWORD)8 << 21)				// Month 7 
	| ((DWORD)2 << 16)				// Day 28 
	| ((DWORD)20 << 11)				// Hour 0..24
	| ((DWORD)30 << 5)				// Min 0 
	| ((DWORD)0 >> 1);				// Sec/2 0
}



int main(void)
{
	DDRD |= (1<<PD0); // Set LED pin output
	DDRD |= (1<<PD1); // Set LED pin output
	PORTD &= ~(1<<PD0); // Turn LED off
	PORTD &= ~(1<<PD1); // Turn LED off
	
	/*// reboot delay
	_delay_ms(200);
	
	// init sdcard
	UINT bw;
	f_mount(0, &FatFs);		// Give a work area to the FatFs module 
	// open file
	fp = (FIL *)malloc(sizeof (FIL));
	if (f_open(fp, "file.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	// Create a file // THREEDO: Change from text
		char *text = "Hello World! SDCard support up and running!\r\n";
		f_write(fp, text, strlen(text), &bw);	// Write data to the file 
		f_close(fp); // Close the file 
		if (bw == strlen(text)) { // We wrote the entire string
			PORTD |= (1<<PD0); // Turn LED on
		}
		//else led2.Set(1); //TODO: Fix
	}
	// test append
	if (f_open(fp, "file.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {	// Open existing or create new file
		if (f_lseek(fp, f_size(fp)) == FR_OK) 
			{
			char *text2 = "This is a new line, appended to existing file!\r\n";
			f_write(fp, text2, strlen(text2), &bw);	// Write data to the file
			if (bw == strlen(text2)) { //we wrote the entire string
				PORTD |= (1<<PD1); // Turn LED on
			}
			
		}	
		f_close(fp);// Close the file		
	}	*/
	//------------------------------------------
	// Image test
	
	// reboot delay
	_delay_ms(200);
	
	// init sdcard
	UINT bw;
	f_mount(0, &FatFs);		// Give a work area to the FatFs module
	// open file
	fp = (FIL *)malloc(sizeof (FIL));
	if (f_open(fp, "image.jpg", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	// Create a file // THREEDO: Change from text
		f_write(fp, "test.jpg", sizeof("test.jpg"), &bw);	// Write data to the file
		f_close(fp); // Close the file
		if (bw ==  sizeof("test.jpg")) { // We wrote the entire string
			PORTD |= (1<<PD0); // Turn LED on
		}
		//else led2.Set(1); //TODO: Fix
	}
	
	
	
	//-----------------------------------------------
	 // char str[12];

	/*
	// get card volume
	char szCardLabel[12] = {0};
	DWORD sn = 0;
	if (f_getlabel("", szCardLabel, &sn) == FR_OK) {
		lcd.send_format_string("%s SN:%X\n", szCardLabel, sn);
	}			
	*/

	/*
	// read from file
	if (f_open(fp, "file.txt", FA_READ ) == FR_OK) {	// Create a file
		char text[255]; 
		UINT br;
		f_read(fp, text, 255, &br); 
		f_close(fp);// Close the file
		// cut text the easy way
		text[10] = 0;
		lcd.send_format_string("Read:%s", text);
	}	
	*/
	
	// loop
	while(1)
	{
	}
}