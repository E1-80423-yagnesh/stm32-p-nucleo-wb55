//---------------------------------------------------------------------------------------------------------------
/*!
 * @File          	EEPROM_LIB.h
 *
 * @Copyright    		(c)	2023 Aartronix Innovation Private Limited(AIPL). All rights resenved. This sotware
 *		      				constitutes the trade secrets and confudential and proprierty information of AIPL.
 *		      				It is intended solely for use by AIPL.This code not be copied or redistributed to
 *                	third parties without prior written authorization from AIPL.
 *
 * @Discription     STM32F410RBt6 interface with EEPROM using I2C.
 *
 * @Revision 				History
 *
 * Date             Author              Project           Reason for change
 * -------------------------------------------------------------------------------------------------------------
 * 29/04/2020       Kalpesh Gajera      POC               Created
 * 01/05/2024       Shyam Parmar        POC               for standard format
 */
//---------------------------------------------------------------------------------------------------------------

/*include-----------------------------------------------------------------------------------*/
#include "main.h"


/*define-----------------------------------------------------------------------------------*/
#ifndef __EEPROM_LIB_H__
#define __EEPROM_LIB_H__

#define I2C_NM      &hi2c2

#define TIMEOUT1    100             //Timeout duration
#define TX_B        2               //Size of transmit byte(2 byte)
#define TX_B2       1               //Size of transmit byte(1 byte)
#define TX_B3       (noByte + 1)    //No of byte + 1

#define RX_B        1					      //Size of receive byte(1 byte)
#define RX_B2       noByte          //No of byte


#define EEPROM_WRITEID       0xA0   //EEPROM write address
#define EEPROM_READID        0xA1   //EEPROM read address
#define EEPROM_TOTAL_BYTE    11     //Total byte

#define I2C_TRANSMIT   HAL_I2C_Master_Transmit(I2C_NM,EEPROM_WRITEID,gTxB,TX_B,TIMEOUT1);              //Transmit data
#define I2C_TRANSMIT2  HAL_I2C_Master_Transmit(I2C_NM,EEPROM_WRITEID,gTxB,TX_B2,TIMEOUT1);             //Transmit2 data
#define I2C_TRANSMIT3  HAL_I2C_Master_Transmit(I2C_NM,EEPROM_WRITEID,gTxB,TX_B3,TIMEOUT1);             //Transmit3 data

#define I2C_RECEIVE    HAL_I2C_Master_Receive(I2C_NM,EEPROM_READID,gRxB,RX_B,TIMEOUT1);                //Receive data
#define I2C_RECEIVE2   HAL_I2C_Master_Receive(I2C_NM,EEPROM_READID,eepromBytePointer,RX_B2,TIMEOUT1);  //Receive2 data


/*global variables-------------------------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c2;



extern char serverIPstr[20];
extern char serverPortStr[6];
extern char refreshTime1Str[6];
extern char refreshTime2Str[6];
extern char thresholdStr[6];

extern char READIP[16];
extern char READPORT[5];
extern char time1[3];
extern char time2[3];
extern char readthreshold[4];

/*type define------------------------------------------------------------------------------*/
typedef struct
{
	uint8_t writeId[10];                 //WriteID
	uint8_t readId1[10];                 //ReadID 1
	uint8_t startingMemoryLocation;      //Starting memory location
	uint8_t endingMemoryLocation;        //Ending memory location
}EEPROM_ID;

extern EEPROM_ID eepromId;

/*function prototypes---------------------------------------------------------------------*/
extern void WriteEepromByte(uint8_t byteAddress, uint8_t eepromByte);
extern uint8_t ReadEepromByte(uint8_t byteAddress);
extern void WriteEepromMultiByte(uint8_t byteAddress, uint8_t* eepromBytePointer, uint8_t noByte);
extern void ReadEepromMultiByte(uint8_t byteAddress, uint8_t* eepromBytePointer, uint8_t noByte);

void Config_Init(void);
void Test_StringEEPROM(void);
int Load_Config_From_EEPROM(void);



#endif /* __EEPROM_LIB_H__ */

