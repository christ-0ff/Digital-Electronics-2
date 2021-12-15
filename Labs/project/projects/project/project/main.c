/***********************************************************************
 *
 * The I2C bus scanner detects the addresses of the modules that are
 * connected to the SDA and SCL signals. A simple description of FSM is
 * used.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2017-Present Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <stdlib.h>         // C library. Needed for conversion function
#include "uart.h"           // Peter Fleury's UART library
#include "twi.h"            // TWI library for AVR-GCC

/* Functions ---------------------------------------------------------*/
    int32_t comp_temp(int32_t temp);
    uint32_t comp_press(int32_t press);
    uint32_t comp_hum(int32_t hum);
/* Variables ---------------------------------------------------------*/
    uint8_t result = 1;     // ACK result from the bus
    char uart_string[33];   // String for converting numbers by itoa()
    uint8_t comp_data[25];
    
    int32_t t_fine;
    
    volatile int32_t temp;
    volatile int32_t press;
    volatile int32_t hum;
    
    int32_t c_temp;
    uint32_t c_press;
    uint32_t c_hum;
    
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
       
    
/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and send I2C (TWI) address every 33 ms.
 *           Send information about scanning process to UART.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    // Initialize I2C (TWI)
    twi_init();

    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    // Configure 16-bit Timer/Counter1 to update FSM
    // Set prescaler to 33 ms and enable interrupt
    TIM1_overflow_33ms();
    TIM1_overflow_interrupt_enable();
    
    //TIM0_overflow_4ms();
    //TIM0_overflow_interrupt_enable();
    
    // Enables interrupts by setting the global interrupt mask
    sei();
    
    
    // Data compensation readout
    result = twi_start((0x76<<1) + TWI_WRITE);
            
    //Result
    itoa(result, uart_string, 10);
    uart_puts("data readout result:");
    uart_puts(uart_string);
    uart_puts("\r\n");
    
    if (!result)
    {
        twi_write(0x88);
        twi_start((0x76<<1) + TWI_READ);
        for(int i = 0; i < 25; i++)
        {
            comp_data[i] = twi_read_ack();
        }
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
    
    //Result
    itoa(result, uart_string, 10);
    uart_puts("data readout2 result:");
    uart_puts(uart_string);
    uart_puts("\r\n");
    
    if (!result)
    {
        twi_write(0xE1);
        twi_start((0x76<<1) + TWI_READ);
        for(int i = 0; i < 7; i++)
        {
            comp_data[i] = twi_read_ack();
        }
        twi_stop();
    }
    
    dig_H2 = ((short)comp_data[1] << 8) | ((short)comp_data[0]);                //E2,E1  data0 = E1, data1 = E2
    dig_H3 = (unsigned char)comp_data[2];                                       //E3     data2 = E3
    dig_H4 = (short)comp_data[4];                                               //E5,E4  data3 = E4, data4 = E5
    dig_H4 = ((short)comp_data[3] << 4);
    dig_H5 = ((short)comp_data[4] >> 4) | ((short)comp_data[5] << 4);           //E6,E5  data4 = E5, data5 = E6
    dig_H6 = comp_data[6];                                                      //E7     data6 = E7
    
    // Sensor INIT
    result = twi_start((0x76<<1) + TWI_WRITE);
    
    //Result
    uart_puts("\f");
    itoa(result, uart_string, 10);
    uart_puts("init result:");
    uart_puts(uart_string);
    uart_puts("\r\n");
    
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
        // Empty loop. All subsequent operations are performed exclusively
        // inside interrupt service routines ISRs
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
        uart_puts("\f");
        
        //1--Read TempPressHum
        result = twi_start((0x76<<1) + TWI_WRITE);
                    
        //Result
        itoa(result, uart_string, 10);
        uart_puts("result:");
        uart_puts(uart_string);
        uart_puts("\r\n");
        
        if (!result)
        {
            twi_write(0xF7);
            
            twi_start((0x76<<1) + TWI_READ);
            for(int i = 0; i < 8; i++)
            {
                data[i] = twi_read_ack();
            }
            /*data[0] = twi_read_ack(); //press msb
            data[1] = twi_read_ack(); //press lsb
            data[2] = twi_read_ack(); //press xlsb
            data[3] = twi_read_ack(); //temp msb
            data[4] = twi_read_ack(); //temp lsb
            data[5] = twi_read_ack(); //temp xlsb
            data[6] = twi_read_ack(); //hum msb
            data[7] = twi_read_ack(); //hum lsb*/
            twi_stop();
                  
            upress = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);
            utemp = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);
            uhum = ((uint32_t)data[6] << 8) | ((uint32_t)data[7]);

            press = (int32_t)upress;
            temp = (int32_t)utemp;
            hum = (int32_t)uhum;
                        
            ltoa(upress, uart_string, 16);
            uart_puts(uart_string);
            uart_puts("\r\n");
            ltoa(utemp, uart_string, 16);
            uart_puts(uart_string);
            uart_puts("\r\n");
            ltoa(uhum, uart_string, 16);
            uart_puts(uart_string);
            uart_puts("\r\n");
      
        }
    }
}

ISR(TIMER0_OVF_vect)
{
    static int i = 0;
        if (i <= 2000)//250=1s
        {
            i++;
        }
        else
        {
            i = 0;
            c_temp = comp_temp(temp);
            ltoa(c_temp, uart_string, 10);
            //uart_puts("uart_string");
            
            c_press = comp_press(press);
            ltoa(c_press, uart_string, 10);
            //uart_puts("uart_string");
            
            c_hum = comp_hum(hum);
            ltoa(c_hum, uart_string, 10);
            //uart_puts("uart_string");
        }
}

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
        v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6))
        >>10)*(((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
        ((int32_t)2097152)) * ((int32_t)dig_H2)+ 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)*
        ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (uint32_t)(v_x1_u32r>>12);
}
