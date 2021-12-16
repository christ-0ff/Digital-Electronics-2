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

/* Defines -----------------------------------------------------------*/
#define BTN	PC6
#define servo1 PB1
#define servo2 PB2
#define SW PD5

#ifndef F_CPU
# define F_CPU 16000000     // CPU frequency in Hz required for delay
#endif

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function
#include <stdbool.h>		// Add bool data type
#include "gpio.h"			// GPIO library
#include "twi.h"			// I2C library
#include <util/delay.h>     // Functions for busy-wait delay loops
#include <avr/sfr_defs.h>	// For testing bit values in control registers

/* Variables ---------------------------------------------------------*/
char lcd_value[8];
static uint8_t btnpressed = 0; // Variable for button to not act as its being pressed multiple times during interrupts

typedef enum {              // FSM - declaration of states
	STATE_START,
	STATE_TEMP,
	STATE_HUM,
	STATE_PRES,
} state_t;
state_t state = STATE_START;

static uint8_t next_state = 0;

uint16_t photoresistorValue;

uint8_t result = 1;     // ACK result from the bus
char uart_string[33];   // String for converting numbers by itoa()


int32_t t_fine;

volatile int32_t temp;
volatile int32_t press;
volatile int32_t hum;

int32_t c_temp;
uint32_t c_press;
uint32_t c_hum;

uint32_t press_frac = 256.00;
uint32_t hum_frac = 1024.00;
uint32_t temp_frac = 1000;

// Compensation parameters
unsigned short dig_T1;
short dig_T2;
short dig_T3;

unsigned short dig_P1;
short dig_P2;
short dig_P3;
short dig_P4;
short dig_P5;
short dig_P6;
short dig_P7;
short dig_P8;
short dig_P9;

unsigned char dig_H1;
short dig_H2;
unsigned char dig_H3;
short dig_H4;
short dig_H5;
char dig_H6;


/* Functions ---------------------------------------------------------*/
 int32_t comp_temp(int32_t temp);
 uint32_t comp_press(int32_t press);
 uint32_t comp_hum(int32_t hum);

//defining ADC-read function
uint16_t readADC (uint8_t channel)
{
    ADMUX = (0xf0 & ADMUX) | channel;	//Set input channel to read
    ADCSRA |= (1 << ADSC);	//Start an ADC conversion by setting ADSC bit
    loop_until_bit_is_clear(ADCSRA, ADSC);	//wait until AD convertion is finished
    return (ADC);
}

/* Main function ----------------------------------------------*/

