#include "motor.h"
#include <stdio.h>


uint32_t value;
uint32_t gconf_readback;
uint32_t chopconf_readback;
uint32_t pwmconf_readback;
HAL_StatusTypeDef uart_status;
uint32_t time1 = 200;
uint32_t time2 = 200;


//int step = 1;
uint8_t data[8];


uint8_t calculateCRC(uint8_t *data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        uint8_t currentByte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc >> 7) ^ (currentByte & 0x01)) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = crc << 1;
            }
            currentByte = currentByte >> 1;
        }
    }
    return crc;
}

void TMC2226_WriteRegister(uint8_t reg, uint32_t value) {


    // Build the write frame
    data[0] = TMC2226_SYNC_BYTE;
    data[1] = TMC2226_SLAVE_ADDR;
    data[2] = reg | TMC2226_WRITE_FLAG;
    data[3] = (value >> 24) & 0xFF;
    data[4] = (value >> 16) & 0xFF;
    data[5] = (value >> 8) & 0xFF;
    data[6] = value & 0xFF;
    data[7] = calculateCRC(data, 7);

    // Send via UART (adjust for your UART instance)
    HAL_UART_Transmit(&huart5, data, 8, 500);




    HAL_Delay(10);
}

//int TMC2226_WriteRegister(uint8_t reg, uint32_t value)
//{
//
//
//    // Build the write frame
//    data[0] = TMC2226_SYNC_BYTE;
//    data[1] = TMC2226_SLAVE_ADDR;
//    data[2] = reg | TMC2226_WRITE_FLAG;
//    data[3] = (value >> 24) & 0xFF;
//    data[4] = (value >> 16) & 0xFF;
//    data[5] = (value >> 8) & 0xFF;
//    data[6] = value & 0xFF;
//    data[7] = calculateCRC(data, 7);
//
//    // Send via UART
//        uart_status = HAL_UART_Transmit(&huart5, data, 8, 500);
//        if (uart_status != HAL_OK) {
//            printf("UART Transmit Failed: %d\r\n", uart_status);
//            return -1; // Write failed
//        }
//
//
//    HAL_Delay(10);
//    return 0;
//}
uint32_t TMC2226_ReadRegister(uint8_t reg) {
    uint8_t writeData[4];
    uint8_t readData[8];

    // Send read request
    writeData[0] = TMC2226_SYNC_BYTE;
    writeData[1] = TMC2226_SLAVE_ADDR;
    writeData[2] = reg ;
    writeData[3] = calculateCRC(writeData, 3);

    HAL_UART_Transmit(&huart5, writeData, 4, 100);
    HAL_Delay(10);

    // Read response
    HAL_UART_Receive(&huart5, readData, 8, 100);

    // Extract 32-bit value from response (bytes 7-10)
    uint32_t value = ((uint32_t)readData[3] << 24) |
                     ((uint32_t)readData[4] << 16) |
                     ((uint32_t)readData[5] << 8) |
                     (uint32_t)readData[6];

    return value;
}

//uint32_t TMC2226_ReadRegister(uint8_t reg) {
//    uint8_t writeData[4];
//    uint8_t readData[8];
//
//    // Send read request
//    writeData[0] = TMC2226_SYNC_BYTE;    // Should be 0x05
//    writeData[1] = 0x00;   // Should be 0xFF
//    writeData[2] = reg;
//    writeData[3] = calculateCRC(writeData, 3);
//
//    HAL_UART_Transmit(&huart5, writeData, 4, 100);
//    //HAL_Delay(10);
//
//    // Read response
//	uart_status = HAL_UART_Receive(&huart5, readData, 8, 200);
//	if (uart_status != HAL_OK) {
//
//		return -1; // Write failed
//	}
//
//
//    // Byte 0: Sync nibble correct?
//    if (readData[0] != 0x05) {
//        return 2; // Invalid sync byte
//    }
//
//    // Byte 1: Master address correct?
//    if (readData[1] != 0xFF) {
//        return 3; // Invalid master address
//    }
//
//    // Byte 2: Address correct?
//    if (readData[2] != reg) {
//        return 4; // Register address mismatch
//    }
//
//    // Byte 7: CRC correct?
//    if (readData[7] != calculateCRC(readData, 7)) {
//        return 5; // CRC validation failed
//    }
//
//    // Extract 32-bit value from response (bytes 3-6)
//    uint32_t value = ((uint32_t)readData[3] << 24) |
//                     ((uint32_t)readData[4] << 16) |
//                     ((uint32_t)readData[5] << 8) |
//                     (uint32_t)readData[6];
//
//    return value;
//}



