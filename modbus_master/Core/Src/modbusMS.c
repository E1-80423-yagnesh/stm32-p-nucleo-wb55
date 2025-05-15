 /*!----------------------------------------------------------------------------------------------------------
 * @file:   				MODBUS.c

 * @version         3.0

 * @Discription  		This file contains the definitions for MODBUS slave

 *  Date            Author                 Project                        Reason for change
 * ----------------------------------------------------------------------------------------------------------
 * 12/02/2019       Kalpesh Gajera         modbus Master and Slave        Created
 * 19/04/2024       Shyam Parmar           Modbus Master and Slave				firmware Standards
 -----------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ModbusMS.h"
#include "main.h"


/*global variable-------------------------------------------------------------------------------------------*/
volatile uint32_t gCrcModbus;
volatile uint32_t gScanId=1;

uint8_t gRxBuffer[20];
uint8_t gTXCounter;


#ifdef SLAVE
MODBUS_SLAVE_DATA    gModbusSlaveData;         //data of slave
#else
MODBUS_MASTER_DATA	 gModbusMasterData;				//data of master
#endif

/* define---------------------------------------------------------------------------------------------------*/
/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for initialize the modbus master or slave.

 *  @note        none

 *  @param       none

 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void ModbusInit(void)
{
	#ifdef SLAVE
	{
		gModbusSlaveData.address = SLAVE_ID; // Slave address
	  gModbusSlaveData.function = READ_HOLDING_REG;
		gModbusSlaveData.modbusHoldingRegister[0] = 0x02;
		gModbusSlaveData.modbusHoldingRegister[1] = 0x55;
		RECEIVE_QUERY;
	}
	#else
	{
		gModbusMasterData.address = SLAVE_ID; // Slave address
		gModbusMasterData.function = READ_HOLDING_REG;
		gModbusMasterData.startAddress = START_ADDR; // Start address
		gModbusMasterData.totalRegisters = NO_OF_REG;
		TransmitModbusMaster();
		En_SET;
		TRANSMIT_QUERY;
		MASTER_TIMER;
		gTXCounter =0;
	}
	#endif
}


#ifdef SLAVE
/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function for modbus slave rxcllback.

 *  @note        none

 *  @param       none

 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
//void ModbusSlaveRxCpltCallback(void)
//{
//		//enable uart transmiter interrupt
//		//disable uart receiver interrupt
//	gModbusSlaveData.modbusFrame[gModbusSlaveData.rxPointer++] = gRxBuffer[0];
//
//	if(gModbusSlaveData.rxPointer == 7)
//	{
//		if(gModbusSlaveData.modbusFrame[1] == writeMultipleHoldingRegister)
//		{
//			gModbusSlaveData.totalRxValue += (gModbusSlaveData.modbusFrame[6]+1);
//		}
//		else if((gModbusSlaveData.modbusFrame[1] == readHoldingRegisters) ||  (gModbusSlaveData.modbusFrame[1] == writeSingleHoldingRegister))
//		{
//			gModbusSlaveData.totalRxValue = 8;
//		}
//		else
//		{
//			for(uint8_t Index=0; Index<=gModbusSlaveData.rxPointer; Index++)
//			{
//				gModbusSlaveData.modbusFrame[Index] = '\0';
//			}
//			gModbusSlaveData.rxPointer=0;
//		}
//	}
//	if(gModbusSlaveData.rxPointer == gModbusSlaveData.totalRxValue)
//	{
//		if(gModbusSlaveData.modbusFrame[1] == writeMultipleHoldingRegister || gModbusSlaveData.modbusFrame[1] == readHoldingRegisters ||
//			 gModbusSlaveData.modbusFrame[1] == writeSingleHoldingRegister )
//		{
//			ModbusSlaveProcessReceivedQuery();
//			gTXCounter++;
//		}
//		gModbusSlaveData.rxPointer = 0;
//	}
//
//}

/*!-------------------------------------------------------------------------------------------------------
 *  @brief 			 This function modbus slave process received query.

 *  @note        none

 *  @param       none

 *  @return      none
 ------------------------------------------------------------------------------------------------------*/
