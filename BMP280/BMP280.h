#ifndef BMP280_H
#define BMP280_H

/***********************************************************************
 *Libruary to interface with BMP280 Bosch sensor
 *to use in warious projects
 * 
 * Use with included libs, and all will be OK!
 * At list I think it will be
 **********************************************************************/
 
 #include <avr/io.h>
 
 #define BMP280_ADDR 0x76 //0x77 (to Vcc) or 0x76 (to GND)
 
  
//here is oversampling controls in CONTROL reg
#define BMP280_PRES_OVERSAMPLING 0b100 //place here table of P oversamp
/* 0b000 Skiped
 * 0b001 Oversampling x1
 * 0b010 Oversampling x2
 * 0b011 Oversampling x4
 * 0b100 Oversampling x8
 * 0b101 Oversampling x16
 */
 
#define BMP280_TEMP_OVERSAMPLING 0b100 //place here table of T oversamp
/* 0b000 Skiped
 * 0b001 Oversampling x1
 * 0b010 Oversampling x2
 * 0b011 Oversampling x4
 * 0b100 Oversampling x8
 * 0b101 Oversampling x16
 */

#define BMP280_POWER_MODE 0b11 //place here table of power modes
/* 0b00 Sleep mode
 * 0b01 or 0b10 Forced mode (one-shot)
 * 0b11 Normal mode (Continious mesuring)
 */
 
//Here is config settings in CONFIG register
#define BMP280_TIME_STANDBY 0b100 //Stand-by in normal mode
/* 0b000 0.5ms
 * 0b001 62.5ms
 * 0b010 125ms
 * 0b011 250ms
 * 0b100 500ms
 * 0b101 1000ms
 * 0b110 2000ms
 * 0b111 4000ms
 */
#define BMP280_IIR_FILTER 0b011 //IIR filter for measuring
/* 0b000 off
 * 0b001 2
 * 0b010 4
 * 0b011 8
 * 0b100 16
 */
#define BMP280_3WIRE_EN 0b0 //3-wire interface, you do not need it


 uint8_t bmp280_init (void); //base initialization, 1 is sucsess
 uint8_t bmp280_get_status (void); //get status register
 void bmp280_set_config (void); //set config reg 
 void bmp280_set_control (void); //set control red (oversampling i.e)
 void bmp280_measure (void); //get T, P and compute it readable format
 //#define bmp280_get_temperature() {bmp280_temp} //returns temperature
 //#define bmp280_get_pressure() {bmp280_pres} //returns pressure

volatile int32_t bmp280_temp; //variable for temp
volatile uint32_t bmp280_pres; //variable for pressure

#endif //BMP280_H
