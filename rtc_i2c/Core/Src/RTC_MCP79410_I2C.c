//---------------------------------------------------------------------------------------------------------------
/*!
 * @File      	    RTC_MCP79410.c
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


/* includes---------------------------------------------------------------------------------------------*/
#include "main.h"

#include "stm32wbxx_hal.h"
#include "RTC_MCP79410_I2C.h"


/* global variables-------------------------------------------------------------------------------------*/
unsigned char gToggleSec=0;
union SecReg gPreSec;
union SecReg gSec;
union MinReg gMin;
union HourReg gHour;
union DayReg gDay;
union DateReg gDate;
union MonthReg gMonth;
union YearReg gYear;
uint8_t gTxBuff[2] = {'\0'}; //To store transmit data
uint8_t gRxBuff[2] = {'\0'}; //To store receive data

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for write RTC byte.
 *  @note        none
 *  @param       byteAdress,rtcbyte
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void WriteRtcByte(uint16_t byteAddress, uint16_t rtcByte)
{
	gTxBuff[0] = byteAddress;  //Store register address
	gTxBuff[1] = rtcByte;      //Store data

	I2C_TRANSMIT2;             //Transmit data

}


/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for read RTC byte.
 *  @note        none
 *  @param       byteAdress
 *  @return      return received data(gRxBuff[0])
 -----------------------------------------------------------------------------------------------------*/
uint8_t ReadRtcByte(uint8_t byteAddress)
{
	gTxBuff[0] = byteAddress;  //Store register address

	I2C_TRANSMIT ;              //Transmit data

	I2C_RECEIVE;               //Receive data

	return gRxBuff[0];
}


/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for initialize RTC.
 *  @note        none
 *  @param       none
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
//void InitRtc(void)
//{
//	gDay.bit.vbaten = 1;
//
//	gSec.bit.st			  = 1;	 	// Enable oscilltor (7th bit of RTCSEC register)
//	gSec.bit.second10 = gatewayParaStruct.timeArray[6];   	// Assign 10's digit of 'Second'(6th to 4th bit of RTCSEC register)
//	gSec.bit.second   = gatewayParaStruct.timeArray[7];   	// Assign 1's digit of 'Second' (3th to 0th bit of RTCSEC register)
//
//	gMin.bit.minute10 = gatewayParaStruct.timeArray[3];   	// Assign 10's digit of 'Minute'(6th to 4th bit of RTCMIN register)
//	gMin.bit.minute   = gatewayParaStruct.timeArray[4];   	// Assign 1's digit of 'Minute'(3th to 0th bit of RTCMIN register)
//
//	gHour.bit24H.mode = 0;   	// Enable 12-hour format (6th bit of RTCHOUR register is '1' for 12-hour format or '0' for 24-hour format)
//	//gHour.bit24H.ampm = 1;   	// AM/PM Indicator bit (5th bit of RTCHOUR register is '1' for PM or '0' for AM)
//
//	gHour.bit24H.hour10 = gatewayParaStruct.timeArray[0]; 	// Assign 10's digit of 'Hour'(4th bit of RTCHOUR register)
//	gHour.bit24H.hour   = gatewayParaStruct.timeArray[1]; 	// Assign 1's digit of 'Hour'(3th to 0th bit of RTCHOUR register)
//
//	gDate.bit.date10 = gatewayParaStruct.dateArray[0];     // Assign 10's digit of 'date'(5th & 4th bit of RTCDATE register)
// 	gDate.bit.date   = gatewayParaStruct.dateArray[1];     // Assign 1's digit of 'date'(3th to 0th bit of RTCDATE register)
//
//	gMonth.bit.month10 = gatewayParaStruct.dateArray[3];   // Assign 10's digit of 'month'(5th & 4th bit of RTCMTH register)
//	gMonth.bit.month   = gatewayParaStruct.dateArray[4];   // Assign 1's digit of 'month'(3th to 0th bit of RTCMTH register)
//
//	gYear.bit.year10 = gatewayParaStruct.dateArray[8];     // Assign 10's digit of 'year'(7th to 4th bit of RTCYEAR register)
//	gYear.bit.year   = gatewayParaStruct.dateArray[9];     // Assign 1's digit of 'year'(3th to 0th bit of RTCYEAR register)
//
//	WriteRtcByte(HOUR_ADDRESS, gHour.all);    //Write hour in RTC
//	WriteRtcByte(MIN_ADDRESS, gMin.all);      //Write minute in RTC
// 	WriteRtcByte(SEC_ADDRESS, gSec.all);		  //Write second in RTC
//	WriteRtcByte(DATE_ADDRESS, gDate.all);    //Write date in RTC
//	WriteRtcByte(MONTH_ADDRESS, gMonth.all);  //Write month in RTC
//	WriteRtcByte(YEAR_ADDRESS, gYear.all);    //Write year in RTC
//	WriteRtcByte(DAY_ADDRESS, gDay.all);    //Write year in RTC
//
//}
/**
 * Initialize the RTC with hardcoded date and time values
 */