void ModbusSlaveProcessReceivedQuery(void)
{
	unsigned int i, j, k;

	gCrcModbus = 0xFFFF;
	for(i=0; i<gModbusSlaveData.totalRxValue-2; i++)
	{
			CalculateCrc(gModbusSlaveData.modbusFrame[i]);
	}

	if(gModbusSlaveData.modbusFrame[i++] !=  (gCrcModbus & 0xFF))
	{
			gModbusSlaveData.flags.bitValue.error = 1;
			return;
	}

	if(gModbusSlaveData.modbusFrame[i] !=  (gCrcModbus>>8))
	{
			gModbusSlaveData.flags.bitValue.error = 1;
			return;
	}

	if(gModbusSlaveData.modbusFrame[MBUS_SLAVE_ADDRESS_PTR] != gModbusSlaveData.address)
	{
			return;
	}

	gModbusSlaveData.totalTxValue = 0;
	switch(gModbusSlaveData.modbusFrame[MBUS_FUNCTION_PTR])
	{
	case readHoldingRegisters:
		if(((gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1) + (gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1)) < MBUS_FRAME_BUFFER_SIZE)
		{
			if(((gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1) + (gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1)))
			{
				j = (gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1);
				gModbusSlaveData.totalTxValue = 2;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR]<<1;

				for(i=j; i<(gModbusSlaveData.modbusFrame[2]+j); i++)
				{
					gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gModbusSlaveData.modbusHoldingRegister[i];
				}
			}
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
			{
				i = gModbusSlaveData.modbusFrame[MBUS_REGISTER_ADDRESS_LO_PTR]<<1;
				gModbusSlaveData.modbusHoldingRegister[i++]  = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_HI_PTR];
				gModbusSlaveData.modbusHoldingRegister[i]    = gModbusSlaveData.modbusFrame[MBUS_NO_OF_POINTS_LO_PTR];

			gModbusSlaveData.totalTxValue = 6;
			}
			else
			{
				gModbusSlaveData.totalTxValue = 1;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
				gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x02;
			}
		}
		else
		{
			gModbusSlaveData.totalTxValue = 1;
			gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] |= 0x80;
			gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] =  0x02;
		}
		break;

	case writeMultipleHoldingRegister:
		if((gModbusSlaveData.modbusFrame[MBUS_REGISTER_ADDRESS_LO_PTR]<<1) < MBUS_FRAME_BUFFER_SIZE)
		{
			j = gModbusSlaveData.modbusFrame[MBUS_START_ADDRESS_LO_PTR]<<1;
			k = MBUS_BYTE_COUNT_PTR+1;
			for(i=j; i<(gModbusSlaveData.modbusFrame[MBUS_BYTE_COUNT_PTR]+j); i++)
			{
				gModbusSlaveData.modbusHoldingRegister[i] = gModbusSlaveData.modbusFrame[k++];
			}
			gModbusSlaveData.totalTxValue = 6;
		}
		else
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

	gCrcModbus = 0xFFFF;
	for(i=0; i<gModbusSlaveData.totalTxValue; i++)
	{
			CalculateCrc(gModbusSlaveData.modbusFrame[i]);
	}
	gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gCrcModbus & 0xFF;
	gModbusSlaveData.modbusFrame[gModbusSlaveData.totalTxValue++] = gCrcModbus >> 8;
}

#else
/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function for modbus master rxcllback.

 *  @note        none

 *  @param       none

 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
// void ModbusMasterRxCpltCallback(void)
//{
//  if(gModbusMasterData.rxPointer < gModbusMasterData.totalRxValue)
//		    gModbusMasterData.modbusFrame[gModbusMasterData.rxPointer++] = gRxBuffer[0];
//
//	if(gModbusMasterData.rxPointer == gModbusMasterData.totalRxValue)
//			ModbusMasterProcessReceivedResponse();
//	{
//		if(gModbusMasterData.flags.bitValue.processSuccess)
//
//   	 //disable receiver uart interrupt
//	   // enable transmiter uart interrupt
//		gTXCounter++;
//	}
//}

/*!---------------------------------------------------------------------------------------------------------
 * @brief    Transmit Modbus Master

 * @note     None

 * @param    None

 * @return   None
-------------------------------------------------------------------------------------------------------- */
void TransmitModbusMaster(void)
{
	//used to built modbus frame
	uint16_t j;

	//1.no of bytes to transmit initialized to zero
	gModbusMasterData.totalTxValue = 0;

	//we use switch case here to check function code
	switch(gModbusMasterData.function)
	{
	case readHoldingRegisters:

		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.address;				// deviceId
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = readHoldingRegisters;		// function
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.startAddress >> 8;		// start address hi
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.startAddress & 0xFF;	// start address lo
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.totalRegisters >> 8;	// total registers hi
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.totalRegisters & 0xFF;	// total registers lo
		gModbusMasterData.totalRxValue = (gModbusMasterData.modbusFrame[5]<<1)+5;
		gModbusMasterData.responseTimeout = 1; // seconds
		break;

	case writeSingleHoldingRegister:
		break;

	case writeMultipleHoldingRegister:
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.address;				// deviceId
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = writeMultipleHoldingRegister;		// function
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.startAddress >> 8;		// start address hi
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.startAddress & 0xFF;	// start address lo
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.totalRegisters >> 8;	// total registers hi
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.totalRegisters & 0xFF;	// total registers lo
		gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.totalRegisters<<1;		// total bytes
		for(j=0; j<(gModbusMasterData.totalRegisters<<1); j++)
		{
			gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++] = gModbusMasterData.modbusHoldingRegister[j];
		}
		gModbusMasterData.totalRxValue = 8;
		gModbusMasterData.responseTimeout = 300; //mili seconds
		break;

	default:
		break;
	}

	gCrcModbus	=	0xFFFF;
	for(j=0; j<gModbusMasterData.totalTxValue; j++)
	{
		CalculateCrc(gModbusMasterData.modbusFrame[j]);
	}
	gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++]	=	gCrcModbus & 0xFF;
	gModbusMasterData.modbusFrame[gModbusMasterData.totalTxValue++]	=	gCrcModbus >> 8;

	gModbusMasterData.txPointer = 0;
	gModbusMasterData.rxPointer = 0;
	gModbusMasterData.responseTime = 0;
	gModbusMasterData.flags.all = 0;
	gModbusMasterData.flags.bitValue.busyProcessing = 1;
}

