/***********************************************************************
 * 
 * Decimal counter with 7-segment output.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2018-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
# define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "segment.h"        // Seven-segment display library for AVR-GCC

volatile uint8_t digit1;
volatile uint8_t digit0;
volatile uint8_t citac;


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Display decimal counter values on SSD (Seven-segment 
 *           display) when 16-bit Timer/Counter1 overflows.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Configure SSD signals
    SEG_init();

    // Test of SSD: display number '3' at position 0
    SEG_update_shift_regs(3,0);
    // Configure 16-bit Timer/Counter1 for Decimal counter
    // Set the overflow prescaler to 262 ms and enable interrupt
    //TIM1_overflow_262ms();
    //TIM1_overflow_interrupt_enable();
    //
    //TIM0_overflow_4ms();
    //TIM0_overflow_interrupt_enable();
    // Enables interrupts by setting the global interrupt mask
    
    sei();

    // Infinite loop
    while (1)
    {
    SEG_update_shift_regs(9,0);
    SEG_update_shift_regs(5,1);
    SEG_update_shift_regs(11,2);
    SEG_update_shift_regs(9,2);
    SEG_update_shift_regs(5,3);
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Increment decimal counter value and display it on SSD.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    if(citac <= 59)
    {
    citac++;    
    digit1 = citac / 10;
    digit0 = citac % 10;
    }
    
    else
    {
    citac = 0;
    }
}

ISR(TIMER0_OVF_vect)
{
    static uint8_t pos = 0;
        if (pos == 0)
        {
            SEG_update_shift_regs(digit1, 1);
            pos++;
        }
        else
        { 
            SEG_update_shift_regs(digit0, 0);
            pos = 0;      
        }
       
}
