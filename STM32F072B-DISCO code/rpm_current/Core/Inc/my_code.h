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

//vikash
#define TIMCLOCK  48000000
#define PRESCALAR  48

// redeclare external global variables so that my functions can see them
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef hadc;
extern I2C_HandleTypeDef hi2c1;

// redeclare public globals
extern float rpm;
extern uint16_t readValue;

// define custom struct for gps global variables
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

/* custom struct to hold all variables for a hall effect current sensor */
struct hall_struct
{
	float rawVoltage;
	float current;
	float offsetVoltage;
};

struct lora_struct
	{
		enum {tx , canSend } buff_state;
		int index;
		char buff[256];
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
