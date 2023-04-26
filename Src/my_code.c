/*
 * 	File Name: my_code.c
 *
 *  Created on: Mar 14, 2023
 *  updated: 4/18/2023
 *  Author: William Winters
 *
 *  Written for senior design as Stony Brook University
 *  for the Stony Brook Solar Racing team
 *
 *  Copyright William Winters 2023
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



/* define global variables */

char message[] = "hello world\n";

/* define GPS stuff */
volatile struct gps_struct gps; // custom struct see my_code.h
lwgps_t hgps; // struct used by GPS parsing library

// for tachometer
float Frequency = 0;
float rpm = 0;

// for hall effect sensors
struct hall_struct hall1, hall2, hall3;

// for internal ADC
uint16_t readValue;

// needed for Vikash's code
float sensitivity;
float rawVoltage;
float current;
float offsetVoltage;

// runs once
void setup()
{
	// gps setup code
	gps.tail = 0;
	gps.buffStateFlag = write;
	gps.numLines = 14;
	lwgps_init(&hgps); // init gps parsing library

	HAL_UART_Receive_IT(&huart2 , (uint8_t *) & gps.newchar , 1); // could use higher size and change parsing
	// end gps setup code

	//external ADC setup code
	//if(ADS1115_Init(&hi2c1, ADS1115_DATA_RATE_64, ADS1115_PGA_FOUR) != HAL_OK){
	//	// init failed
	//	while(1){}
	//}
	//end external ADC setup code



	//init lcd
	LCD_Init();


	//HAL_ADC_Start(&hadc);

	//test_gps_blocking();
	//HAL_UART_Transmit_IT( &huart1 , (uint8_t *) message , 12);
}

/* runs forever */
void loop()
{

	// get gps data
	gps_rx();

	// get current
	//hall_read();

	//get voltage
	//TKTKTK

	//get temperature
	//TKTKT

	//update display
	LCD_paint();

	HAL_Delay(100);

}


/*
 * This function assembles a message and sends it to the display.
 * it will block temporarily due to internal delay functions
 * */
void LCD_paint()
{
	char line1[20];
	char line2[20];
	char line3[20];

	sprintf(line1,"I1 %3dI2 %3dI3 %3d", (int) (hall1.current + 0.5 ), (int) (hall2.current + 0.5), (int) (hall3.current + 0.5));
	sprintf(line2,"%4d RPM", (int) (rpm + 0.5));
	sprintf(line3,"%.1f", hgps.speed);
	LCD_Clear();
	LCD_Puts(0,0,line1);
	LCD_Puts(0,2,line2);
	LCD_Puts(0,3,line3);
}


/*
 * This function reads current from each hall effect current sensor
 * using an ADS1115 external ADC. All 3 Vref outputs are connected
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

/*
 * This function zeros all three hall effect sensors
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

void internal_ADC()
{
	HAL_ADC_Start(&hadc);

	HAL_ADC_PollForConversion(&hadc,1000);
	readValue = HAL_ADC_GetValue(&hadc);


	rawVoltage = (float) readValue * 3.3 * .9104151493 / 4095.0 ;
	// If rawVoltage is not 2.5Volt, multiply by a factor.In my case it is 1.035
	// This is due to tolerance in voltage divider resister & ADC accuracy
	current =(rawVoltage - 2.4988276)/sensitivity;
	HAL_Delay (100);
	//2.48155546
}

void gps_rx()
{
	if(gps.buffStateFlag == read) // if buffer is ready to read
	{

		/* Process all input data */
		lwgps_process(&hgps, gps.buff, gps.tail - 1);
		/* Print messages */
		char message[64];
		sprintf(message, "Valid status: %d\r\n", hgps.is_valid);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		sprintf(message, "Latitude: %f degrees\r\n", hgps.latitude);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		sprintf(message, "Longitude: %f degrees\r\n", hgps.longitude);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		sprintf(message, "Altitude: %f meters\r\n", hgps.altitude);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		sprintf(message, "Speed: %f knots\r\n", hgps.speed);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		sprintf(message, "Fix Mode: %d \r\n", hgps.fix_mode);
		HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
		/* put receiver back in RX state */
		gps.buffStateFlag = write;
		gps.tail = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if( huart == &huart2)
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
		HAL_UART_Receive_IT(&huart2 , (uint8_t *) & gps.newchar , 1);
	}
}



void lora_TX_blocking(char * payload, int length)
{
	/*
	* format is AT+SEND=<Address>,<Payload Length>,<Data>\r\n
	* ex: AT+SEND=5,5,hello\r\n
	* brackets are not sent
	*/

	char message[64];
	int adress = 5;
	sprintf(message, "AT+SEND=%d,%d,%s\r\n",adress,length,payload);

	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	// TX takes some time, can be calculated. Also should read back in the "+OK" response.

}