/*!--------------------------------------------------------------------------------------------------------------
 * @brief    Modbus Master Process Received Response

 * @note     None

 * @param    None

 * @return   None
 --------------------------------------------------------------------------------------------------------------*/
void ModbusMasterProcessReceivedResponse(void)
{
	uint16_t i,j;

	if(gModbusMasterData.modbusFrame[0] != gModbusMasterData.address)
	{
		gModbusMasterData.flags.bitValue.error = 1;
		gModbusMasterData.flags.bitValue.busyProcessing = 0;
		return;
	}

	if((gModbusMasterData.modbusFrame[1] & 0x7f) != gModbusMasterData.function)
	{
		gModbusMasterData.flags.bitValue.error = 1;
		gModbusMasterData.flags.bitValue.busyProcessing = 0;
		return;
	}

	if(gModbusMasterData.modbusFrame[1] & 0x80)
	{
		gModbusMasterData.flags.bitValue.excption = 1;
		gModbusMasterData.flags.bitValue.busyProcessing = 0;
		return;
	}

	switch(gModbusMasterData.function)
	{
	case readHoldingRegisters:
		if(gModbusMasterData.modbusFrame[2] != (gModbusMasterData.totalRegisters<<1))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		gCrcModbus	=	0xFFFF;
		for(i=0; i < (gModbusMasterData.totalRxValue-2); i++)
		{
			CalculateCrc(gModbusMasterData.modbusFrame[i]);
		}

		if(gModbusMasterData.modbusFrame[gModbusMasterData.totalRxValue-2] !=  (gCrcModbus & 0xFF))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		if(gModbusMasterData.modbusFrame[gModbusMasterData.totalRxValue-1] !=  (gCrcModbus>>8))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}
    gModbusMasterData.flags.bitValue.processSuccess = 1;
		gModbusMasterData.flags.bitValue.busyProcessing = 0;
		j = gModbusMasterData.startAddress << 1;
		if(gScanId == 1)
		{
			for(i=0; i<gModbusMasterData.modbusFrame[2]; i++)
			{
				gModbusMasterData.modbusHoldingRegister[i+j] = gModbusMasterData.modbusFrame[i+3];
			}
		}
		else if(gScanId == 2)
		{
			for(i=0; i<gModbusMasterData.modbusFrame[2]; i++)
			{
				gModbusMasterData.modbusHoldingRegister1[i+j] = gModbusMasterData.modbusFrame[i+3];
			}
		}
		break;

	case writeSingleHoldingRegister:
		break;

	case writeMultipleHoldingRegister:
		if(gModbusMasterData.modbusFrame[2] != gModbusMasterData.startAddress >> 8)
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		if(gModbusMasterData.modbusFrame[3] != (gModbusMasterData.startAddress & 0xFF))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		if(gModbusMasterData.modbusFrame[4] != gModbusMasterData.totalRegisters >> 8)
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		if(gModbusMasterData.modbusFrame[5] != (gModbusMasterData.totalRegisters & 0xFF))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		gCrcModbus	=	0xFFFF;
		for(i=0; i < 6; i++)
		{
			CalculateCrc(gModbusMasterData.modbusFrame[i]);
		}

		if(gModbusMasterData.modbusFrame[6] !=  (gCrcModbus & 0xFF))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		if(gModbusMasterData.modbusFrame[7] !=  (gCrcModbus>>8))
		{
			gModbusMasterData.flags.bitValue.error = 1;
			gModbusMasterData.flags.bitValue.busyProcessing = 0;
			return;
		}

		gModbusMasterData.flags.bitValue.processSuccess = 1;
		gModbusMasterData.flags.bitValue.busyProcessing = 0;
		break;

	default:
		break;
	}
}

#endif

/*!--------------------------------------------------------------------------------------------------------------
 * @brief   This function calculate CRC for modbus protocol communication.

 * @note    none

 * @param   crcreg - data for crc generation.

 * @return  none
 --------------------------------------------------------------------------------------------------------------*/
void CalculateCrc(unsigned char crcreg)
{
	unsigned char i;
	uint16_t	crcbit;

	crcreg &= 0xFF;
	gCrcModbus 	^= crcreg;
	for(i=0; i<=7; i++)
	{
		crcbit = 0;
		if((gCrcModbus & 0x0001) == 0x0001)
			crcbit = 1;

		gCrcModbus	 >>= 1;
		if(crcbit)
			gCrcModbus  ^= 0xa001;
	}
}







