#ifndef ADC_H
#define ADC_H

#include "main.h"
#include <math.h>
#include "EEPROM_LIB.h"

/* Defines */
#define RMS_SAMPLE_COUNT 160      // Calculate RMS every 160 samples (80ms)
#define FAST_CHECK_COUNT 40       // every 40 samples (20ms)
#define NUM_SLOW_CHANNELS 6       // A10-A15
#define ADC_CHANNELS 16

/* External variables from main.c that ADC module needs */
extern ADC_HandleTypeDef hadc;
/* ADC data structures and variables */
extern uint16_t adc_buffer[ADC_CHANNELS];
extern uint16_t adc_values_all[16];
extern uint32_t adc_sum[NUM_SLOW_CHANNELS];
extern uint16_t adc_average[NUM_SLOW_CHANNELS];
extern uint16_t adc_average_m[6];
extern float adc_average_F[NUM_SLOW_CHANNELS];
extern float adc_AI_f[16];
void Check_Fast_Channels_A0_A9(void);
void Check_Slow_Channels_A10_A15(void);
void Update_Refresh_Rate_From_Flags(void);
void ADC_Process_Conversion_Complete(void);
void ADC_Process_Timer_Interrupt(void);
extern volatile uint8_t average_ready;
extern volatile uint8_t check_A0_A9_flag;

/* Threshold monitoring variables */
extern  uint8_t exceeded;
extern  uint8_t exceeded1;
extern volatile uint8_t current_refresh_rate;
extern volatile uint8_t threshold_exceeded;

/* Function prototypes */


#endif /* ADC_H */