// just testing the GPS library
void test_gps_blocking()
{

	/* GPS handle */

	lwgps_t hgps;


	/**

	 * \brief           Dummy data from GPS receiver

	 */

	const char gps_rx_data[] = ""

	                           ",,,,,V*71\r\n$GPRMC,183729,A,3907.356,N,12102.482,W,000.0,360.0,080301,015.5,E*6F\r\n"

	                           "$GPRMB,A,,,,,,,,,,,,V*71\r\n"

	                           "$GPGGA,183730,3907.356,N,12102.482,W,1,05,1.6,646.4,M,-24.1,M,,*75\r\n"

	                           "$GPGSA,A,3,02,,,07,,09,24,26,,,,,1.6,1.6,1.0*3D\r\n"

	                           "$GPGSV,2,1,08,02,43,088,38,04,42,145,00,05,11,291,00,07,60,043,35*71\r\n"

	                           "$GPGSV,2,2,08,08,02,145,00,09,46,303,47,24,16,178,32,26,18,231,43*77\r\n"

	                           "$PGRME,22.0,M,52.9,M,51.0,M*14\r\n"

	                           "$GPGLL,3907.360,N,12102.481,W,183730,A*33\r\n"

	                           "$PGRMZ,2062,f,3*2D\r\n"

	                           "$PGRMM,WGS84*06\r\n"

	                           "$GPBOD,,T,,M,,*47\r\n"

	                           "$GPRTE,1,1,c,0*07\r\n"

	                           "$GPRMC,183731,A,3907.482,N,12102.436,W,000.0,360.0,080301,015.5,E*67\r\n"

	                           "$GPRMB,A,,,,,,,,,,,,V*71\r\n";



	const char gps_rx_data2[] = "$GNGGA,030135.000,4054.69060,N,07307.33222,W,1,05,8.2,31.5,M,-32.1,M,,*4C\r\n\
			$GNGLL,4054.69060,N,07307.33222,W,030135.000,A,A*5C\r\n\
			$GNGSA,A,3,11,13,20,30,,,,,,,,,10.4,8.2,6.4,1*0F\r\n\
			$GNGSA,A,3,28,,,,,,,,,,,,10.4,8.2,6.4,4*03\r\n\
			$GPGSV,3,1,10,02,73,317,,05,60,045,,11,22,120,27,13,67,115,23,0*66\r\n\
			$GPGSV,3,2,10,15,60,199,,18,30,309,,20,35,071,28,23,12,265,,0*6D\r\n\
			$GPGSV,3,3,10,29,52,236,,30,16,062,21,0*6C\r\n\
			$BDGSV,1,1,01,28,32,160,25,0*4E\r\n\
			$GNRMC,030135.000,A,4054.69060,N,07307.33222,W,0.00,322.62,220323,,,A,V*14\r\n\
			$GNVTG,322.62,T,,M,0.00,N,0.00,K,A*24\r\n\
			$GNZDA,030135.000,22,03,2023,00,00*4C\r\n\
			$GPTXT,01,01,01,ANTENNA OK*35\r\n" ;



	/* Init GPS */

	lwgps_init(&hgps);


	/* Process all input data */

	lwgps_process(&hgps, gps_rx_data2, strlen(gps_rx_data));


	/* Print messages */
	char message[64];

	sprintf(message, "Valid status: %d\r\n", hgps.is_valid);
	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	sprintf(message, "Latitude: %f degrees\r\n", hgps.latitude);
	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	sprintf(message, "Longitude: %f degrees\r\n", hgps.longitude);
	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	sprintf(message, "Altitude: %f meters\r\n", hgps.altitude);
	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);

	sprintf(message, "Speed: %f \r\n", hgps.speed);
	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);



}

/* Transmits a string over LoRa using interrupts */
void lora_tx_it(char * payload , int length)
{
	/*
	* message format is "AT+SEND=<Address>,<Payload Length>,<Data>\r\n"
	* ex: "AT+SEND=5,5,hello\r\n"
	* (brackets are not sent)
	* Response received on success is "+OK\r\n"
	*/

		char message[64];

		// generate message string
		sprintf(message, "AT+SEND=%d,%d,%s\r\n",LORA_RECIVER_ADDRESS,length,payload);

		HAL_UART_Transmit_IT( &huart2 , (uint8_t *) message , strlen(message));

		/*
		 * TX takes some time, which can be calculated in theory
		 * If another message is sent too soon, the radio will ignore it.
		 * Alternatively, the program could read back in the "+OK" response
		 * and only send another message after that.
		 * (this is not currently implemented)
		 */
}


void debug_print_blocking(char * string)
{

	char message[64];
	sprintf(message, "[%s]\r\n", string);

	HAL_UART_Transmit( &huart1 , (uint8_t *) message , strlen(message) , HAL_MAX_DELAY);
}
