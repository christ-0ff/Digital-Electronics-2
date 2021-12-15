#ifndef BME280_H_
#define BME280_H_


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

/**
 * @mainpage
 * AVR library for the course Digital Electronics 2, 
 * Brno University of Technology, Czechia
 * @author Tomas Fryza, Peter Fleury, Krystof Buron xd
 * @copyright (c) 2018-2021 Tomas Fryza, This work is licensed under 
 *                the terms of the MIT license
 *
 * @file 
 * @defgroup fryza_twi TWI Library <twi.h>
 * @code #include "bme280.h" @endcode
 *
 * @brief TWI library for AVR-GCC.
 *
 * This library defines functions for the BME280 sensor via TWI (I2C) --
 * communication between AVR and slave device(s).
 * Functions use internal TWI module of AVR.
 *
 * @note Based on Microchip Atmel ATmega16, ATmega328P and BME280 manuals.
 * @author Tomas Fryza, Dept. of Radio Electronics, Brno University 
 *         of Technology, Czechia
 * @copyright (c) 2018-2021 Tomas Fryza, This work is licensed under 
 *                the terms of the MIT license
 * @{
 */

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <stdlib.h>         // C library. Needed for conversion function
#include "twi.h"            // TWI library for AVR-GCC

/* Defines -----------------------------------------------------------*/
/**
 * @name Definitions of global variables
 */
#define int32_t t_fine /**< @brief Fine resolution temperature value */

#define volatile int32_t temp /**< @brief Temperature register data */
#define volatile int32_t press /**< @brief Pressure register data */
#define volatile int32_t hum /**< @brief Humidity register data */

#define int32_t c_temp /**< @brief Compensated temperature data */
#define uint32_t c_press /**< @brief Compensated pressure data */
#define uint32_t c_hum /**< @brief Compensated humidity data */
/**
 * @name Definition of compensation parameters
 */
/**< @brief Temperature compensation data */
#define unsigned short dig_T1
#define short dig_T2
#define short dig_T3
 /**< @brief Pressure compensation data */   
#define unsigned short dig_P1
#define short dig_P2
#define short dig_P4
#define short dig_P4
#define short dig_P5
#define short dig_P6
#define short dig_P7
#define short dig_P8
#define short dig_P9
/**< @brief Humidity compensation data */    
#define unsigned char dig_H1
#define short dig_H2
#define unsigned char dig_H3
#define short dig_H4
#define short dig_H5
#define char dig_H6



/* Function prototypes -----------------------------------------------*/
/**
 * @name Functions
 */

/**
 * @brief  Compensation data readout.
 * @param  none
 * @return none
 */
void comp_data_read(void);


/**
 * @brief  Initialize sensor, turn on IIR filter, set oversampling, set standby and mode.
 * @par    Implementation notes:
 *           - IIR Filter - ON & Filter coefficient - 16.
 *           - Oversampling set to 16 for all values.
 *           - Standby set to 1000ms.
 *           - Mode set - Normal.
 * @return none
 */
uint8_t sensor_init(void);


/**
 * @brief  Read raw sensor measurement data.
 * @param  none
 * @return none
 */
void sensor_data_read(void);


/**
 * @brief  Compensate raw temperature data.
 * @return Received temperature data.
 */
int32_t get_temp(int32_t temp);


/**
 * @brief  Compensate raw pressure data.
 * @return Received pressure data.
 */
uint32_t get_press(int32_t press);


/**
 * @brief  Compensate raw humidity data.
 * @return Received humidity data.
 */
uint32_t get_hum(int32_t hum);

/** @} */



#endif /* BME280_H_ */