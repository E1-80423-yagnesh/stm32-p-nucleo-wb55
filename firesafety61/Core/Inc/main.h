/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "EEPROM_LIB.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern ADC_HandleTypeDef hadc;
extern DMA_HandleTypeDef hdma_adc;

extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim15;

extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart3;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

typedef struct{

	float A0_A9_fact;
	float V_fact;
	float I_fact;


}Calibration_t;

extern Calibration_t calib;

typedef struct {
    char serverIP[20];
    uint16_t serverPort;
    uint16_t refreshTime1;       // Normal refresh rate (seconds)
    uint16_t refreshTime2;       // Fast refresh rate when threshold exceeded (seconds)
    uint16_t threshold;          // ADC threshold value
    char password[20];
} SystemConfig_t;

extern volatile uint16_t timer_counter; //CHANGED ON 25/09/2025
extern uint8_t imeireceived;
extern volatile uint8_t refresh_rate_locked;
extern volatile uint8_t fast_cycle_completed;

// EEPROM Configuration Functions
void Save_Config_To_EEPROM(void);
int Load_Config_From_EEPROM(void);
void Config_EEPROM(void);

// External Variables
extern SystemConfig_t systemConfig;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
