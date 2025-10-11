#ifndef __GSM_H
#define __GSM_H

#include "main.h"

#define GSM_PWRKEY_PIN GPIO_PIN_3
#define GSM_PWRKEY_PORT GPIOB

// Function prototypes
void GSM_PowerOn(void);
void GSM_SendCommand(const char *cmd, char *response, uint16_t resp_size);
void GSM_SendCommand_Extended(const char *cmd, char *response, uint16_t resp_size, uint32_t timeout_ms);
void GSM_SendCommand_Improved(const char *cmd, char *response, uint16_t resp_size);
void Clear_UART_Buffer(void);

// GSM Status Functions
void gsm_get_time_from_module(void);
void gsm_get_imei(void);
int signal_strength(void);
int signal_strength_improved(void);
int gsm_get_battery_voltage(void);

// SMS Functions
void Setup_SMS_Mode(void);
void Check_Incoming_SMS_Improved(void);
int Parse_SMS_Messages(char* sms_data);
int Extract_Config_From_SMS_Improved(char* sms_content);
void Create_JSON_String(void);
// GPRS and HTTP Functions
void GPRS_Send_JSON(void);
void GSM_Debug_Print(const char *data);
void GSM_SetBaudRate(uint32_t baudrate);

// External variable declarations (not definitions!)
extern SystemConfig_t systemConfig;
extern uint16_t adc_values[16];
extern volatile uint8_t send_json_flag;

extern int imei_fetched;
extern int strength;
extern uint16_t delay;
extern char uart_buf[64];
extern char imei[64];
extern char sms_msg[256];
extern char sms_buffer[512];
extern char gsm_time[32];
extern char json_string[1024];
extern uint8_t i;
extern int gsm_battery_voltage;
extern int gsm_input_voltage;
extern volatile uint8_t adc_read_flag;
extern volatile uint8_t rms_calc_flag;
extern uint16_t adc_values_all[16];
extern uint16_t sample_index;
extern volatile uint8_t current_refresh_rate;
extern volatile uint8_t threshold_exceeded;

#ifdef __cplusplus
}
#endif

#endif /* __GSM_H */
