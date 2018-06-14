/*
 * main.c
 * 
 * Copyright 2018 username <username@zvorikin>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
// F_CPU 8MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include "HD44780/hd44780.h"

#include "petitfs/pff.h"

#include "BMP280/BMP280.h"

void main(void)
{
    uint8_t te;
    FATFS fs;
    DDRD |= 1;
    
    lcd_init();
    lcd_clrscr();
    
    lcd_goto(0x00);
    lcd_puts_P("logger");
    lcd_goto(0x40);
    te=bmp280_init();
    lcd_putc(te+0x30);
    te=0;
    
    if (pf_mount(&fs)==FR_OK)
        {
            te++;
        }
    lcd_putc(te+0x30);
    te=0;
    if (pf_open("log.txt")==FR_OK)
        {
            te++;
        }
    lcd_putc(te+0x30);
    te=0;
    _delay_ms(4000);
    
    bmp280_measure();
    

}

