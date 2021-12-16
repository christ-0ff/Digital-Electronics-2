/***********************************************************************
 * 
 * TWI library for AVR-GCC.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2018-2021 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include "bme280.h"

/* Functions ---------------------------------------------------------*/
/**********************************************************************
 * Function: comp_data_read()
 * Purpose:  Read, convert and store compensation data for data calculations.
 * Returns:  none
 **********************************************************************/
void comp_data_read(void)
{
	uint8_t comp_data[25];
		
	result = twi_start((0x76<<1) + TWI_WRITE);

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
	
	if (!result)
	{
		twi_write(0xE1);
		twi_start((0x76<<1) + TWI_READ);
		for(int i = 0; i < 7; i++)
		{
			comp_data[i] = twi_read_ack();
		}
		twi_stop();
		
		dig_H2 = ((short)comp_data[1] << 8) | ((short)comp_data[0]);                //E2,E1  data0 = E1, data1 = E2
		dig_H3 = (unsigned char)comp_data[2];                                       //E3     data2 = E3
		dig_H4 = (short)comp_data[4];                                               //E5,E4  data3 = E4, data4 = E5
		dig_H4 = ((short)comp_data[3] << 4);
		dig_H5 = ((short)comp_data[4] >> 4) | ((short)comp_data[5] << 4);           //E6,E5  data4 = E5, data5 = E6
		dig_H6 = comp_data[6];												    	 //E7     data6 = E7
	}                                               
}

/**********************************************************************
 * Function: sensor_init()
 * Purpose:  Initialize sensor, turn on IIR filter, set oversampling, set standby and mode.
 * Input:    none
 * Returns:  0 - none
 *           1 - Failed to access slave device.
 **********************************************************************/
uint8_t sensor_init(void)
{
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
	else
	{
		return 1;
	}
}

/**********************************************************************
 * Function: sensor_data_read()
 * Purpose:  Get raw sensor data.
 * Input:    none
 * Returns:  none
 **********************************************************************/
void sensor_data_read(void)
{
	uint8_t data[8];
	
	uint32_t utemp;
	uint32_t upress;
	uint32_t uhum;
	
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
   
		/*data[0] = twi_read_ack(); //press msb
		data[1] = twi_read_ack(); //press lsb
		data[2] = twi_read_ack(); //press xlsb
		data[3] = twi_read_ack(); //temp msb
		data[4] = twi_read_ack(); //temp lsb
		data[5] = twi_read_ack(); //temp xlsb
		data[6] = twi_read_ack(); //hum msb
		data[7] = twi_read_ack(); //hum lsb*/ 
   
        upress = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);
        utemp = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);
        uhum = ((uint32_t)data[6] << 8) | ((uint32_t)data[7]);

        press = (int32_t)upress;
        temp = (int32_t)utemp;
        hum = (int32_t)uhum;     
	}
}

/**********************************************************************
 * Function: get_temp()
 * Purpose:  Compensate raw temperature data.
 * Returns:  Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23DegC.
 **********************************************************************/
int32_t get_temp(int32_t temp)
{
    int32_t var1, var2, T;
    
    var1 = ((((temp>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((temp>>4) - ((int32_t)dig_T1)) * ((temp>>4) - ((int32_t)dig_T1)))
    >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

/**********************************************************************
 * Function: get_press()
 * Purpose:  Compensate raw pressure data.
 * Returns:  Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
			 Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
 **********************************************************************/
uint32_t get_press(int32_t press)
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
	    return 0;
    }
    p = 1048576 - press;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
    return (uint32_t)p;
}

/**********************************************************************
 * Function: get_hum()
 * Purpose:  Compensate raw humidity data.
 * Returns:  Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
			 Output value of “47445” represents 47445/1024 = 46.333 %RH
 **********************************************************************/
uint32_t get_hum(int32_t hum)
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
