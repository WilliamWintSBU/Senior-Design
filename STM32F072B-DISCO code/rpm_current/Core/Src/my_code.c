/*! \file my_code.c*/
/*
 * 	File Name: my_code.c
 *
 *  Created on: Mar 14, 2023
 *  updated: 4/18/2023
 *  Authors: William Winters and Vikash Ramharack
 *
 *  Written for senior design as Stony Brook University
 *  for the Stony Brook Solar Racing team
 *
 *  Copyright William Winters and Vikash Ramharack 2023
 *	released under GPL version 3
 *	Any example code or external libraries used here are the property of their owners
 */

/* include standard libraries */
#include <string.h>
#include <stdio.h>

/*
 * include custom header with function declarations ect.
 * if something is broken you may need to look here
 */
#include "my_code.h"
//#include "TestFuncs.h"

/* include third party libraries */
#include "lwgps/lwgps.h"
#include "ads1115.h"
#include "LCD.h"
#include "MAX31865.h"
#include "pcf8574.h"



/* define global variables */

char lora_payload[LORA_PAYLOAD_MAX_LENGTH];

/* define GPS stuff */
volatile struct gps_struct gps; /// custom struct see my_code.h
lwgps_t hgps; /// struct used by GPS parsing library

// for tachometer
//float Frequency = 0;
//float rpm = 0;

// for hall effect sensors
struct hall_struct hall1, hall2, hall3;


// for tach
uint32_t IC_Val1 = 0;
uint32_t IC_Val2 = 0;
uint32_t Difference = 0;
int Is_First_Captured = 0;

//for temp sensor
Max31865_t  pt100;
bool        pt100isOK;
float       pt100Temp;

float Frequency = 0;
float rpm = 0;
uint16_t readValue;
float sensitivity = 0.001; // 0.1 for 20A Model
float rawVoltage;
float current;

float volt1 = 0;
float volt2 = 0;
float volt3 = 0;
float volt0 = 0;

void ADC_Disable_channels(void);

/**
 * \brief called before while loop of auto generated main.c. Runs once
 * */
void setup()
{

	//init lcd
	//LCD_Init();
	pcf8574_init();
	pcf8574_clr();
	pcf8574_cursor(0, 0);
	pcf8574_send_string("  HELLO SKIPPER O7");
	HAL_Delay(1000);

	// tach setup
	//HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);


	// gps setup code
	gps.tail = 0;
	gps.buffStateFlag = write;
	gps.numLines = 14;
	lwgps_init(&hgps); // init gps parsing library

	HAL_UART_Receive_IT(&huart3 , (uint8_t *) & gps.newchar , 1); // could use higher size and change parsing
	// end gps setup code

	//external ADC setup code
	if(ADS1115_Init(&hi2c1, ADS1115_DATA_RATE_64, ADS1115_PGA_FOUR) != HAL_OK){
		// init failed
		//while(1){}
		pcf8574_clr();
		pcf8574_cursor(0, 0);
		pcf8574_send_string("EXT ADC INIT FAILED");
		HAL_Delay(1000);
	}
	//end external ADC setup code

	// init internal adc
	HAL_ADC_Stop(&hadc);
	HAL_ADCEx_Calibration_Start (&hadc);
	HAL_ADC_Start(&hadc);

	//test_gps_blocking();


	// temp setup
	Max31865_init(&pt100,&hspi2,GPIOA,GPIO_PIN_8,4,50);

}// end setup

/**
 * \brief called in while loop of auto generated main.c. Runs forever
 * */
