/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gsm.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<stdbool.h>
#include <math.h>
#include "EEPROM_LIB.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
volatile uint8_t save_calib_flag = 0;


Calibration_t calib;
SystemConfig_t systemConfig;



uint8_t cali_mode = 0;      // 0 = normal, 1 = calibration
char cali_type[8] = {0};    // store command type: "VR","VY","VB","IR","IY","IB"








volatile uint8_t average_ready = 0; // Flag when 40 samples are complete

float adc_AI_f[16] = {0.0};
float adc_f = 9.2;
float current_diff =0;
uint8_t imeireceived = 0;

volatile uint8_t refresh_rate_locked = 0;  // Flag to prevent premature switching back
volatile uint8_t fast_cycle_completed = 0; // Flag to track if fast cycle completed
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim15;

UART_HandleTypeDef huart3;
UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */

volatile uint8_t calib_value_pending = 0;
float pending_calib_value = 0.0;
char pending_calib_type[8] = {0};


uint16_t adc_values[16];

volatile uint8_t send_json_flag = 0;
volatile uint16_t timer_counter = 0;  ////CHANGED ON 25/09/2025

int imei_fetched = 0;
int strength = -1;

uint16_t delay = 300;
char uart_buf[64];

char imei[64];
char sms_msg[256];
char sms_buffer[512];
char gsm_time[32] = {0};
char json_string[1024];
uint8_t i;
int gsm_battery_voltage = -1;
int gsm_input_voltage = -1; // From external device

volatile uint8_t adc_read_flag = 0;
volatile uint8_t rms_calc_flag = 0;

uint16_t adc_values_all[16];  // All channels A0-A13

// RMS calculation arrays - store 40 samples (20ms worth)

uint16_t sample_index = 0;
float individual_rms[NUM_SLOW_CHANNELS] = {0};

//static uint16_t rms_counter = 0;

// Add variables for threshold-based refresh rate control
volatile uint8_t current_refresh_rate = 0;  // 0 = refreshTime1, 1 = refreshTime2
volatile uint8_t threshold_exceeded = 0;
volatile uint8_t sms_counter = 0;
volatile uint8_t check_A0_A9_flag = 0;

