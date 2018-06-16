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
#include <string.h>

#include <stdlib.h>
#include "HD44780/hd44780.h"

#include "petitfs/pff.h"

#include "BMP280/BMP280.h"

unsigned int minutes=0, seconds=0;
volatile unsigned char cycle=0;
ISR(TIMER1_COMPA_vect)
{
    TCNT1=0;
    seconds++;
    if (seconds == 60)
        {
            minutes++;
            seconds = 0;
        }
    cycle=1;
    lcd_putc('i');
    sei();
}


void main(void)
{
    //init display
    uint8_t te;
    FATFS fs;
    DDRD |= 1;
    
    lcd_init();
    lcd_clrscr();
    
    lcd_goto(0x00);
    lcd_puts("logger");
    _delay_ms(500);
    lcd_goto(0x40);
    lcd_putc('L');//one
    te=bmp280_init();
    lcd_putc(te+0x30);//two
    te=0;
    //init card and file
    if (pf_mount(&fs)==FR_OK)
        {
            te++;
            lcd_putc(te+0x30);//three
        }
    te=0;
    if (pf_open("LOG.TXT")==FR_OK)
        {
            te++;
            lcd_putc(te+0x30);//four
        }
    te=0;
    //_delay_ms(4000);
    //init timer
    TIMSK |= 1<<OCIE1A; //enable interupt
    TCNT1 = 0; // set counter to 0
    OCR1A = 7812; //set compare to 1 tick/s
    TCCR1B= 0b101; //prescaler to 1024, start
    sei();
    lcd_putc('I');
    for (;;)
    {
        if (cycle==1)
        {   //mmmm:ss,-tt.tt,pppppp/n     22
            char buff[20], buff_temp[8];
            unsigned char count;
            unsigned int count2;
            const char comma[]=",", newline[]="\n";
            buff[0]='\0';
            bmp280_measure();
            itoa(minutes, buff_temp, 10);
            strcat(buff, buff_temp);
            strcat(buff, ":");
            itoa(seconds, buff_temp, 10);
            strcat(buff, buff_temp);
            
            ltoa(bmp280_temp, buff_temp, 10);
            count = strlen(buff_temp);
            buff_temp[count+1]=buff_temp[count];
            buff_temp[count]=buff_temp[count-1];
            buff_temp[count-1]=buff_temp[count-2];
            buff_temp[count-2]='.';
            
            strcat(buff, comma);
            strcat(buff, buff_temp);
            lcd_clrscr();
            lcd_goto(0x00);
            count=0;
            while (buff_temp[count] != '\0')
            {
                lcd_putc(buff_temp[count]);
                count++;
            }
            
            ultoa(bmp280_pres, buff_temp, 10);
            strcat(buff, comma);
            strcat(buff, buff_temp);
            
            lcd_goto(0x40);
            count=0;
            while (buff_temp[count] != '\0')
            {
                lcd_putc(buff_temp[count]);
                count++;
            }
            strcat(buff, newline);
            count = strlen(buff);
            
            pf_write(buff, count, &count2);
            
            if (count != count2)
            {
                lcd_clrscr();
                lcd_goto(0x00);
                lcd_puts("EOF");
                pf_write(0,0,&count2);
                cli();
            }
            cycle=0;
            
        }
        
    }

}

