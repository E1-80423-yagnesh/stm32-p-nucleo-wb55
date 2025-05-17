//---------------------------------------------------------------------------------------------------------------
/*!
 * @File Name         	modbus.c
 *
 * @Copyright(c)	      2023 Aartronix Innovation Private Limited(AIPL). All rights resenved. This sotware
 *		      	          constitutes the trade secrets and confudential and proprierty information of AIPL.
 *		      	          It is intended solely for use by AIPL.This code not be copied or redistributed to
 *                    	third parties without prior written authorization from AIPL.
 *
 * @Discription       	This file is use for receive query from Modbus master and process it and also generate proper responce for modbus master.
 *
 * @Revision            History
 *
 * Date             Author              Project           Reason for change
 * -------------------------------------------------------------------------------------------------------------
 * 13-03-2019     Kalpesh Gajera        POC               Created
 * 19-04-2024     Miral Rathod          POC               For standard Formet
 */
//---------------------------------------------------------------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "modbus.h"
#include "main.h"

/* Global variables ---------------------------------------------------------*/
volatile unsigned int gCrcModbusSlave;        //crc of slave
MODBUS_SLAVE_DATA     gModbusSlaveData;       //data of slave

//---------------------------------------------------------------------------------------------------------------
/**
  * @brief  Modbus Slave Process on received query from Master
  * @param  None
	* @note   None
  * @return None
  */
//---------------------------------------------------------------------------------------------------------------
// for example this is [0x01] [0x03] [0x00] [0x10] [0x00] [0x02] [CRC_L] [CRC_H]
/*0x01 – Slave Address

0x03 – Function Code → Read Holding Registers

0x0010 – Start Address

0x0002 – Quantity of registers

CRC – 2 bytes*/



void ModbusSlaveProcessReceivedQuery(void)
{
	unsigned int i, j, k;

	gCrcModbusSlave = 0xFFFF;
    //calculate CRC of all bytes except last two
	for(i=0; i<gModbusSlaveData.totalRxValue-2; i++)
	{
		CalculateCrcSlave(gModbusSlaveData.modbusFrame[i]);
	}

    //after calculating CRC it is checked against low byte and high byte if it doesnt matched raise error
	if(gModbusSlaveData.modbusFrame[i++] !=  (gCrcModbusSlave & 0xFF))
	{
		gModbusSlaveData.flags.bitValue.error = 1;
		return;
	}

	if(gModbusSlaveData.modbusFrame[i] !=  (gCrcModbusSlave>>8))
	{
		gModbusSlaveData.flags.bitValue.error = 1;
		return;
	}

	//If the message isn't addressed to this slave, ignore it.
	if(gModbusSlaveData.modbusFrame[MBUS_SLAVE_ADDRESS_PTR] != gModbusSlaveData.address)
	{
		return;
	}

	gModbusSlaveData.totalTxValue = 0; //transmit counter set to zero
	switch(gModbusSlaveData.modbusFrame[MBUS_FUNCTION_PTR])   //this is equivalent to switch(gModbusSlaveData.modbusFrame[1])  // from modbus frame access Function code byte

	{
		case readHoldingRegister:
			//check if requested registers are in buffer limit and <<1(x2) as 16bit registers
			if(((gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1) + (gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1)) < MBUS_FRAME_BUFFER_SIZE)
			{
				if(((gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1) + (gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1)))
				{   //Saves the starting register address
					j = (gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1);
					//Starts the response with 2 bytes (slave address(byte 0) and function code(byte 1)) already in place
					gModbusSlaveData.totalTxValue = 2;
					//Adds the byte count to the response
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1;
                    //Copies the requested register values to the response buffer.
					for(i=j; i<(gModbusSlaveData.modbusFrame[2]+j); i++)
					{
						gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gModbusSlaveData.modbusHoldingRegister[i];
					}
				}
				//Error handling functioncode+0x80 for exception , Exception code 0x02 (illegal data address)
				else
				{
					gModbusSlaveData.totalTxValue = 1;
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =   0x02;
				}
			}
			else
			{
				gModbusSlaveData.totalTxValue = 1;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =   0x02;
			}
		break;

		case writeSingleHoldingRegister:
			if((gModbusSlaveData.modbusFrame[MBUS_REGISTER_ADDRESS_LO_PTR]<<1) < MBUS_FRAME_BUFFER_SIZE)
			{
				if(((gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1) + (gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1)))
				{   //Writes the high and low bytes of the data to the holding register
					i = gModbusSlaveData.modbusFrame[MBUS_REGISTER_ADDRESS_LO_PTR]<<1;
					gModbusSlaveData.modbusHoldingRegister[i++]  = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_HI_PTR];
					gModbusSlaveData.modbusHoldingRegister[i]    = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR];
					gModbusSlaveData.totalTxValue = 6;
				}
				else
				{   //Error handling functioncode+0x80 for exception , Exception code 0x02 (illegal data address)
					gModbusSlaveData.totalTxValue = 1;
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x02;
				}
			}
			else
			{   // j is the starting register (in bytes) and k points to the first data byte
				gModbusSlaveData.totalTxValue = 1;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x02;
			}
		break;


		case writeMultipleHoldingRegister:
			if((gModbusSlaveData.modbusFrame[MBUS_REGISTER_ADDRESS_LO_PTR]<<1) < MBUS_FRAME_BUFFER_SIZE)
			{   // j is the starting register (in bytes) and k points to the first data byte
				j = gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1;
				k = MBUS_BYTE_COUNT_PTR+1;
				//Copies data from the request into holding registers.
				for(i=j; i<(gModbusSlaveData.modbusFrame[MBUS_BYTE_COUNT_PTR]+j); i++)
				{
					gModbusSlaveData.modbusHoldingRegister[i] = gModbusSlaveData.modbusFrame[k++];
				}
					gModbusSlaveData.totalTxValue = 6;
				}
			else////Error handling functioncode+0x80 for exception , Exception code 0x02 (illegal data address)
			{
				gModbusSlaveData.totalTxValue = 1;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x02;
			}
		break;

		default:
			gModbusSlaveData.totalTxValue = 1;
			gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
			gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x01;
		break;
	}

	gCrcModbusSlave = 0xFFFF;
	for(i=0; i<gModbusSlaveData.totalTxValue; i++)
	{
		CalculateCrcSlave(gModbusSlaveData.modbusFrame[i]);
	}
	gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gCrcModbusSlave & 0xFF;
	gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gCrcModbusSlave >> 8;
}
/*---------------------------------------------------------------------------------------------------------------
  *
  * @brief  calculate CRC for modbus protocol communication.
  * @param  crcreg - data for crc generation.
	* @note   None
  * @return None
  *
--------------------------------------------------------------------------------------------------------------- */
void CalculateCrcSlave(unsigned char crcreg)
{
    unsigned char i;
    unsigned int  crcbit;

    crcreg &= 0xFF;
    gCrcModbusSlave  ^= crcreg;
    for(i=0; i<=7; i++)
    {
        crcbit = 0;
        if((gCrcModbusSlave & 0x0001) == 0x0001)
            crcbit = 1;

        gCrcModbusSlave   >>= 1;
        if(crcbit)
            gCrcModbusSlave  ^= 0xa001;
    }
}