int main(void)
{
	// Register settings for servos
	DDRB |= (1 << servo1);	//OC1A
	DDRB |= (1 << servo2);	//OC1B
	DDRD |= ~(1 << SW);  // Arduino PIN5
	PORTD |= (1 << SW);
	
	//PWM is set to non-inverting mode
	//Setting Fast PWM mode 14: set WGM11, WGM12, WGM13 to 1, set prescaler to 8
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	ICR1 = 19999;	//Set ICR1 register: PWM period
	
	ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0); //ADC clock prescaler to 8
	ADCSRA |= (1 << ADEN); //enables the ADC
	
	// Configure button for switching between states
	GPIO_config_input_pullup(&DDRC, BTN);
	GPIO_write_low(&PORTC, BTN);
    
	// Initialize LCD display	
	lcd_init(LCD_DISP_ON);
	
	lcd_gotoxy(0, 0);
	lcd_puts("Weather ");
	lcd_gotoxy(8, 0);
	lcd_puts("station ");
	
	// Initialize I2C (TWI)
	twi_init();
	
	// Configure 16-bit Timer/Counter1 to update FSM
	// Set prescaler to 33 ms and enable interrupt
	TIM1_overflow_33ms();
	TIM1_overflow_interrupt_enable();
	
    // Configure 8-bit Timer/Counter2
	// Set prescaler to 128 us and enable interrupt
	TIM2_overflow_128us();
	TIM2_overflow_interrupt_enable();

    // Enables interrupts by setting the global interrupt mask
    sei();
	
	// COMPENSATION DATA READOUT - needed for calculation of the output data
	result = twi_start((0x76<<1) + TWI_WRITE);
	if (!result)
	{
		uint8_t comp_data[24];
		
		twi_write(0x88);
		twi_start((0x76<<1) + TWI_READ);
		for(int i = 0; i <= 24; i++)
		{
			comp_data[i] = twi_read_ack();
		}
		comp_data[24] = twi_read_nack();
		twi_stop();
		
		dig_T1 = ((unsigned short)comp_data[1] << 8) | ((unsigned short)comp_data[0]); //89,88
		dig_T2 = ((short)comp_data[3] << 8) | ((short)comp_data[2]);                   //8B,8A
		dig_T3 = ((short)comp_data[5] << 8) | ((short)comp_data[4]);                   //8C,8D
		
		dig_P1 = ((unsigned short)comp_data[7] << 8) | ((unsigned short)comp_data[6]); //8F,8E
		dig_P2 = ((short)comp_data[9] << 8) | ((short)comp_data[8]);                   //91,90
		dig_P3 = ((short)comp_data[11] << 8) | ((short)comp_data[10]);                 //93,92
		dig_P4 = ((short)comp_data[13] << 8) | ((short)comp_data[12]);                 //95,94
		dig_P5 = ((short)comp_data[15] << 8) | ((short)comp_data[14]);                 //97,96
		dig_P6 = ((short)comp_data[17] << 8) | ((short)comp_data[16]);                 //99,98
		dig_P7 = ((short)comp_data[19] << 8) | ((short)comp_data[18]);                 //9B,9A
		dig_P8 = ((short)comp_data[21] << 8) | ((short)comp_data[20]);                 //9D,9C
		dig_P9 = ((short)comp_data[23] << 8) | ((short)comp_data[22]);                 //9F,9E
		
		dig_H1 = (unsigned char)comp_data[24];                                         //A1
	}

	result = twi_start((0x76<<1) + TWI_WRITE);
	if (!result)
	{
		uint8_t comp_data[7];
		
		twi_write(0xE1);
		twi_start((0x76<<1) + TWI_READ);
		for(int i = 0; i <= 5; i++)
		{
			comp_data[i] = twi_read_ack();
		}
		comp_data[6] = twi_read_nack();
		twi_stop();
		
		dig_H2 = ((short)comp_data[1] << 8) | ((short)comp_data[0]);
		dig_H3 = (unsigned char)comp_data[2];
		dig_H4 = (short)comp_data[4];
		dig_H4 = ((short)comp_data[3] << 4);
		dig_H5 = ((short)comp_data[4] >> 4) | ((short)comp_data[5] << 4);
		dig_H6 = comp_data[6];
	}
	
	/* SENSOR INIT */
	result = twi_start((0x76<<1) + TWI_WRITE);
	if (!result)
	{
		twi_write(0xF5);        //CONFIG
		twi_write(0b10110000);
		twi_write(0xF2);        //CTRL_HUM
		twi_write(0b00000101);
		twi_write(0xF4);        //CTRL_MEAS
		twi_write(0b10110111);
		twi_stop();
	}

	
    // Infinite loop
    while (1)
    {
		// Function for servos - can be operated either by photoresistor or joystick
        if(bit_is_clear(PIND,5)) //hold the SW bu
        {
            OCR1A = (readADC(ADC1D)*5); //reading analog value from A_pin A1
            
            OCR1B = (readADC(ADC2D)*5); //reading analog value from A_pin A2
        }
        if(bit_is_set(PIND,5))
        {
            ADCSRA |= (1 << ADSC); //start ADC conversion
            loop_until_bit_is_clear(ADCSRA, ADSC); //wait until ADC conversion is done
            photoresistorValue = ADC; //read ADC value from photo-resistor
            
            OCR1A = 500 + photoresistorValue*5.5;  //Servo reads values from photo-resistor,where 500 is 0 deg

            OCR1B = 500 + photoresistorValue*5.5;
        }
        
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/

ISR(TIMER1_OVF_vect)
{
	static int i = 0;
	uint8_t data[8];
	
	uint32_t utemp;
	uint32_t upress;
	uint32_t uhum;

	if (i <= 150)//30=1s
	{
		i++;
	}
	else
	{
		i = 0;
		
		//1--Read TempPressHum
		result = twi_start((0x76<<1) + TWI_WRITE);
		if (!result)
		{
			twi_write(0xF7);
			twi_start((0x76<<1) + TWI_READ);
			for(int i = 0; i < 8; i++)
			{
				data[i] = twi_read_ack();
			}
			twi_stop();
			
			upress = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);
			utemp = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);
			uhum = ((uint32_t)data[6] << 8) | ((uint32_t)data[7]);

			press = (int32_t)upress;
			temp = (int32_t)utemp;
			hum = (int32_t)uhum;
		}
	}
}