void loop()
{

	// get gps data
	gps_rx();

	// get current
	hall_read();

	//get voltage
	/* Poll for voltage divider*/
	readVoltage(1.0);

	volt1 = volt1 * ((2700.0 + 36000.0) / 2700.0) * (1183.0/1305.0) ;


	//get temperature
	float t;
	pt100isOK = Max31865_readTempC(&pt100,&t);
	pt100Temp = Max31865_Filter(t,pt100Temp,0.1);   //  << For Smoothing data


	//update display
	LCD_paint();



	/* tx data */

	//message code,lat,long,Speed,Battery Current,Motor 1 Current,Motor 2 current,Solar current,V12,V24,V36,Vm1,Vm2,Vsolar,Temp1,Temp2
	//0,40.546906,73.0733222,12.3,200.1,400.1,400.1,40.1,12.1,24.2,36.3,16.1,16.1,17.5,20.5,30.5
	char lora_message[LORA_PAYLOAD_MAX_LENGTH];
	sprintf(lora_message,"%d,%f,%f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",0,hgps.latitude,hgps.longitude,hgps.speed,hall1.current,hall2.current,hall3.current,420.0,420.0,42.0,420.0,420.0,420.0,420.0,420.0,420.0);

	//sprintf(payload,"%d RPM, %d c, %d s, %d lat , %d lon",(int)rpm,(int)pt100Temp, (int) hgps.speed /10, hgps.latitude * 1000000 , hgps.longitude * 1000000);
	//sprintf(lora_message,"0,40.546906,73.0733222,12.3,200.1,400.1,400.1,40.1,12.1,24.2,36.3,16.1,16.1,17.5,20.5,30.5");
	//lora_TX_blocking(payload, strlen(payload));
	lora_tx_it(lora_message , strlen(lora_message));


	// delay
	HAL_Delay(1000);

}



/**
 * \brief This function assembles a message and sends it to the display.
 * It may block temporarily due to internal delay functions
 * */
void LCD_paint()
{
	char line1[20];
	char line2[20];
	char line3[20];
	char line4[20];

	// assemble messages (uncomment one)

	/*All parameters*/
	/*
	sprintf(line1,"I1 %3dI2 %3dI3 %3d", (int) (hall1.current + 0.5 ), (int) (hall2.current + 0.5), (int) (hall3.current + 0.5));
	sprintf(line2,"%4d RPM", (int) (rpm + 0.5));
	sprintf(line3,"%.1f", hgps.speed);
	*/

	/*GPS test*/
	/*
	sprintf(line1,"Lat:%f",hgps.latitude);
	sprintf(line2,"Lon:%f",hgps.longitude);
	sprintf(line3,"Speed:%f",hgps.speed);
	sprintf(line4,"fix type: %d",hgps.fix_mode);
	*/

	/*Internal ADC test*/
	sprintf(line1,"V0:%f",volt0);
	sprintf(line2,"V1:%f",volt1);
	sprintf(line3,"V2:%f",volt2);
	sprintf(line4,"V3:%f",volt3);


	// update LCD
	pcf8574_clr();
	pcf8574_cursor(0, 0);
	pcf8574_send_string(line1);
	pcf8574_cursor(1, 0);
	pcf8574_send_string(line2);
	pcf8574_cursor(2, 0);
	pcf8574_send_string(line3);
	pcf8574_cursor(3, 0);
	pcf8574_send_string(line4);
}


/**
 * @brief This function reads current from each hall effect current sensor.
 *
 * It uses an ADS1115 external ADC. All 3 Vref outputs are connected
 * to AIN3, and all Vout outputs are read relative to that
 * */
void hall_read()
{
	// hall sensor 1
	if(ADS1115_read(ADS1115_MUX_AIN0_AIN3, &hall1.rawVoltage) == HAL_OK)
	{
		//Voltage in mV successfully read.
		hall1.current = (hall1.rawVoltage - hall1.offsetVoltage) * HALL_SENSITIVITY;
	}
	else
	{
		// if current read fails, set current to a conspicuous value
		hall1.current = 123.456789;
	}

	// hall sensor 2
	if(ADS1115_read(ADS1115_MUX_AIN1_AIN3, &hall2.rawVoltage) == HAL_OK)
	{
		//Voltage in mV successfully read.
		hall2.current = (hall2.rawVoltage - hall2.offsetVoltage) * HALL_SENSITIVITY;
	}
	else
	{
		// if current read fails, set current to a conspicuous value
		hall2.current = 123.456789;
	}

	// hall sensor 3
	if(ADS1115_read(ADS1115_MUX_AIN2_AIN3, &hall3.rawVoltage) == HAL_OK)
	{
		//Voltage in mV successfully read.
		hall3.current = (hall3.rawVoltage - hall3.offsetVoltage) * HALL_SENSITIVITY;
	}
	else
	{
		// if current read fails, set current to a conspicuous value
		hall3.current = 123.456789;
	}
}

/**
 * \brief This function zeros all three hall effect sensors
 *  it will block due to internal delay functions
 * */
