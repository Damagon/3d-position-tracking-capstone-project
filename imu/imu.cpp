#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 1000000UL //changes from 4000000UL
#endif

/* I2C clock in Hz */
#define SCL_CLOCK  100000L

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

void i2c_stop(void){
	/* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));
}

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

unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
	return TWDR;
}

unsigned char i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    

    return TWDR;
}

void imu_write(unsigned char addr, unsigned char data)
{
	i2c_start(0xD0); //b110100 0 0 (write)
	i2c_write(addr);
	i2c_readAck();
	i2c_write(data);
	i2c_readAck();
	i2c_stop();
}

unsigned char imu_read(unsigned char addr) // TODO: Finish
{
	i2c_start(0xD0); //b110100 0 0 (write)
	i2c_write(addr);
	i2c_readAck();
	i2c_start(0xD1); //b110100 0 1 (read)
	unsigned char data = i2c_readAck(); //read data ?
	//write nack ??
	i2c_stop();
	return data;
}

int main(void) {
	DDRD |= (1 << PD0); DDRD |= (1 << PD1);
	
	PORTD &= ~(1 << PD0);
	PORTD &= ~(1 << PD1);
	
	i2c_init();
	
	// Initialize:
	// Reset registers (PWR_MGMT_1.DEVICE_RESET)
	imu_write(0x68, 0b10000000);
	// Delay for 1 ms
	_delay_ms(1);
	// Reset signal paths (USER_CTRL.SIG_COND_RST)
	imu_write(0x6A, 0b00000001);
	// Delay for 1 ms
	_delay_ms(1);
	// Disable temperature senosr (PWR_MGMT_1.TEMP.DIS)
	imu_write(0x6B, 0b00001000);
	// Disable Accelerometer (PWR_MGMT_2.STBY_[X,Y,Z]A)
	imu_write(0x6B, 0b00111000);
	// Use default fullscale range of 250 degrees
	// Possibly: [X,Y,Z]G_OFFS_USR[H,L]
	
	// Test - Read WHO_AM_I = 0xAF
	unsigned char testReg = imu_read(0x75);
	if (testReg == 0xAF){
		PORTD|= (1 << PD0);
	}
	// Test - Compare SELF_TEST_[X,Y,Z]_GYRO vs output as a result of GYRO_CONFIG.[X,Y,Z]G_ST
	unsigned char selfTestX = imu_read(0x00);
	unsigned char selfTestX = imu_read(0x01);
	unsigned char selfTestX = imu_read(0x02);
	imu_write(0x1B, 0b10000000);
	unsigned char readTestX = imu_read(0x1B); // ?? No idea if this is correct
	imu_write(0x1B, 0b01000000);
	unsigned char readTestY = imu_read(0x1B);
	imu_write(0x1B, 0b00100000);
	unsigned char readTestX = imu_read(0x1B);
	if ((selfTestX == readTestX) && (selfTestY == readTestY) && (selfTestZ == readTestZ)){
		PORTD|= (1 << PD1);
	}
	
	// Read values:
	// GYRO_[X,Y,Z]OUT_[H,L]
	unsigned char gyroX_H = imu_read(0x43);
	unsigned char gyroX_L = imu_read(0x44);
	unsigned char gyroY_H = imu_read(0x45);
	unsigned char gyroY_L = imu_read(0x46);
	unsigned char gyroZ_H = imu_read(0x47);
	unsigned char gyroZ_L = imu_read(0x48);
	// Convert to angle: GYRO_[X,Y,Z]OUT = Gyro_Sensitivity * X/Y/Z_angular_rate (Gyro_Sensitivity = 131 LSB/(deg./s))
	// Add writing to SD card to test?
	// May use calibration functions using multiple reads
}