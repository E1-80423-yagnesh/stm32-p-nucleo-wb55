//---------------------------------------------------------------------------------------------------------------
/*!
 * @File Name         	modbus.h
 *
 * @Copyright(c)	      2023 Aartronix Innovation Private Limited(AIPL). All rights resenved. This sotware
 *		      	          constitutes the trade secrets and confudential and proprierty information of AIPL.
 *		      	          It is intended solely for use by AIPL.This code not be copied or redistributed to
 *                    	third parties without prior written authorization from AIPL.
 *
 * @Discription       	This file is use for receive query from Modbus master and process it and also generate proper responce for modbus master.
 *
 * @Revision 						History
 *
 * Date             Author              Project           Reason for change
 * -------------------------------------------------------------------------------------------------------------
 * 13-03-2019     Kalpesh Gajera        POC               Created
 * 19-04-2024     Miral Rathod          POC               For standard Formet
 */
//---------------------------------------------------------------------------------------------------------------


#ifndef SERIAL_COMMUNICATION_H_
#define SERIAL_COMMUNICATION_H_

/* Define -------------------------------------------------------------------*/
/* If you move this file to a different CPU, make sure to check the UART number,enable pin.*/
/*if you use STM cpu and another UART number,change the RX & TX pin in STM32FXXXX_hal_msp.c.*/

#define RS485_EN_PIN GPIO_PIN_6
#define RS485_EN_PORT GPIOB

#define SLAVE_ID        1
#define NO_OF_REG       5
#define READ_REGISTERS  3

#define MBUS_SLAVE_ADDRESS_PTR          0  //Position 0 in frame buffer contains slave address (1st byte of any Modbus frame).
#define MBUS_FUNCTION_PTR               1  //Position 1 contains function code (2nd byte of any Modbus frame).
#define MBUS_START_ADDRESS_LO_PTR       3  //These point to the same position (3) - the low byte of the starting register address.
#define MBUS_REGISTER_ADDRESS_LO_PTR    3  // The dual naming allows semantic clarity when handling different functions.
#define MBUS_NO_OF_POINTS_HI_PTR        4  //Positions 4  hold the high  byte of the "number of points" field (quantity of registers).
#define MBUS_NO_OF_POINTS_LO_PTR        5  //Positions 5  hold the low  byte of the "number of points" field (quantity of registers).
#define MBUS_PRESET_DATA_HI_PTR         4   //Position 4 contains the high byte of data in a preset (write) command.
#define MBUS_BYTE_COUNT_PTR             6   //Position 6 holds the byte count field in responses
#define MBUS_FRAME_BUFFER_SIZE          200 //Sets the maximum Modbus data payload to 200 bytes

/* type define-------------------------------------------------------------------------*/
typedef enum
{
	readHoldingRegister = 3,
	writeSingleHoldingRegister = 6,
	writeMultipleHoldingRegister = 16

}MODBUS_FUNCTION;

typedef union
{
	unsigned int  all;
	struct  MODBUS_BITS
	{
		unsigned int  error:1;

	}bitValue;
}MODBUS_FLAG;

typedef union
{
	unsigned int  all;
	struct  MODBUS_SLAVE_BITS
	{
			unsigned int  error:1;

	}bitValue;
}MODBUS_SLAVE_FLAG;

typedef struct
{
	unsigned int  modbusHoldingRegister[MBUS_FRAME_BUFFER_SIZE>>1]; //stores holding register values >>1(x2) as 16 byte register
	unsigned char modbusFrame[MBUS_FRAME_BUFFER_SIZE+5];
	/* buffer that holds the raw Modbus frame. The "+5" adds extra bytes for:1)Slave address (1 byte) 2) Function code (1 byte)  3) CRC (2 bytes) 4)extra byte for padding or alignment*/
	unsigned int  rxPointer; //Tracks the current position in the receive buffer during message reception.
	unsigned int  totalTxValue; //Count bytes transmitted
	unsigned int  totalRxValue; //Count bytes  received

	MODBUS_SLAVE_FLAG   flags; //Contains status flags for the Modbus slave operation.
	MODBUS_FUNCTION function;  //Stores the current function code being processed.

	unsigned int  address;    //Holds the device's Modbus address

}MODBUS_SLAVE_DATA;

/* Global variables ---------------------------------------------------------*/
extern MODBUS_SLAVE_DATA   gModbusSlaveData;

/* Function prototypes ------------------------------------------------------*/
void ModbusSlaveProcessReceivedQuery(void);
void CalculateCrcSlave(unsigned char crcreg);

#endif /*SERIAL_COMMUNICATION_H_*/