void hall_calibrate()
{
	if(ADS1115_read(ADS1115_MUX_AIN0_AIN1, &hall1.offsetVoltage) == HAL_OK &&
		ADS1115_read(ADS1115_MUX_AIN0_AIN1, &hall2.offsetVoltage) == HAL_OK &&
		ADS1115_read(ADS1115_MUX_AIN0_AIN1, &hall3.offsetVoltage) == HAL_OK)
	{
		//calibration successful
		LCD_Clear();
		LCD_Puts(0,1,"****** CAL OK ******");
		HAL_Delay(500);
	}
	else
	{
		LCD_Clear();
		LCD_Puts(0,1,"***** CAL FAIL *****");
		HAL_Delay(500);
	}
}


/**
 * \brief Passes received nema string to lwgps. Must be called at least once per second.
 *
 * lwgps (lightweight gps) is a nema string parsing library. see documentation for details)
 * */
void gps_rx()
{
	if(gps.buffStateFlag == read) // if buffer is ready to read
	{

		/* Process all input data */
		lwgps_process(&hgps, gps.buff, gps.tail - 1);

		/* put receiver back in RX state */
		gps.buffStateFlag = write;
		gps.tail = 0;
	}
}

/**
 * \brief Callback for uart RX interrupts. Processes GPS strings
 *
 * Parses incoming serial data one byte at a time using a state machine.
 * Uses state variables defined in gps_struct my_code.h.
 * When it has received the number of lines in one transmission from the sensor,
 * it changes gps.buffStateFlag from write to read.
 *
 * gps_rx() polls for this flag. As long as gps_rx() is called once per second no data is lost.
 *
 * In the future I might add functionality so it discards data and starts over if it has been more
 * than one second since it has seen the last line.
 *
 *  \param[in] huart: uart peripheral that has interrupted. Returns immediately if not huart3
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if( huart == &huart3)
	{
		if(gps.buffStateFlag == write)
		{
			switch (gps.parsingState)
			{
			case init:
				if(gps.newchar == '$')
				{
					gps.buff[gps.tail] = gps.newchar;
					gps.tail++;
					gps.parsingState = seen$;
				}
				break;
			case seen$:
				gps.buff[gps.tail] = gps.newchar;
				gps.tail++;
				if(gps.newchar == '\n')
				{
					gps.lineCounter++;
					if (gps.lineCounter >= gps.numLines)
					{
						// full message complete
						if(1) // check for time TKTKT
						{
							gps.buff[gps.tail + 1] = '0'; // null terminate buffer might not be necessary)
							gps.buffStateFlag = read; // set flag for main program;
							gps.parsingState = init;
							gps.lineCounter = 0;
						}
						else
						{
							//too long since last line, restart
							gps.tail = 0;
							gps.parsingState = init;

						}
					}
				}
				break;
			default : break;
			}
		}
		HAL_UART_Receive_IT(&huart3 , (uint8_t *) & gps.newchar , 1);
	}
}



void lora_TX_blocking(char * payload, int length)
{
	/*
	* format is AT+SEND=<Address>,<Payload Length>,<Data>\r\n
	* ex: AT+SEND=5,5,hello\r\n
	* brackets are not sent
	*/

	char message[128];

	sprintf(message, "AT+SEND=%d,%d,%s\r\n",LORA_RECIVER_ADDRESS,length,payload);

	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	// TX takes some time, can be calculated. Also should read back in the "+OK" response.

}


/**
 * \brief Transmits a string over LoRa using interrupts */
void lora_tx_it(char * payload , int length)
{
	/*
	* message format is "AT+SEND=<Address>,<Payload Length>,<Data>\r\n"
	* ex: "AT+SEND=5,5,hello\r\n"
	* (brackets are not sent)
	* Response received on success is "+OK\r\n"
	*/



		/*
		 * generate message string
		 * Make sure that it is saved to a global variable that won't go out of scope
		 * (I lost sleep over this)
		*/
		sprintf(lora_payload,"AT+SEND=%d,%d,%s\r\n",LORA_RECIVER_ADDRESS,length,payload);

		HAL_UART_Transmit_IT( &huart1 , (uint8_t *) lora_payload , strlen(lora_payload));

		/*
		 * TX takes some time, which can be calculated in theory
		 * If another message is sent too soon, the radio will ignore it.
		 * Alternatively, the program could read back in the "+OK" response
		 * and only send another message after that.
		 * (this is not currently implemented)
		 */
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
	if(Is_First_Captured == 0){
		IC_Val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		Is_First_Captured = 1;
	}
	else {
		IC_Val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

		if (IC_Val2 > IC_Val1){
			Difference = IC_Val2-IC_Val1;
		}
		else if (IC_Val1 > IC_Val2){
			Difference = (0xffffffff - IC_Val1) + IC_Val2;
		}

		float refClock = TIMCLOCK/(PRESCALAR);

		Frequency = refClock/Difference;
		rpm = Frequency * 60.0;

		__HAL_TIM_SET_COUNTER(htim, 0);
		Is_First_Captured = 0;
	}
  }
}


