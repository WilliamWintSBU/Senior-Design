/*
 * my_code.h
 *
 *  Created on: Mar 14, 2023
 *      Author: willi
 */

#ifndef MY_CODE_H_
#define MY_CODE_H_

// include auto generated main file
#include "main.h"

/* define custom macros*/
#define LORA_RECIVER_ADDRESS 5
#define ADS1115_ADDRESS 0b1001000;
#define HALL_SENSITIVITY 600.0 / 625.0 // sensitivity of current sensor in A/mV
#define LORA_PAYLOAD_MAX_LENGTH 256
#define ADC_CAL_MULTIPLIER 1
#define VOLT_DIV_RATIO 1
#define ADC_PROCESS_VOLTAGE(rawVoltage) rawvoltage * ADC_CAL_MULTIPLIER * VOLT_DIV_RATIO

// macros for vikash's code
#define TIMCLOCK  48000000
#define PRESCALAR  48

// redeclare external global variables so that my functions can see them
extern UART_HandleTypeDef huart3; // for gps
extern UART_HandleTypeDef huart1; // for lora
extern ADC_HandleTypeDef hadc;	  // for internal ADC channels
extern I2C_HandleTypeDef hi2c1;   //  for ADS1115


// redeclare public globals
extern float rpm;
extern uint16_t readValue;

/// @brief custom struct for gps global variables
struct gps_struct
	{
		enum {read , write} buffStateFlag;
		int tail;
		enum { init, seen$, seenNewline } parsingState;
		char newchar;
		char buff[1024] ;
		int lineCounter;
		int numLines;

	};

/** @brief custom struct to hold all variables for a hall effect current sensor */
struct hall_struct
{
	float rawVoltage;
	float current;
	float offsetVoltage;
};



// declare functions
void extern setup();

void extern loop();

void gps_rx();

void lora_TX_blocking(char * payload , int length);
void lora_tx_it(char * payload , int length);
void debug_print_blocking(char * string);

void gps_blocking();

void LCD_test();
void LCD_paint();
void hall_read();
void hall_calibrate();




//ADC
void ADC_Select_CH0 (void);
void ADC_Select_CH3 (void);
void ADC_Select_CH2 (void);
void ADC_Select_CH1 (void);
void readVoltage(float div);
#endif /* MY_CODE_H_ */