void InitRtc(void)
{
    // Enable battery backup
    gDay.bit.vbaten = 1;

    // Set time: 14:30:45 (2:30:45 PM)
    gSec.bit.st = 1;         // Enable oscillator
    gSec.bit.second10 = 4;   // 10's digit of seconds (4)
    gSec.bit.second = 5;     // 1's digit of seconds (5)

    gMin.bit.minute10 = 3;   // 10's digit of minutes (3)
    gMin.bit.minute = 0;     // 1's digit of minutes (0)

    gHour.bit24H.mode = 0;   // Enable 12-hour format
    //gHour.bit24H.ampm = 1; // AM/PM Indicator (1 for PM, 0 for AM)
    gHour.bit24H.hour10 = 1; // 10's digit of hours (1)
    gHour.bit24H.hour = 4;   // 1's digit of hours (4)

    // Set date: 19/05/2025 (May 19, 2025)
    gDate.bit.date10 = 1;    // 10's digit of date (1)
    gDate.bit.date = 9;      // 1's digit of date (9)

    gMonth.bit.month10 = 0;  // 10's digit of month (0)
    gMonth.bit.month = 5;    // 1's digit of month (5)

    gYear.bit.year10 = 2;    // 10's digit of year (2)
    gYear.bit.year = 5;      // 1's digit of year (5)

    // Write values to RTC registers
    WriteRtcByte(HOUR_ADDRESS, gHour.all);     // Write hour in RTC
    WriteRtcByte(MIN_ADDRESS, gMin.all);       // Write minute in RTC
    WriteRtcByte(SEC_ADDRESS, gSec.all);       // Write second in RTC
    WriteRtcByte(DATE_ADDRESS, gDate.all);     // Write date in RTC
    WriteRtcByte(MONTH_ADDRESS, gMonth.all);   // Write month in RTC
    WriteRtcByte(YEAR_ADDRESS, gYear.all);     // Write year in RTC
    WriteRtcByte(DAY_ADDRESS, gDay.all);       // Write day in RTC
}

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for get time.(hour:minute:second)
 *  @note        none
 *  @param       char array[] (for store time value)
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void GetTimeToRtcHmsAscii(char* tempTimeArray)
{
		gHour.all = ReadRtcByte(HOUR_ADDRESS);             //Read hour
		gMin.all 	= ReadRtcByte(MIN_ADDRESS);              //Read minute
		gSec.all	= ReadRtcByte(SEC_ADDRESS);              //Read second

		tempTimeArray[0]	=	gHour.bit24H.hour10 + 0x30;	   //Convert 10's value of Hour in ASCII
		tempTimeArray[1]	=	gHour.bit24H.hour	+	0x30;      //Convert 1's value of Hour in ASCII

		if(gPreSec.all != gSec.all)                        //Compare previous second and actual second for toggling(:)
		{
			if(gToggleSec == 1)
			{
				tempTimeArray[2]	=	':';
				gToggleSec=0;
			}
			else
			{
				tempTimeArray[2]	=	' ';
				gToggleSec=1;
			}
		}
		else
		{
			if(gToggleSec == 1)
				tempTimeArray[2]	=	':';
			else
				tempTimeArray[2]	=	' ';
		}
		gPreSec.all = gSec.all;

		tempTimeArray[3]	=	gMin.bit.minute10	+	0x30;     //Convert 10's value of Minute in ASCII
		tempTimeArray[4]	=	gMin.bit.minute	+	0x30;       //Convert 1's value of Minute in ASCII
		tempTimeArray[5]	=	':';

		tempTimeArray[6]	=	gSec.bit.second10	+	0x30;     //Convert 10's value of Second in ASCII
		tempTimeArray[7]	=	gSec.bit.second	+	0x30;       //Convert 1's value of Second in ASCII

		tempTimeArray[8]	=	'\0';
}


/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for get date. (date/month/year)
 *  @note        none
 *  @param       char array[] (for store date value)
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void GetDateToRtcHmsAscii(char* tempDateArray)
{
	gDate.all	=	ReadRtcByte(DATE_ADDRESS);           //Read date
	gMonth.all = ReadRtcByte(MONTH_ADDRESS);         //Read month
	gYear.all	=	ReadRtcByte(YEAR_ADDRESS);           //Read year

	tempDateArray[0]	=	gDate.bit.date10 + 0x30;	   //Convert 10's value of Date in ASCII
	tempDateArray[1]	=	gDate.bit.date	+	0x30;      //Convert 1's value of Date in ASCII
	tempDateArray[2]	=	'/';

	tempDateArray[3]	=	gMonth.bit.month10 + 0x30;   //Convert 10's value of Month in ASCII
	tempDateArray[4]	=	gMonth.bit.month + 0x30;     //Convert 1's value of Month in ASCII
	tempDateArray[5]	=	'/';

	tempDateArray[6]	=	2	+	0x30;
	tempDateArray[7]	=	0	+	0x30;
	tempDateArray[8]	=	gYear.bit.year10	+	0x30;    //Convert 10's value of Year in ASCII
	tempDateArray[9]	=	gYear.bit.year	+	0x30;      //Convert 1's value of Year in ASCII

	tempDateArray[10]	=	' ';
	tempDateArray[11]	=	'\0';
}

