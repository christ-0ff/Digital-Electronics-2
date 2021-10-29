/***********************************************************************
 * 
 * Stopwatch with LCD display output.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2017-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function

uint8_t customChar[] = {
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
    //
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
    //
    0b11100,
    0b11100,
    0b11100,
    0b11100,
    0b11100,
    0b11100,
    0b11100,
    0b11100,
    //
    0b11110,
    0b11110,
    0b11110,
    0b11110,
    0b11110,
    0b11110,
    0b11110,
    0b11110,
    //
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Update stopwatch value on LCD display when 8-bit 
 *           Timer/Counter2 overflows.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);

    // Put string(s) at LCD display
    //lcd_gotoxy(1, 0);
    //lcd_puts("LCD Test");
    //lcd_putc('!');
    
    lcd_gotoxy(1,0);
    lcd_puts("00:00.0   ");
    //lcd_gotoxy(1,1);
    //lcd_puts("b         c");

    // Set pointer to beginning of CGRAM memory
    lcd_command(1 << LCD_CGRAM);
    
    for (uint8_t i = 0; i < 40; i++)
    {
        // Store all new chars to memory line by line
        lcd_data(customChar[i]);    
    }
    
    // Set DDRAM address
    lcd_command(1 << LCD_DDRAM);
              
    // Display first custom character
    //lcd_gotoxy(0,0);
    //lcd_puts(0);
    //lcd_gotoxy(1,0);
    //lcd_putc(1);
    //lcd_gotoxy(2,0);
    //lcd_putc(2);
    //lcd_gotoxy(3,0);
    //lcd_putc(3);
    //lcd_gotoxy(4,0);
    //lcd_putc(4);

    // Configure 8-bit Timer/Counter2 for Stopwatch
    // Set the overflow prescaler to 16 ms and enable interrupt
    TIM2_overflow_interrupt_enable();
    TIM2_overflow_16ms();
     
//    TIM1_overflow_interrupt_enable();
//    TIM1_overflow_262ms();
       
//    TIM0_overflow_interrupt_enable();
//    TIM0_overflow_16ms();

    // Enables interrupts by setting the global interrupt mask
    sei();

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter2 overflow interrupt
 * Purpose:  Update the stopwatch on LCD display every sixth overflow,
 *           ie approximately every 100 ms (6 x 16 ms = 100 ms).
 **********************************************************************/
ISR(TIMER2_OVF_vect)
{
    static uint8_t number_of_overflows = 0;
    static uint8_t tens = 0;        // Tenths of a second
    static uint8_t secs = 0;        // Seconds
    static uint8_t mins = 0;        // Minutes
    
    char lcd_string[2] = "";        // String for converting numbers by itoa()

    //int16_t sec_sqr;                  //MOCNINY...
    //char lcd_string_sqr[4] = "";

    number_of_overflows++;
    if (number_of_overflows >= 6)
    {
        // Do this every 6 x 16 ms = 100 ms
        number_of_overflows = 0;
        tens++; 
        
        if (tens >= 10)
        {
            tens = 0;
            secs++;
            
            if(secs >= 60) 
            {
                mins++;
                secs = 0;
            }    
            
            //sec_sqr = secs * secs;                //MOCNINY...
            //itoa(sec_sqr, lcd_string_sqr, 10);
            //lcd_gotoxy(11,0);
            //lcd_puts(lcd_string_sqr);
        }    
        
        lcd_gotoxy(7,0);                      //TENS TO LCD
        lcd_putc(tens + '0');
        
        itoa(secs, lcd_string, 10);
        lcd_gotoxy(4,0);                      //SECS TO LCD
        if (secs < 10)lcd_putc('0');
        lcd_puts(lcd_string);
   
        lcd_gotoxy(2,0);                      //MINS TO LCD
        itoa(mins, lcd_string, 10);
        lcd_puts(lcd_string);
            
    }
    // Else do nothing and exit the ISR
}
ISR(TIMER0_OVF_vect)                           //BAR for 1 sec.
{
    static uint8_t position = 0;        // Position
    static uint8_t square_pos = 0;      // Square position
    
    lcd_gotoxy(1 + position, 1);
    lcd_putc(square_pos);
    
    square_pos++;
    
    if (square_pos >= 5)
    {
        square_pos = 0;
        position++;
    }
    
    if (position >= 10)
    {
        position = 0;
        lcd_gotoxy(1,1);
        lcd_puts("          ");
    }
}