/*---------------------------------------------------------------------------------------------------------------
  *
  * @brief   Uart2 Receive Complete callback
  * @param   None
	* @note    Replace RxCompletecallback according to your CPU and write in main.c file
  * @return  None

--------------------------------------------------------------------------------------------------------------- */
/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == huart2.Instance)					// UART2 RX ISR : For 485-Modbus Interface
	{

		__HAL_UART_ENABLE_IT(&huart2,UART_IT_TC);
		__HAL_UART_DISABLE_IT(&huart2,UART_IT_RXNE);

		gModbusSlaveData.modbusFrame[gModbusSlaveData.rxPointer++] = rx_buffer[0];

		if(gModbusSlaveData.rxPointer == 7)
		{
			if(gModbusSlaveData.modbusFrame[1] == writeMultipleHoldingRegister)
			{
				modbusSlaveData.totalRxValue += (gModbusSlaveData.modbusFrame[6]+1);
			}
			else if((gModbusSlaveData.modbusFrame[1] == readHoldingRegister) ||  (modbusSlaveData.modbusFrame[1] == writeSingleHoldingRegisters))
			{
				gModbusSlaveData.totalRxValue = 8;
			}
			else
			{
				for(uint8_t Index=0; Index<=gModbusSlaveData.rxPointer; Index++)
				{
					gModbusSlaveData.modbusFrame[Index] = '\0';
				}
				gModbusSlaveData.rxPointer=0;
			}
		}
		if(gModbusSlaveData.rxPointer == gModbusSlaveData.totalRxValue)
		{
			if(gModbusSlaveData.modbusFrame[1] == writeMultipleHoldingRegister || gModbusSlaveData.modbusFrame[1] == readHoldingRegister ||
			   gModbusSlaveData.modbusFrame[1] == writeSingleHoldingRegisters )
			{
				ModbusSlaveProcessReceivedQuery();
        TempCounter2++;
				HAL_UART_Transmit_IT(&huart2,(uint8_t*)gModbusSlaveData.modbusFrame,gModbusSlaveData.TotalTxValue);

			}

			gModbusSlaveData.rxPointer = 0;
		}
		Tempcounter=0;
		HAL_UART_Receive_IT(&huart2,(uint8_t*)rx_buffer,1);
	}
}*/