ISR(TIMER2_OVF_vect)
{
	// Interrupt service routine for switching states and display of corresponding data
	static uint8_t counter = 0; // Counter for running following functions only once every 128*250 us
	char lcd_string[3];
	if(counter <= 250){
		counter++;
	}
	else{			
		counter = 0;
		if(!GPIO_read(&PINC, BTN)){
			if(btnpressed == 0){
				next_state++;
				btnpressed = 1;
			}
		}
		else if(GPIO_read(&PINC, BTN))
		{
			btnpressed = 0;
		};
		
		switch(state)
		{
		// Starting state - does nothing, "Weather station" is displayed
		case STATE_START:
			if(next_state == 1){
				state = STATE_TEMP;
			}
			break;
		// Displays temperature
		case STATE_TEMP:
			if(next_state == 2){
				state = STATE_HUM;
			}
			lcd_gotoxy(0, 0);
			lcd_puts("                ");
			lcd_gotoxy(0, 0);
			lcd_puts("  TEMP: ");
			lcd_gotoxy(8, 0);
			c_temp = comp_temp(temp)/temp_frac;
			ltoa(c_temp, lcd_string, 10);
			lcd_puts(lcd_string);
			lcd_gotoxy(10, 0);
			lcd_puts(" °C");
			break;
		// Displays humidity
		case STATE_HUM:
			if(next_state == 3){
				state = STATE_PRES;
			}
			lcd_gotoxy(0, 0);
			lcd_puts("                ");
			lcd_gotoxy(0, 0);
			lcd_puts("   HUM: ");
			c_hum = comp_hum(hum)/hum_frac;
			ltoa(c_hum, lcd_string, 10);
			lcd_gotoxy(8, 0);
			lcd_puts(lcd_string);
			lcd_gotoxy(11, 0);
			lcd_puts(" %");
			break;
		// Displays pressure
		case STATE_PRES:
			// Sends FSM to the second state
			if(next_state > 3){
				state = STATE_TEMP;
				next_state = 1;
			}
			lcd_gotoxy(0, 0);
			lcd_puts("                ");
			lcd_gotoxy(0, 0);
			lcd_puts(" PRESS: ");
			c_press = comp_press(press)/(press_frac*1000);
			ltoa(c_press, lcd_string, 10);
			lcd_gotoxy(8, 0);
			lcd_puts(lcd_string);
			lcd_gotoxy(11, 0);
			lcd_puts(" hPa");
			break;
		}	
	}
}
/* Compensation functions --------------------------------------------*/
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23DegC.
// t_fine carries fine temperature as global value
int32_t comp_temp(int32_t temp)
{
	int32_t var1, var2, T;
	
	var1 = ((((temp>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((temp>>4) - ((int32_t)dig_T1)) * ((temp>>4) - ((int32_t)dig_T1)))
	>> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format
//(24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t comp_press(int32_t press)
{
	int64_t var1, var2, p;
	
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0)
	{
		return 0;   // avoid exception caused by division by zero
	}
	p = 1048576 - press;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
	return (uint32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format
// (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32_t comp_hum(int32_t hum)
{
	int32_t v_x1_u32r;
	
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((hum << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5)*
	v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *
	((int32_t)dig_H6))>>10)*(((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +
	((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2)+
	8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)*
	((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	
	return (uint32_t)(v_x1_u32r>>12);
}