uint8_t exceeded = 0;
uint8_t exceeded1 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM15_Init(void);
static void MX_USART5_UART_Init(void);
static void MX_USART3_UART_Init(void);
void Parse_And_Store_Calibration(const char *cmd);
void Save_CalibrationEEPROM(void);
int Load_Calib_From_EEPROM(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile uint8_t sms_flag = 0;


#define RX_BUFFER_SIZE 64
uint8_t uart_rx_buffer[RX_BUFFER_SIZE];
uint8_t uart_rx_index = 0;
char uart_cmd[32];


void calib_config(void)
{
	calib.A0_A9_fact = 1;
	calib.V_fact = 1;
	calib.I_fact = 1;

}



// Initialize default configuration
void Config_Init(void)
{
    strcpy(systemConfig.serverIP, "150.129.151.136");
    systemConfig.serverPort = 4300;
    systemConfig.refreshTime1 = 10; // Normal refresh rate (seconds)
    systemConfig.refreshTime2 = 20; // Fast refresh rate when threshold exceeded (seconds)
    systemConfig.threshold = 40;   // ADC threshold value
    strcpy(systemConfig.password, "12345");
}


void Handle_Command(const char *cmd)
{
    // Check for calibration mode commands
    if      (strncmp(cmd, "caliVR", 6) == 0) { cali_mode = 1; strcpy(cali_type, "VR"); }
    else if (strncmp(cmd, "caliVY", 6) == 0) { cali_mode = 1; strcpy(cali_type, "VY"); }
    else if (strncmp(cmd, "caliVB", 6) == 0) { cali_mode = 1; strcpy(cali_type, "VB"); }
    else if (strncmp(cmd, "caliIR", 6) == 0) { cali_mode = 1; strcpy(cali_type, "IR"); }
    else if (strncmp(cmd, "caliIY", 6) == 0) { cali_mode = 1; strcpy(cali_type, "IY"); }
    else if (strncmp(cmd, "caliIB", 6) == 0) { cali_mode = 1; strcpy(cali_type, "IB"); }

    else if (strncmp(cmd, "caliA0", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A0"); }
    else if (strncmp(cmd, "caliA1", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A1"); }
    else if (strncmp(cmd, "caliA2", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A2"); }
    else if (strncmp(cmd, "caliA3", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A3"); }
    else if (strncmp(cmd, "caliA4", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A4"); }
    else if (strncmp(cmd, "caliA5", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A5"); }
    else if (strncmp(cmd, "caliA6", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A6"); }
    else if (strncmp(cmd, "caliA7", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A7"); }
    else if (strncmp(cmd, "caliA8", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A8"); }
    else if (strncmp(cmd, "caliA9", 6) == 0) { cali_mode = 1; strcpy(cali_type, "A9"); }



    else if (strncmp(cmd, "caliSTOP", 8) == 0) { cali_mode = 0; strcpy(cali_type, "");  }

    // Check for calibration value setting (format: TYPE:VALUE)
    else if (strchr(cmd, ':') != NULL)
    {
        Parse_And_Store_Calibration(cmd);
    }

    // Command to read current calibration values
    else if (strcmp(cmd, "READ_CALIB") == 0)
    {
        char calib_info[128];
        snprintf(calib_info, sizeof(calib_info),
                 "A0_A9_fact:%.4f V_fact:%.4f I_fact:%.4f\r\n",
                 calib.A0_A9_fact, calib.V_fact, calib.I_fact);
        HAL_UART_Transmit(&huart3, (uint8_t*)calib_info, strlen(calib_info), HAL_MAX_DELAY);
    }

    // Command to reset to default calibration
    else if (strcmp(cmd, "RESET_CALIB") == 0)
    {
        calib_config();
        //save_calibration_flag = 1;
        //Save_Calibration_To_EEPROM();
        const char *msg = "Calibration reset to defaults and saved\r\n";
        HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
}


void UART3_Send_Cali_Data(void)
{
    char buffer[128];

    if      (strcmp(cali_type, "VR") == 0)
        snprintf(buffer, sizeof(buffer), "VR:%d\r\n", adc_average_m[0]);
    else if (strcmp(cali_type, "VY") == 0)
        snprintf(buffer, sizeof(buffer), "VY:%d\r\n", adc_average_m[2]);
    else if (strcmp(cali_type, "VB") == 0)
        snprintf(buffer, sizeof(buffer), "VB:%d\r\n", adc_average_m[4]);
    else if (strcmp(cali_type, "IR") == 0)
        snprintf(buffer, sizeof(buffer), "IR:%f\r\n", adc_average_F[1]);
    else if (strcmp(cali_type, "IY") == 0)
        snprintf(buffer, sizeof(buffer), "IY:%f\r\n", adc_average_F[3]);
    else if (strcmp(cali_type, "IB") == 0)
        snprintf(buffer, sizeof(buffer), "IB:%f\r\n", adc_average_F[5]);



    else if (strcmp(cali_type, "A0") == 0)
           snprintf(buffer, sizeof(buffer), "A0:%f\r\n", adc_AI_f[0]);
    else if (strcmp(cali_type, "A1") == 0)
	       snprintf(buffer, sizeof(buffer), "A1:%f\r\n", adc_AI_f[1]);
    else if (strcmp(cali_type, "A2") == 0)
	       snprintf(buffer, sizeof(buffer), "A2:%f\r\n", adc_AI_f[2]);
    else if (strcmp(cali_type, "A3") == 0)
	      snprintf(buffer, sizeof(buffer),  "A3:%f\r\n", adc_AI_f[3]);
    else if (strcmp(cali_type, "A4") == 0)
	      snprintf(buffer, sizeof(buffer),  "A4:%f\r\n", adc_AI_f[4]);

    else if (strcmp(cali_type, "A5") == 0)
           snprintf(buffer, sizeof(buffer), "A0:%f\r\n", adc_AI_f[5]);
    else if (strcmp(cali_type, "A6") == 0)
	       snprintf(buffer, sizeof(buffer), "A1:%f\r\n", adc_AI_f[6]);
    else if (strcmp(cali_type, "A7") == 0)
	       snprintf(buffer, sizeof(buffer), "A2:%f\r\n", adc_AI_f[7]);
    else if (strcmp(cali_type, "A8") == 0)
	      snprintf(buffer, sizeof(buffer),  "A3:%f\r\n", adc_AI_f[8]);
    else if (strcmp(cali_type, "A9") == 0)
	      snprintf(buffer, sizeof(buffer),  "A4:%f\r\n", adc_AI_f[9]);

    HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}


// Replace Parse_And_Store_Calibration() function in main.c

void Parse_And_Store_Calibration(const char *cmd)
{
    float known_voltage = 0.0f;
    char type[8] = {0};
    float cal_factor = 0.0f;
    uint16_t adc_count = 0;

    // Parse command like "VR:5.000" (instrument's actual voltage)
    if (sscanf(cmd, "%[^:]:%f", type, &known_voltage) == 2)
    {
        // --- Voltage calibration ---
        if (strcmp(type, "VR") == 0)
        {
            adc_count = adc_average[0];
            cal_factor = known_voltage / adc_count;
            calib.V_fact = cal_factor;
        }
        else if (strcmp(type, "VY") == 0)
        {
            adc_count = adc_average[2];
            cal_factor = known_voltage / adc_count;
            calib.V_fact = cal_factor;
        }
        else if (strcmp(type, "VB") == 0)
        {
            adc_count = adc_average[4];
            cal_factor = known_voltage / adc_count;
            calib.V_fact = cal_factor;
        }

        // --- Current calibration ---
        else if (strcmp(type, "IR") == 0)
        {
            float adc_measured = adc_average[1];
            cal_factor = known_voltage / adc_measured;
            calib.I_fact = cal_factor;
        }
        else if (strcmp(type, "IY") == 0)
        {
            float adc_measured = adc_average[3];
            cal_factor = known_voltage / adc_measured;
            calib.I_fact = cal_factor;
        }
        else if (strcmp(type, "IB") == 0)
        {
            float adc_measured = adc_average[5];
            cal_factor = known_voltage / adc_measured;
            calib.I_fact = cal_factor;
        }

        // --- Analog input calibration (A0–A9) ---
        // CRITICAL FIX: Apply the SAME offset subtraction used in display!
        else if (strncmp(type, "A", 1) == 0)
        {
            int ch = atoi(&type[1]);
            if (ch >= 0 && ch <= 9)
            {
                // FIXED: Use the same formula as in Create_JSON_String
                float adc_adjusted = (float)(adc_values_all[ch] - 4);

                // Calculate calibration factor
                cal_factor = known_voltage / adc_adjusted;
                calib.A0_A9_fact = cal_factor;

                // Debug output to verify calculation
                char debug_msg[128];
                snprintf(debug_msg, sizeof(debug_msg),
                         "A%d: Raw=%d, Adj=%.2f, Known=%.2f, Factor=%.6f\r\n",
                         ch, adc_values_all[ch], adc_adjusted, known_voltage, cal_factor);
                HAL_UART_Transmit(&huart3, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
            }
        }

        // --- Save flag trigger ---
        save_calib_flag = 1;

        // --- Confirmation message ---
        char confirm_msg[64];
        snprintf(confirm_msg, sizeof(confirm_msg),
                 "Calib %s set (factor=%.6f) and saved\r\n",
                 type, cal_factor);
        HAL_UART_Transmit(&huart3, (uint8_t*)confirm_msg, strlen(confirm_msg), HAL_MAX_DELAY);
    }
    else
    {
        const char *error_msg = "Invalid format. Use TYPE:VOLTAGE (e.g., VR:5.000)\r\n";
        HAL_UART_Transmit(&huart3, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
    }
}




void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	ADC_Process_Conversion_Complete();
}


// Modified JSON creation function - replace your existing Create_JSON_String
void Create_JSON_String(void)
{
	 for (uint8_t ch = 0; ch < 10; ch++)
	 {
	    adc_AI_f[ch] = ((float)(adc_values_all[ch]-4)) * calib.A0_A9_fact;
	 }

    // Read GSM status
	 //if(imeireceived == 0)
    gsm_get_imei();
    gsm_get_time_from_module();
    int sig_strength = signal_strength();

    // Create JSON with raw values for A0-A7 and averages for A8-A15
    snprintf(json_string, sizeof(json_string),
        "{"
        "\"imei\":\"%s\","
        "\"timestamp\":\"%s\","
        "\"signal_strength\":%d,"
        "\"adc_channels\":{"
            "\"A0\":%.2f,"
            "\"A1\":%.2f,"
            "\"A2\":%.2f,"
            "\"A3\":%.2f,"
            "\"A4\":%.2f,"
            "\"A5\":%.2f,"
            "\"A6\":%.2f,"
            "\"A7\":%.2f,"
            "\"A8\":%.2f,"
            "\"A9\":%.2f,"
            "\"VR\":%d,"
            "\"IA\":%.2f,"
            "\"VY\":%d,"
            "\"IB\":%.2f,"
            "\"VB\":%d,"
            "\"IC\":%.2f"
        "}"
        "}",
        imei, gsm_time,
		sig_strength,
		adc_AI_f[0], adc_AI_f[1], adc_AI_f[2], adc_AI_f[3],
		adc_AI_f[4], adc_AI_f[5], adc_AI_f[6], adc_AI_f[7],
		adc_AI_f[8], adc_AI_f[9],
        // Average values for A8-A15
		adc_average_m[0], adc_average_F[1], adc_average_m[2], adc_average_F[3],
		adc_average_m[4], adc_average_F[5]
    );
    //GSM_Debug_Number("JSON Length", strlen(json_string));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)   // if from UART3
    {
        char c = uart_rx_buffer[uart_rx_index];

        if (c == '\n' || c == '\r')  // End of command
        {
            uart_cmd[uart_rx_index] = '\0'; // null terminate
            Handle_Command(uart_cmd);       // process command
            uart_rx_index = 0;              // reset for next command
        }
        else
        {
            if (uart_rx_index < (RX_BUFFER_SIZE - 1))
            {
                uart_cmd[uart_rx_index++] = c;
            }
        }

        // re-enable reception
        HAL_UART_Receive_IT(&huart3, &uart_rx_buffer[uart_rx_index], 1);
    }
}


float mul = 0.19;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        ADC_Process_Timer_Interrupt();
    }

    if (htim->Instance == TIM15)
    {
        timer_counter++;
        sms_counter++;

        // Use dynamic refresh rate based on current mode
        uint16_t active_refresh_time = (current_refresh_rate == 0) ?
                                       systemConfig.refreshTime1 :
                                       systemConfig.refreshTime2;

        // Check if it's time to send JSON
        if (timer_counter >= active_refresh_time)
        {
            send_json_flag = 1;
            timer_counter = 0;  // Reset for next cycle

            // Mark that a complete cycle finished in fast mode
            // This flag will be used to count actual transmitted cycles
            if (current_refresh_rate == 1)
            {
                fast_cycle_completed = 1;
            }
        }

        if (sms_counter >= 60)
        {
            sms_flag = 1;
            sms_counter = 0;
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_I2C2_Init();
  MX_TIM6_Init();
  MX_TIM15_Init();
  MX_USART5_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  Load_Config_From_EEPROM();
  Load_Calib_From_EEPROM();
  GSM_PowerOn();
  //GSM_SetBaudRate(9600);
  gsm_get_imei();
  HAL_Delay(5000);
  HAL_ADCEx_Calibration_Start(&hadc);
  HAL_UART_Receive_IT(&huart3, &uart_rx_buffer[uart_rx_index], 1);

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim15);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)


  {

	  //GSM_SetBaudRate(9600);
	    //Check_Incoming_SMS_Improved();

	    if ( save_calib_flag)
	    {
	    	 save_calib_flag = 0;
	    	 Save_CalibrationEEPROM();


	    }

      if (sms_flag)
      {
          sms_flag = 0;  // Clear the flag
          Check_Incoming_SMS_Improved();
      }
      if (cali_mode == 0)
      {
             // Normal operation
             if (check_A0_A9_flag)
             {
                 check_A0_A9_flag = 0;
                 Check_Fast_Channels_A0_A9();
                 Update_Refresh_Rate_From_Flags();
             }

             if (average_ready)
             {
                 Check_Slow_Channels_A10_A15();
                 Update_Refresh_Rate_From_Flags();

             }

             if (send_json_flag && average_ready)
             {
                 send_json_flag = 0;
                 average_ready = 0;
                 GPRS_Send_JSON();
                //Update_Refresh_Rate_From_Flags();
             }
      }
	  else
	  {
		 // Calibration mode → only send raw ADC via UART3
		 if (average_ready)
		 {
			 average_ready = 0;
			 UART3_Send_Cali_Data();
		 }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_15;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x2000090E;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 7;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period =499;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 7999;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 999;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART5_UART_Init(void)
{

  /* USER CODE BEGIN USART5_Init 0 */

  /* USER CODE END USART5_Init 0 */

  /* USER CODE BEGIN USART5_Init 1 */

  /* USER CODE END USART5_Init 1 */
  huart5.Instance = USART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART5_Init 2 */

  /* USER CODE END USART5_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