/**
 * \brief reads all voltages from internal ADC.
 * */
void readVoltage(float div){

	  /* Poll for voltage divider*/
	HAL_StatusTypeDef retval;
	ADC_Disable_channels();

	ADC_Select_CH0();
	HAL_ADC_Start(&hadc);
	retval = HAL_ADC_PollForConversion(&hadc,1000);
	readValue = HAL_ADC_GetValue(&hadc);
	volt0 = ((float) readValue * 3.3 / 4095.0) / (float) div ;
	//HAL_Delay (100);
	HAL_ADC_Stop(&hadc);

	ADC_Disable_channels();

	ADC_Select_CH1();
	HAL_ADC_Start(&hadc);
	retval = HAL_ADC_PollForConversion(&hadc,1000);
	readValue = HAL_ADC_GetValue(&hadc);
	volt1 = ((float) readValue * 3.3  / 4095.0) / (float) div ;
	//HAL_Delay (100);
	HAL_ADC_Stop(&hadc);
	ADC_Disable_channels();

	ADC_Select_CH2();
	HAL_ADC_Start(&hadc);
	retval = HAL_ADC_PollForConversion(&hadc,1000);
	readValue = HAL_ADC_GetValue(&hadc);
	volt2 = ((float) readValue * 3.3  / 4095.0) / (float) div ;
	//HAL_Delay (100);
	HAL_ADC_Stop(&hadc);
	ADC_Disable_channels();

	ADC_Select_CH3();
	HAL_ADC_Start(&hadc);
	retval = HAL_ADC_PollForConversion(&hadc,1000);
	readValue = HAL_ADC_GetValue(&hadc);
	volt3 = ((float) readValue * 3.3  / 4095.0) / (float) div ;
	//HAL_Delay (100);
	HAL_ADC_Stop(&hadc);

	Frequency = retval;

}

float readCurrent(void){

	  HAL_ADC_PollForConversion(&hadc,1000);
	  readValue = HAL_ADC_GetValue(&hadc);
	  rawVoltage = (float) readValue * 3.3 * .9104151493 / 4095.0 ;
	   // If rawVoltage is not 2.5Volt, multiply by a factor.In my case it is 1.035
	   // This is due to tolerance in voltage divider resister & ADC accuracy
	  current =(rawVoltage - 2.4988276)/sensitivity;
	  return current;
}

void ADC_Disable_channels(void)
{
	ADC_ChannelConfTypeDef sConfig = {0};

	//disable channel 0
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = ADC_RANK_NONE;
	sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
	Error_Handler();
	}

	//disable channel 1
	sConfig.Channel = ADC_CHANNEL_1;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
	Error_Handler();
	}

	//disable channel 2
	sConfig.Channel = ADC_CHANNEL_2;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
	Error_Handler();
	}

	//disable channel 3
	sConfig.Channel = ADC_CHANNEL_3;
	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{
	Error_Handler();
	}


}

/**
 * \brief configures the internal ADC to use input channel 1
 * */
void ADC_Select_CH1 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.

	  sConfig.Channel =ADC_CHANNEL_1;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

/**
 * \brief configures the internal ADC to use input channel 2
 * */
void ADC_Select_CH2 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.

	  sConfig.Channel = ADC_CHANNEL_2;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

/**
 * \brief configures the internal ADC to use input channel 3
 * */
void ADC_Select_CH3 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.

	  sConfig.Channel = ADC_CHANNEL_3;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

/**
 * \brief configures the internal ADC to use input channel 0
 * */
void ADC_Select_CH0 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.

	  sConfig.Channel = ADC_CHANNEL_0;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime =  ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}
