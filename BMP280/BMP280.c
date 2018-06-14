/***********************************************************************
 * Functions for BMP280 
 * **********************************/
  
 #include "BMP280.h"
 #include <avr/io.h>
 #include <util/delay.h>
 #include "../i2c/i2cmaster.h"
 
#define BMP280_ID_REG		0xD0 //ID in 280 here
#define BMP280_ID_VAL		0x58 //it's content

#define BMP280_CAL_REG_FIRST	0x88 //fist calibration reg
#define BMP280_CAL_REG_LAST	0xA1 //last calibration reg, read to this
#define BMP280_CAL_DATA_SIZE	(BMP280_CAL_REG_LAST+1 - BMP280_CAL_REG_FIRST)
//size of calibration data

#define BMP280_STATUS_REG	0xF3 //status, a little of data
#define BMP280_CONTROL_REG	0xF4 //control, oversampling T & P, and power
#define BMP280_CONFIG_REG	0xF5 //rate, filter and interface

#define BMP280_PRES_REG		0xF7 //first pressure reg
#define BMP280_TEMP_REG		0xFA //first temperature reg
#define BMP280_RAWDATA_BYTES	6	//size of data


static void bmp280_writemem (uint8_t reg, uint8_t value)
{
	i2c_start_wait((BMP280_ADDR << 1) | I2C_WRITE);
	i2c_write(reg);
	i2c_write(value);
	i2c_stop();
}

void bmp280_readmem (uint8_t reg, uint8_t buffer[], uint8_t bytes)
{
	uint8_t i;
	i2c_start_wait ((BMP280_ADDR << 1) | I2C_WRITE);
	i2c_write (reg);
	i2c_rep_start ((BMP280_ADDR << 1) | I2C_READ);
	
	for (i=0; i<bytes; i++)
	{
		if (i==bytes-1)
			buffer[i]=i2c_readNak();
		else
			buffer[i]=i2c_readAck();
	}
	i2c_stop();
}

static union bmp280_cal_union 
	{
		uint8_t bytes[BMP280_CAL_DATA_SIZE];
		struct
			{
			uint16_t dig_t1;
			int16_t  dig_t2;
			int16_t  dig_t3;
			uint16_t dig_p1;
			int16_t  dig_p2;
			int16_t  dig_p3;
			int16_t  dig_p4;
			int16_t  dig_p5;
			int16_t  dig_p6;
			int16_t  dig_p7;
			int16_t  dig_p8;
			int16_t dig_p9;
			};
	} bmp280_cal;
	
static void bmp280_get_calibration (void)
{
	bmp280_readmem (BMP280_CAL_REG_FIRST, bmp280_cal.bytes, BMP280_CAL_DATA_SIZE);
}

#define BMP280_CONTROL_VALUE ((BMP280_TEMP_OVERSAMPLING << 5) | (BMP280_PRES_OVERSAMPLING << 2) | (BMP280_POWER_MODE))

void bmp280_set_control (void)
{
	bmp280_writemem (BMP280_CONTROL_REG, BMP280_CONTROL_VALUE);
}



#define BMP280_CONFIG_VALUE ((BMP280_TIME_STANDBY << 5) | (BMP280_IIR_FILTER << 2) | (BMP280_3WIRE_EN))

void bmp280_set_config (void)
{
	bmp280_writemem (BMP280_CONFIG_REG, BMP280_CONFIG_VALUE);
}

uint8_t bmp280_init (void)
{
	i2c_init();
	_delay_us(20);
	 uint8_t buffer[1];
	 buffer[0]=0;
	 
	 bmp280_readmem (BMP280_ID_REG, buffer, 1);
	 if (buffer[0] != BMP280_ID_VAL) return 0;
	 
	 bmp280_get_calibration();
	 bmp280_set_control ();
	 bmp280_set_config ();
	 return 1;
}

uint8_t bmp280_get_status (void)
{
	uint8_t status[1];
	status[0]=0;
	bmp280_readmem (BMP280_STATUS_REG, status, 1);
	return status[0];
}

#define bmp280_20bit_reg(b1, b2, b3)	( ((int32_t)(b1) << 12) | ((int32_t)(b2) << 4) | ((int32_t)(b3) >> 4) );


void bmp280_measure(void)
{
	uint8_t data[BMP280_RAWDATA_BYTES];
	int32_t temp_raw, pres_raw,
		var1, var2, t_fine;
	
	// read the raw ADC data from the I2C registers
	bmp280_readmem(BMP280_PRES_REG, data, BMP280_RAWDATA_BYTES);
	pres_raw = bmp280_20bit_reg(data[0], data[1], data[2]);
	temp_raw = bmp280_20bit_reg(data[3], data[4], data[5]);

	// The following code is based on a 32-bit integer code
	// from the BMP280 datasheet

	// compute the temperature
	var1 = ((((temp_raw >> 3) - ((int32_t)bmp280_cal.dig_t1 << 1)))
		* ((int32_t)bmp280_cal.dig_t2)) >> 11;
	var2 = (((((temp_raw >> 4) - ((int32_t)bmp280_cal.dig_t1))
		* ((temp_raw >> 4) - ((int32_t)bmp280_cal.dig_t1))) >> 12)
		* ((int32_t)bmp280_cal.dig_t3)) >> 14;
	t_fine = var1 + var2;
	bmp280_temp = (t_fine * 5 + 128) >> 8;
	

	// compute the pressure
	var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)bmp280_cal.dig_p6);
	var2 = var2 + ((var1 * ((int32_t)bmp280_cal.dig_p5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)bmp280_cal.dig_p4) << 16);
	var1 = (((bmp280_cal.dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3)
		+ ((((int32_t)bmp280_cal.dig_p2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)bmp280_cal.dig_p1)) >> 15);

	if (var1 == 0) {
		bmp280_pres = 0;
	} else {
		bmp280_pres = (((uint32_t)(((int32_t)1048576)-pres_raw)
			- (var2 >> 12))) * 3125;
		if (bmp280_pres < 0x80000000) {
			bmp280_pres = (bmp280_pres << 1) / ((uint32_t)var1);
		} else {
			bmp280_pres = (bmp280_pres / (uint32_t)var1) * 2;
		}
		var1 = (((int32_t)bmp280_cal.dig_p9) * ((int32_t)(((bmp280_pres>>3) * (bmp280_pres >> 3)) >> 13))) >> 12;
		var2 = (((int32_t)(bmp280_pres >> 2)) * ((int32_t)bmp280_cal.dig_p8)) >> 13;
		bmp280_pres = (uint32_t)((int32_t)bmp280_pres + ((var1 + var2 + bmp280_cal.dig_p7) >> 4));
	}


}