void TMC2226_OTP(void)
{


    uint32_t otpProgValue = TMC2226_OTP_MAGIC | (OTP_INTERNAL_RSENSE_BYTE << 4) | OTP_INTERNAL_RSENSE_BIT;
    HAL_Delay(40);
    TMC2226_WriteRegister(TMC2226_OTP_PROG, otpProgValue);
    HAL_Delay(15); // Wait for programming


}

void TMC2226_Init(void) {
    HAL_Delay(100);  // Wait for driver to be ready

    // Reset and configure GCONF register
    uint32_t gconf = 0;
    gconf |= (1 << 0);   // I_scale_analog = 1 (use VREF for current setting)
    gconf |= (1 << 1);   // internal_rsense = 1 (use internal sense resistors)
    gconf |= (0 << 2);   // en_spreadcycle = 0 (start with StealthChop)
    gconf |= (0 << 3);   // shaft = 0 (normal direction)
    gconf |= (0 << 4);   // index_otpw = 0  INDEX shows the first microstep position of sequencer
    gconf |= (0 << 5);   // index_step = 0
    gconf |= (1 << 6);   // pdn_disable = 1 (UART control)
    gconf |= (0 << 7);   // mstep_reg_select = 1 (microsteps via UART)
    gconf |= (0 << 8);   // multistep_filt = 0
    gconf |= (0 << 9);   // test_mode = 0
	TMC2226_WriteRegister(TMC2226_GCONF, gconf);

	HAL_Delay(10);

	//gconf_readback = TMC2226_ReadRegister(TMC2226_GCONF);



    // Configure CHOPCONF register for basic StealthChop operation
    uint32_t chopconf = 0;
    chopconf |= (5 << 0);    // toff = 5 (chopper off time)
    chopconf |= (2 << 4);    // hstrt = 2 (hysteresis start)
    chopconf |= (0 << 7);    // hend = 0 (hysteresis end)
    chopconf |= (0 << 15);   // tbl = 0 (blanking time)
    chopconf |= (0 << 17);   // vsense = 0 (high sensitivity)
    //chopconf |= (8 << 24);   // mres = 4 (16 microsteps)
    chopconf |= (1 << 28);   // intpol = 1 (interpolation)
    chopconf |= (0 << 29);   // dedge = 0

    chopconf |= (0 << 30);   // diss2g = 0
    chopconf |= (0 << 31);   // diss2vs = 0
    TMC2226_WriteRegister(TMC2226_CHOPCONF, chopconf);
    //chopconf_readback = TMC2226_ReadRegister(TMC2226_CHOPCONF);
    //setup hold, run current
    TMC2226_SetCurrent(6,3);

//    // Configure PWM for StealthChop
//    uint32_t pwmconf = 0;
//    pwmconf |= (4 << 0);     // pwm_ofs = 4
//    pwmconf |= (8 << 8);     // pwm_grad = 8
//    pwmconf |= (4 << 16);    // pwm_freq = 4
//    pwmconf |= (1 << 18);    // pwm_autoscale = 1
//    pwmconf |= (1 << 19);    // pwm_autograd = 1
//    pwmconf |= (0 << 20);    // freewheel = 0
//    pwmconf |= (0 << 23);    // pwm_reg = 0
//    pwmconf |= (4 << 24);    // pwm_lim = 4
//    TMC2226_WriteRegister(TMC2226_PWMCONF, pwmconf);
//    // pwmconf_readback = TMC2226_ReadRegister(TMC2226_PWMCONF);

    HAL_Delay(10);
   // printf("TMC2226 Initialized Successfully\r\n");
}

