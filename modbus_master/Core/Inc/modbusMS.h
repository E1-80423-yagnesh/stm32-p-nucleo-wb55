/*!----------------------------------------------------------------------------------------------------
 * @file  				  MODBUS.h

 * @version         3.0

 * @Discription 	 	This file contains the definitions for MODBUS.c

 *  Date           	Author                 Project                               Reason for change
 * -----------------------------------------------------------------------------------------------------
 * 13/03/2018       Kalpesh Gajera         Modbus Master and Slave               Created
 * 19/04/2024       shyam parmar           Modbus Master and Slave							 firmware Standards
 */
//-------------------------------------------------------------------------------------------------------
#include<stdint.h>
/* Define ---------------------------------------------------------------------------------------------*/
#ifndef SERIAL_COMMUNICATION_H_
#define SERIAL_COMMUNICATION_H_
/* If you move this file to a different CPU, make sure to check the UART and TIME number and configure RX , TX and GPIO according the CPU format.*/
/* if you use ARM cpu but another UART ,change UART_NO value and change pin in stm32Fxxx_hal_msp.c file.*/
/* This file use for configure our device as a master or slave*/

//#define  SLAVE //MASTER
#define  MASTER
#define  SLAVE_ID          1
#define  NO_OF_REG         3
#define  READ_HOLDING_REG  3
#define  START_ADDR        0
#define  RECEIVE_BYTE      1

#define UART_NO            &huart1
#define TIMER_NO           &htim2

#define RS485_EN_PIN       GPIO_PIN_6
#define RS485_EN_PORT      GPIOB

#define En_SET             HAL_GPIO_WritePin(RS485_EN_PORT,RS485_EN_PIN, GPIO_PIN_SET);
#define En_RESET           HAL_GPIO_WritePin(RS485_EN_PORT,RS485_EN_PIN, GPIO_PIN_RESET);

#define RECEIVE_QUERY      HAL_UART_Receive_IT(UART_NO,gRxBuffer,RECEIVE_BYTE);
#define TRANSMIT_QUERY     HAL_UART_Transmit_IT(UART_NO,(uint8_t*)gModbusMasterData.modbusFrame,gModbusMasterData.totalTxValue);
#define TRANSMIT_RESPONCE  HAL_UART_Transmit_IT(UART_NO,(uint8_t*)gModbusSlaveData.modbusFrame,gModbusSlaveData.totalTxValue);

#define MASTER_TIMER       HAL_TIM_Base_Start_IT(TIMER_NO);


#define MBUS_SLAVE_ADDRESS_PTR          0
#define MBUS_FUNCTION_PTR               1
#define MBUS_START_ADDRESS_HI_PTR       2
#define MBUS_START_ADDRESS_LO_PTR       3
#define MBUS_REGISTER_ADDRESS_HI_PTR    2
#define MBUS_REGISTER_ADDRESS_LO_PTR    3
#define MBUS_NO_OF_POINTS_HI_PTR        4
#define MBUS_NO_OF_POINTS_LO_PTR        5
#define MBUS_PRESET_DATA_HI_PTR         4
#define MBUS_PRESET_DATA_LO_PTR         5
#define MBUS_NO_OF_REGISTERS_HI_PTR     4
#define MBUS_NO_OF_REGISTERS_LO_PTR     5
#define MBUS_BYTE_COUNT_PTR             6
#define MBUS_CRC_LO_PTR                 6
#define MBUS_CRC_HO_PTR                 7
#define MBUS_FRAME_BUFFER_SIZE          200


/*global variable------------------------------------------------------------------------------------*/
extern unsigned char gRxBuffer[20];
extern uint8_t gTXCounter;//changes made by me


/* Type define---------------------------------------------------------------------------------------*/
typedef enum
{
    readHoldingRegisters = 3,
    writeSingleHoldingRegister = 6,
    writeMultipleHoldingRegister = 16
}MODBUS_FUNCTION;

typedef union
{
    unsigned int  all;
    struct  ModbusBits
    {
        unsigned int  busyProcessing:1;
        unsigned int  processTimeOut:1;
        unsigned int  processSuccess:1;
        unsigned int  error:1;
        unsigned int  responseReceived:1;
        unsigned int  excption:1;
        unsigned int  rsved:11;
    }bitValue;
}MODBUS_FLAG;

#ifdef SLAVE
typedef struct
{
    unsigned int     startAddress;
    unsigned int     totalRegisters;
    unsigned int     modbusHoldingRegister[MBUS_FRAME_BUFFER_SIZE>>1];
    unsigned char    modbusFrame[MBUS_FRAME_BUFFER_SIZE+5];
    unsigned int     txPointer;
    unsigned int     rxPointer;
    unsigned int     totalTxValue;
    unsigned int     totalRxValue;

    MODBUS_FLAG      flags;
    MODBUS_FUNCTION  function;

    unsigned int     address;
    unsigned int     responseTime;
    unsigned int     responseTimeout;
}MODBUS_SLAVE_DATA;

extern MODBUS_SLAVE_DATA gModbusSlaveData;

#else
typedef struct
{
	unsigned int	  startAddress;
	unsigned int	  totalRegisters;
	unsigned int 	  modbusHoldingRegister[50];
  unsigned int    modbusHoldingRegister1[50];
	unsigned char   modbusFrame[200];
	unsigned int	  txPointer;
	unsigned int	  rxPointer;
	unsigned int	  totalTxValue;
	unsigned int	  totalRxValue;

	MODBUS_FLAG		  flags;
	MODBUS_FUNCTION	function;

	unsigned int	  address;
	unsigned int	  responseTime;
	unsigned int	  responseTimeout;
	unsigned int	  totalRetry;
	unsigned int	  retryCount;
}MODBUS_MASTER_DATA;

extern MODBUS_MASTER_DATA	gModbusMasterData;

#endif

/* Function prototypes -------------------------------------------------------------*/
void ModbusInit(void);
void ModbusSlaveProcessReceivedQuery(void);
void CalculateCrc(unsigned char crcreg);
void ModbusMasterProcessReceivedResponse(void);
void TransmitModbusMaster(void);
void ModbusMasterRxCpltCallback(void);
void ModbusSlaveRxCpltCallback(void);

#endif /*SERIAL_COMMUNICATION_H_*/
