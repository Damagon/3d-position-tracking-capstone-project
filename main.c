/*
 * TWI_Communication.c
 *
 * Created: 3/13/22 2:34:06 PM
 * Author : Ayaan Ahmed
 */ 



#include <stdint.h>
#include <avr/io.h>
#include <util/twi.h>>
#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 1000000UL //changes from 4000000UL
#endif



#define SPI_ACTIVE			0 // SS Pin put Low
#define SPI_INACTIVE		1 // SS Pin put High

/*SPI CLOCK*/
#define SPI_CLOCK_DIV4		0x00
#define SPI_CLOCK_DIV16		0x01
#define SPI_CLOCK_DIV64		0x02
#define SPI_CLOCK_DIV128	0x03
#define SPI_CLOCK_DIV2		0x04
#define SPI_CLOCK_DIV8		0x05
#define SPI_CLOCK_DIV32		0x06


#define CS_CAM_1        2        //ArduCam 1
#define SPI_PIN_SS		4		//SS PIN
#define SPI_PIN_MOSI	5		//MOSI PIN
#define SPI_PIN_MISO	6		//MISO PIN
#define SPI_PIN_SCK		7		//SCK PIN
#define SPI_DDR			DDRB	//SPI on PORTB
#define SPI_PORT		PORTB	//SPI on PORTB



/* I2C clock in Hz */
#define SCL_CLOCK  100000L

#define CS_CAM_1        2        //ArduCam 1

void spi_master_init(uint8_t mode, uint8_t clock){
	
	// Pin Configuration
	SPI_DDR |= (1<<SPI_PIN_SS);
	SPI_PORT|= (1<<SPI_PIN_SS);
	//set PB2 to output and set it high
	SPI_DDR |= (1<<CS_CAM_1);
	SPI_PORT|= (1<<CS_CAM_1);
	
	static volatile uint8_t SPI_CTS	 = SPI_INACTIVE;
	// Set MOSI and SCK output, all others input
	SPI_DDR |= (1<<SPI_PIN_MOSI)|(1<<SPI_PIN_SCK);
	// Enable SPI, Master, set clock rate
	SPCR = (1<<SPIE)|(1<<SPE)|(1<<MSTR)|(mode<<CPHA)|(clock<<SPR0);

}
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

void camInit (void){
	writeSPI(0x86,0x02); // Power on the sensor
	writeSPI(0x81,0x01); //Set 1 frame to be captured in CCR
	readSPI(0x01);
	writeSPI(0x83,0x02); //guessed
	//readSPI(0x86);
	//readSPI(0x45);
}

void startCapture(void) {
	
	writeSPI(0x84, 0b00110011); //start capture
	
	uint8_t data  = 0x00;
	
	while (!(data & (1<<3))) {
		data = readSPI(0x41); //check write FIFO done flag
	}
}


uint32_t readSize(void){
	uint32_t low = readSPI(0x42);
	uint32_t mid =  readSPI(0x43);
	uint32_t high = readSPI(0x44);
	
	uint32_t size = low | (mid<<8) | (high<<16);
	
	return size;
}