void TMC2226_SetCurrent(uint16_t run_current, uint16_t hold_current) {
    if (run_current > 31) run_current = 31;
    if (hold_current > 31) hold_current = 31;

    uint32_t ihold_irun = 0;
    ihold_irun |= (hold_current << 0);  // IHOLD
    ihold_irun |= (run_current << 8);   // IRUN
    ihold_irun |= (5 << 16);            // IHOLDDELAY

    TMC2226_WriteRegister(TMC2226_IHOLD_IRUN, ihold_irun);
   // printf("Current Set - Run: %d/31, Hold: %d/31\r\n", run_current, hold_current);
}

void setupMotor(void)
{
    // Power-on reset sequence
    HAL_GPIO_WritePin(ENABLE_PORT, ENABLE_PIN, GPIO_PIN_SET);  // Disable first
    HAL_Delay(10);  // Wait 10ms

    // Configure pins before enabling
    HAL_GPIO_WritePin(PDN_UART_TX_PORT, PDN_UART_TX_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PDN_UART_RX_PORT, PDN_UART_RX_PIN, GPIO_PIN_SET);


     TMC2226_Init();
    // Set microstepping
    setMicrostepping(0, 0);





    // Enable stealth mode
    //enableStealthMode();

    enablespreadcyclemode();

    HAL_Delay(1);  // Short delay before enabling

    // enable the driver
    HAL_GPIO_WritePin(ENABLE_PORT, ENABLE_PIN, GPIO_PIN_RESET);


    HAL_Delay(10);


}


void setMicrostepping(uint8_t ms1_state, uint8_t ms2_state)
{
    // Set MS1 pin
    if (ms1_state) {
        HAL_GPIO_WritePin(MS1_PORT, MS1_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(MS1_PORT, MS1_PIN, GPIO_PIN_RESET);
    }

    // Set MS2 pin
    if (ms2_state) {
        HAL_GPIO_WritePin(MS2_PORT, MS2_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(MS2_PORT, MS2_PIN, GPIO_PIN_RESET);
    }


}

void enableStealthMode(void)
{

    HAL_GPIO_WritePin(SPREAD_PORT, SPREAD_PIN, GPIO_PIN_RESET);

}

void enablespreadcyclemode(void)
{

    HAL_GPIO_WritePin(SPREAD_PORT, SPREAD_PIN, GPIO_PIN_SET);

}


void rotateMotor(int steps, uint8_t clockwise)
{


    // Set motor direction
    if(clockwise)
    {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
    }

    // Step the motor
    for(int i = 0; i < steps; i++)
    {
        HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);

        microsecond_delay(time1);
        HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);

        microsecond_delay(time2);
    }

}

void homePosition(void)
{
    GPIO_PinState caseState;
    caseState = HAL_GPIO_ReadPin(CASE_PORT, CASE_PIN);


   if(caseState == GPIO_PIN_SET)
   {
	   rotateMotor(300, 1);
   }

    else if(caseState == GPIO_PIN_RESET)
    {
	   rotateMotor(0, 1);
       currentState = 255;

   }

   //    HAL_GPIO_WritePin(INDEX_PORT,INDEX_PIN, GPIO_PIN_SET);
   //    microsecond_delay(100);
   //    HAL_GPIO_WritePin(INDEX_PORT,INDEX_PIN, GPIO_PIN_RESET);
}


void lampOn(void)
{

    // ARR = 1000, PSC = 79 ,pwm freq = 1KHZ
    //uint32_t value = (duty_cycle * 1000) / 100;
    //TIM3->CCR2 = 1000
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    // Set the PWM duty cycle
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 1000);

    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
}

void lampOff(void)
{

    // ARR = 1000, PSC = 79 ,pwm freq = 1KHZ
    //uint32_t value = (duty_cycle * 1000) / 100;
    //TIM3->CCR2 = 1000
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    // Set the PWM duty cycle
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);

    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
}





uint16_t readLampCurrent(void)
{

    return 0;
}

void microsecond_delay(uint16_t microseconds)
{


	__HAL_TIM_SET_COUNTER(&htim16, 0);
	 HAL_TIM_Base_Start(&htim16);
    //__HAL_TIM_SET_COUNTER(&htim16, 0);




    while (__HAL_TIM_GET_COUNTER(&htim16) < microseconds);
    HAL_TIM_Base_Stop(&htim16);
}
