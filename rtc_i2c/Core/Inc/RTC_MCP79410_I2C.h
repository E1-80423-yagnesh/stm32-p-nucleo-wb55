//---------------------------------------------------------------------------------------------------------------
/*!
 * @File      	    RTC_MCP79410.h
 *
 * @Copyright    		(c)	2023 Aartronix Innovation Private Limited(AIPL). All rights resenved. This sotware
 *		      				constitutes the trade secrets and confudential and proprierty information of AIPL.
 *		      				It is intended solely for use by AIPL.This code not be copied or redistributed to
 *                	third parties without prior written authorization from AIPL.
 *
 * @Discription     This file use for configure and initialize the RTC. also use for read and write RTC.
 *
 * @Revision 				History
 *
 * Date             Author              Project           Reason for change
 * -------------------------------------------------------------------------------------------------------------
 * 29/04/2020       Kalpesh Gajera      POC               Created
 * 23/04/2024       Shyam Parmar        POC               for standard format
 */
//---------------------------------------------------------------------------------------------------------------


/* includes--------------------------------------------------------------------------------------*/
#include "main.h"
#include "stm32wbxx_hal.h"

/* defines---------------------------------------------------------------------------------------*/
#ifndef __RTC_MCP79410_H__
#define __RTC_MCP79410_H__
/* If you move this file to a different CPU, make sure to check the I2C number, SCL and SDA pins and port, transmit and receive bytes and functions*/

#define I2C_NM         &hi2c1

#define RTC_SCL_PIN    GPIO_PIN_8  //Gpio pin 8 (scl)
#define RTC_SCL_PORT   GPIOB        //Gpio port

#define RTC_SDA_PIN    GPIO_PIN_9  //Gpio pin 9 (sda)
#define RTC_SDA_PORT   GPIOB        //Gpio port

#define TIMEOUT        100         //Timeout duration
#define TX_BYTE        1           //Size of transmit byte(1 byte)
#define TX_BYTE2       2           //Size of transmit byte(2 byte)
#define RX_BYTE        1					 //Size of receive byte(1 byte)

#define I2C_TRANSMIT   HAL_I2C_Master_Transmit(I2C_NM,RTC_WRITEID,gTxBuff,TX_BYTE,TIMEOUT);   //Transmit data
#define I2C_TRANSMIT2  HAL_I2C_Master_Transmit(I2C_NM,RTC_WRITEID,gTxBuff,TX_BYTE2,TIMEOUT);  //Transmit data
#define I2C_RECEIVE    HAL_I2C_Master_Receive(I2C_NM,RTC_READID,gRxBuff,RX_BYTE,TIMEOUT);     //Receive data

#define RTC_WRITEID	  0xde   //RTC write address
#define RTC_READID	  0xdf	 //RTC read address

#define SEC_ADDRESS    0     //Second address
#define MIN_ADDRESS    1     //Minute address
#define HOUR_ADDRESS   2     //Hour address
#define DAY_ADDRESS    3     //Day address
#define DATE_ADDRESS   4     //Date address
#define MONTH_ADDRESS  5     //Month address
#define YEAR_ADDRESS   6     //Year address
#define HOUR_12	       1     //Hour-12hr
#define HOUR_24	       0     //Hour-24hr
#define AM		         0     //AM
#define PM		         1     //PM


/*global variables------------------------------------------------------------------------------*/
extern union SecReg gSec;
extern union MinReg gMin;
extern union HourReg gHour;
extern union DayReg gDay;
extern union DateReg gDate;
extern union MonthReg gMonth;
extern union YearReg gYear;

/*type defines----------------------------------------------------------------------------- -----*/
typedef struct
{
	uint16_t 	second:4;		  //second lower digit
	uint16_t	second10:3;	  //second higher digit
	uint16_t	st:1;			    //rtc start bit
	uint16_t	reserved:8;	  //reserved
}SEC_REG_BITS;

union SecReg
{
	uint16_t		all;
	SEC_REG_BITS  	bit;
};

typedef struct
{
	uint16_t 	minute:4;		  //minute lower digit
	uint16_t	minute10:3;		//minute higher digit
	uint16_t	reserved:9;		//reserved
}MIN_REG_BITS;

union MinReg
{
	uint16_t		all;
	MIN_REG_BITS  	bit;
};

typedef struct
{
	uint16_t 	hour:4;		   //hour lower digit
	uint16_t	hour10:1;	   //hour higher digit
	uint16_t	ampm:1;	     //am/pm bit
	uint16_t	mode:1;		   //12/24 hour mode
	uint16_t	reserved:9;	 //reserved
}HOUR12_REG_BITS;

typedef struct
{
	uint16_t 	hour:4;		   //hour lower digit
	uint16_t	hour10:2;	   //hour higher digit
	uint16_t	mode:1;		   //12/24 hour mode
	uint16_t	reserved:9;	 //reserved
}HOUR24_REG_BITS;

union HourReg
{
	uint16_t		all;
	HOUR12_REG_BITS bit12H;
	HOUR24_REG_BITS	bit24H;
};

typedef struct
{
	uint16_t 	day:3;			 //day lower digit
	uint16_t	vbaten:1;		 //external battery enable bit
	uint16_t	pwrfail:1;	 //enabled by rtc hardware when power fails and rtc is powered by external battery
	uint16_t	oscon:1;		 //rtc oscillator status bit
	uint16_t	reserved:10; //reserved
}DAY_REG_BITS;

union DayReg
{
	uint16_t		all;
	DAY_REG_BITS  	bit;
};

typedef struct
{
	uint16_t 	date:4;			 //date lower digit
	uint16_t	date10:2;		 //date higher digit
	uint16_t	reserved:10; //reserved
}DATE_REG_BITS;

union DateReg
{
	uint16_t		all;
	DATE_REG_BITS  	bit;
};

typedef struct
{
	uint16_t 	month:4;		 //month lower digit
	uint16_t	month10:1;	 //month higher digit
	uint16_t	lp:1;
	uint16_t	reserved:10; //reserved
}MONTH_REG_BITS;

union MonthReg
{
	uint16_t			all;
	MONTH_REG_BITS  	bit;
};

typedef struct
{
	uint16_t 	year:4;			// year lower digit
	uint16_t	year10:4;		// year higher digit
	uint16_t	reserved:8;	// reserved
}YEAR_REG_BITS;

union YearReg
{
	uint16_t		all;
	YEAR_REG_BITS  	bit;
};

/* Function prototypes ----------------------------------------------------------------*/
void InitRtc(void);
void WriteRtcByte(uint16_t byteAddress, uint16_t rtcByte);
uint8_t ReadRtcByte(uint8_t byteAddress);
void GetTimeToRtcHmsAscii(char* tempTimeArray);
void GetDateToRtcHmsAscii(char* tempDateArray);

#endif /* __RTC_MCP79410_H__ */