void i2c_init(void){
	/* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
	
	TWSR = 0;                         /* no prescaler */
	TWBR = ((4*F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */
	
}


unsigned char i2c_start(unsigned char address){
	uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

	}
	/* i2c_start */

void i2c_stop(void){
	/* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

	}
	/* i2c_stop */


unsigned char i2c_write( unsigned char data ){
	uint8_t   twst;
	
	// send data to the previously addressed device
	TWDR = data; //for write only
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}
/* i2c_write */
	

unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
	return TWDR;

}
/* i2c_readNak */

void sensor_set(unsigned char reg1, unsigned char reg2, unsigned char address)
{
	i2c_start(0x78);
	
	i2c_write(reg1);
	i2c_write(reg2); //write to register 3103
	
	i2c_write(address); //write 0x03 to register 3103

	
	
	
	
}

void reg_set (void)
{
	
	sensor_set(0x31,0x03, 0x03);
	sensor_set(0x30,0x08, 0x82);
	sensor_set(0x30,0x17,0x7f);
	sensor_set(0x30,0x18,0xfc);
	sensor_set(0x38,0x10,0xc2);
	sensor_set(0x36,0x15,0xf0);
	sensor_set(0x30,0x00,0x00);
	sensor_set(0x30,0x01,0x00);
	sensor_set(0x30, 0x02,0x00);
	sensor_set(0x30, 0x03,0x00);
	sensor_set(0x30, 0x11,0x08);
	sensor_set(0x30, 0x10,0x30);
	sensor_set(0x36, 0x04,0x60);
	sensor_set(0x36, 0x22,0x08);
	sensor_set(0x36, 0x21,0x17);
	sensor_set(0x37, 0x09,0x00);
	sensor_set(0x40, 0x00,0x21);
	sensor_set(0x40, 0x1d,0x02);
	sensor_set(0x36, 0x00,0x54);
	sensor_set(0x36, 0x05,0x04);
	sensor_set(0x36, 0x06,0x3f);
	sensor_set(0x3c, 0x01,0x80);
	sensor_set(0x30, 0x0d,0x21);
	sensor_set(0x36, 0x23,0x22);
	sensor_set(0x50, 0x00,0xcf);
	sensor_set(0x50, 0x20,0x04);
	sensor_set(0x51, 0x81,0x79);
	sensor_set(0x51, 0x82,0x00);
	sensor_set(0x51, 0x85,0x22);
	sensor_set(0x51, 0x97,0x01);
	sensor_set(0x55, 0x00,0x0a);
	sensor_set(0x55, 0x04,0x00);
	sensor_set(0x55, 0x05,0x7f);
	sensor_set(0x50, 0x80,0x08);
	sensor_set(0x30, 0x0e,0x18);
	sensor_set(0x46, 0x10,0x00);
	sensor_set(0x47, 0x1d,0x05);
	sensor_set(0x47, 0x08,0x06);
	sensor_set(0x37, 0x10,0x10);
	sensor_set(0x37, 0x0d,0x06);
	sensor_set(0x36, 0x32,0x41);
	sensor_set(0x37, 0x02,0x40);
	sensor_set(0x36, 0x20,0x37);
	sensor_set(0x36, 0x31,0x01);
	sensor_set(0x37, 0x0c,0xa0);
	sensor_set(0x38, 0x08,0x05);
	sensor_set(0x38, 0x09,0x00);
	sensor_set(0x38, 0x0a,0x03);
	sensor_set(0x38, 0x0b,0xc0);
	sensor_set(0x38, 0x0c,0x0c);
	sensor_set(0x38, 0x0d,0x80);
	sensor_set(0x38, 0x0e,0x07);
	sensor_set(0x38, 0x0f,0xd0);
	sensor_set(0x50, 0x00,0x06);
	sensor_set(0x50, 0x1f,0x03);
	sensor_set(0x35, 0x03,0x07);
	sensor_set(0x35, 0x01,0x73);
	sensor_set(0x35, 0x02,0x80);
	sensor_set(0x35, 0x0b,0x00);
	sensor_set(0x38, 0x18,0xc0);
	sensor_set(0x36, 0x21,0x27);
	sensor_set(0x38, 0x01,0x8a);
	sensor_set(0x3a, 0x00,0x78);
	sensor_set(0x3a, 0x1a,0x04);
	sensor_set(0x3a, 0x13,0x30);
	sensor_set(0x3a, 0x18,0x00);
	sensor_set(0x3a, 0x19,0x7c);
	sensor_set(0x3a, 0x08,0x12);
	sensor_set(0x3a, 0x09,0xc0);
	sensor_set(0x3a, 0x0a,0x0f);
	sensor_set(0x3a, 0x0b,0xa0);
	sensor_set(0x30, 0x04,0xff);
	sensor_set(0x35, 0x0c,0x07);
	sensor_set(0x35, 0x0d,0xd0);
	sensor_set(0x3a, 0x0d,0x08);
	sensor_set(0x3a, 0x0e,0x06);
	sensor_set(0x35, 0x00,0x00);
	sensor_set(0x35, 0x01,0x00);
	sensor_set(0x35, 0x02,0x00);
	sensor_set(0x35, 0x0a,0x00);
	sensor_set(0x35, 0x0b,0x00);
	sensor_set(0x35, 0x03,0x00);
	sensor_set(0x30, 0x30,0x2b);
	sensor_set(0x3a, 0x02,0x00);
	sensor_set(0x3a, 0x03,0x7d);
	sensor_set(0x3a, 0x04,0x00);
	sensor_set(0x3a, 0x14,0x00);
	sensor_set(0x3a, 0x15,0x7d);
	sensor_set(0x3a, 0x16,0x00);
	sensor_set(0x3a, 0x00,0x78);
	sensor_set(0x3a, 0x08,0x09);
	sensor_set(0x3a, 0x09,0x60);
	sensor_set(0x3a, 0x0a,0x07);
	sensor_set(0x3a, 0x0b,0xd0);
	sensor_set(0x3a, 0x0d,0x10);
	sensor_set(0x3a, 0x0e,0x0d);
	sensor_set(0x36, 0x20,0x57);
	sensor_set(0x37, 0x03,0x98);
	sensor_set(0x37, 0x04,0x1c);
	sensor_set(0x58, 0x9b,0x00);
	sensor_set(0x58, 0x9a,0xc0);
	sensor_set(0x36, 0x33,0x07);
	sensor_set(0x37, 0x02,0x10);
	sensor_set(0x37, 0x03,0xb2);
	sensor_set(0x37, 0x04,0x18);
	sensor_set(0x37, 0x0b,0x40);
	sensor_set(0x37, 0x0d,0x02);
	sensor_set(0x36, 0x20,0x52);
	sensor_set(0x50, 0x00,0x06);
	sensor_set(0x50, 0x01,0x01);
	sensor_set(0x50, 0x05,0x00);
	sensor_set(0x38, 0x18,0x80);
	sensor_set(0x36, 0x21,0x17);
	sensor_set(0x38, 0x01,0xb4);
	sensor_set(0xff, 0xff,0xff);
}

int main(void)
{
	
	i2c_init();
	
	sensor_set(0x30,0x08, 0x03);
	reg_set();
	
	
	
	/*i2c_start(0x78); //writing
	
	
	i2c_write(0x31);
	i2c_write(0x03);
	
	i2c_start(0x79); //reading
	uint8_t data = i2c_readNak(); 
	//THIS CODE READS REGISTER - PUT INTO FCN LATER*/
	
	
	
	DDRD |= (1 << PD0); DDRD |= (1 << PD1);
	
	PORTD &= ~(1 << PD0);
	
	spi_master_init(0,SPI_CLOCK_DIV2);
	// Load data into the buffer
	
	writeSPI(0x80, 0x97);
	
	uint8_t data = readSPI(0x00);
	
	if (data==0x97) {
		PORTD|= (1 << PD0);
	}
	
	//_delay_ms(2000);
	PORTD &= ~(1 << PD0);
	
	camInit();
	startCapture();
	writeSPI(0x84,0x01);
	//PORTD|= (1 << PD1);
	uint32_t size = readSize();
}

