#include "gsm.h"

#define DEFAULT_PASSWORD "1234"

char sender_number[20] = {0};  // Store sender's phone number

void GSM_PowerOn(void)
{
    // Pull low for at least 1 second to turn on SIM800C
    HAL_Delay(100);

    // Release (set high)
    HAL_GPIO_WritePin(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN, GPIO_PIN_SET);

    // Wait for module to boot up
    HAL_Delay(1000);

    HAL_GPIO_WritePin(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN, GPIO_PIN_RESET);

    // Pull low for at least 1 second to turn on SIM800C
    HAL_Delay(100);
}

void GSM_Debug_Print(const char *data)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)data, strlen(data), HAL_MAX_DELAY);
}

// for data send on server
void GSM_SendCommand(const char *cmd, char *response, uint16_t resp_size)
{
    memset(response, 0, resp_size);
    HAL_UART_Transmit(&huart5, (uint8_t*)cmd, strlen(cmd), 100);
    HAL_UART_Receive(&huart5, (uint8_t*)response, resp_size, 100);
}


//for sms
void GSM_SendCommand_Extended(const char *cmd, char *response, uint16_t resp_size, uint32_t timeout_ms)
{
    memset(response, 0, resp_size);

    // Clear any pending data
    uint8_t dummy;
    while (HAL_UART_Receive(&huart5, &dummy, 1, 10) == HAL_OK) {
        // Clear buffer
    }

    // Send command
    HAL_UART_Transmit(&huart5, (uint8_t*)cmd, strlen(cmd), 1000);

    // Receive response with longer timeout
    uint16_t idx = 0;
    uint32_t start_time = HAL_GetTick();
    char c;

    while (idx < resp_size - 1 && (HAL_GetTick() - start_time) < timeout_ms) {
        if (HAL_UART_Receive(&huart5, (uint8_t*)&c, 1, 100) == HAL_OK) {
            response[idx++] = c;

            // Check for complete response
            if (strstr(response, "\r\nOK\r\n") || strstr(response, "ERROR") ||
                strstr(response, "+CMGL:") || strstr(response, "+CMGR:")) {
                // Continue reading for a bit more to get complete response
                uint32_t extra_time = HAL_GetTick();
                while ((HAL_GetTick() - extra_time) < 500 && idx < resp_size - 1) {
                    if (HAL_UART_Receive(&huart5, (uint8_t*)&c, 1, 50) == HAL_OK) {
                        response[idx++] = c;
                    }
                }
                break;
            }
        }
    }
    response[idx] = '\0';
}


void Clear_UART_Buffer(void)
{
    uint8_t dummy;
    while (HAL_UART_Receive(&huart5, &dummy, 1, 10) == HAL_OK) {
        // Clear buffer
    }
}

//for signal strength
void GSM_SendCommand_Improved(const char *cmd, char *response, uint16_t resp_size)
{
    memset(response, 0, resp_size);

    // Clear any pending data
    Clear_UART_Buffer();

    // Send command
    HAL_UART_Transmit(&huart5, (uint8_t*)cmd, strlen(cmd), 1000);

    // Wait a bit for response to start
    HAL_Delay(100);

    // Receive response with longer timeout
    HAL_UART_Receive(&huart5, (uint8_t*)response, resp_size-1, 2000);

    // Ensure null termination
    response[resp_size-1] = '\0';
}


void GSM_SetBaudRate(uint32_t baudrate)
{
    char cmd[32];
    char resp[64] = {0};

    // Set new baud rate
    snprintf(cmd, sizeof(cmd), "AT+IPR=%lu\r\n", baudrate);
    GSM_SendCommand(cmd, resp, sizeof(resp));

    // Save to non-volatile memory
    GSM_SendCommand("AT&W\r\n", resp, sizeof(resp));


    HAL_Delay(100);
}



void gsm_get_time_from_module(void)
{
    char response[64] = {0};
    // Enable network time sync and save config
    GSM_SendCommand("AT+CLTS=1\r\n", response, sizeof(response));
    GSM_SendCommand("AT&W\r\n", response, sizeof(response));

    GSM_SendCommand("AT+CCLK?\r\n", response, sizeof(response));

    // Find the quote-delimited time string
    char *start = strchr(response, '\"');
    char *end = strrchr(response, '\"');
    if (start && end && (end > start)) {
        strncpy(gsm_time, start + 1, end - start - 1);
        gsm_time[end - start - 1] = '\0';
    }
}


void gsm_get_imei(void)
{
    char resp[128] = {0};
    GSM_SendCommand_Extended("AT+CGSN\r\n", resp, sizeof(resp),2000);
    //HAL_Delay(1000);

    // Find the first digit sequence in the response
    char *p = resp;
    while (*p && !(*p >= '0' && *p <= '9')) {
        p++;
    }

    if (*p) {
        char *end = p;
        while (*end >= '0' && *end <= '9') {
            end++;
        }

        int imei_len = end - p;
        if (imei_len > 0 && imei_len < sizeof(imei)) {
            strncpy(imei, p, imei_len);
            imei[imei_len] = '\0';
            imeireceived = 1;
        }
    }
}


int signal_strength(void)
{
    char response[64] = {0};
    int rssi = -1, ber = -1;

    GSM_SendCommand("AT+CSQ\r\n", response, sizeof(response));

    char *ptr = strstr(response, "+CSQ:");
    if (ptr)
    {
        // Parse two integers: rssi and ber
        if (sscanf(ptr, "+CSQ: %d,%d", &rssi, &ber) == 2)
        {
            strength = rssi;
            return strength;
        }
    }
    return -1; // failed
}


int signal_strength_improved(void)
{
    char response[128] = {0};
    int rssi = -1, ber = -1;

    // Send command with improved function
    GSM_SendCommand_Improved("AT+CSQ\r\n", response, sizeof(response));

    // Look for the response pattern
    char *ptr = strstr(response, "+CSQ:");
    if (ptr)
    {
        // Try to parse the response
        if (sscanf(ptr, "+CSQ: %d,%d", &rssi, &ber) == 2)
        {
            // Validate RSSI value (should be 0-31, 99 for unknown)
            if (rssi >= 0 && rssi <= 31)
            {
                strength = rssi;
                return rssi;
            }
            else if (rssi == 99)
            {
                strength = -1; // Unknown signal strength
                return -1;
            }
        }
    }

    strength = -1;
    return -1; // Failed to get signal strength
}

int gsm_get_battery_voltage(void)
{
    char response[64] = {0};
    int battery_mv = -1;

    GSM_SendCommand("AT+CBC\r\n", response, sizeof(response));

    char *ptr = strstr(response, "+CBC:");
    if (ptr)
    {
        int bcs, bcl;
        if (sscanf(ptr, "+CBC: %d,%d,%d", &bcs, &bcl, &battery_mv) == 3)
        {
            return battery_mv;
        }
    }
    return battery_mv;
}


void Setup_SMS_Mode(void)
{
    char resp[256] = {0};
    // Set SMS text mode
    GSM_SendCommand_Extended("AT+CMGF=1\r\n", resp, sizeof(resp), 5);
    HAL_Delay(100);
}


//  Extract_Config_From_SMS_Improved function with password verification
int Extract_Config_From_SMS_Improved(char* sms_content)
{
    //GSM_Debug_Print("Analyzing SMS content for config...\r\n");

    // Make content uppercase for easier matching
    char upper_content[512];
    strncpy(upper_content, sms_content, sizeof(upper_content) - 1);
    upper_content[sizeof(upper_content) - 1] = '\0';

    // Convert to uppercase
    for (int i = 0; upper_content[i]; i++) {
        if (upper_content[i] >= 'a' && upper_content[i] <= 'z') {
            upper_content[i] = upper_content[i] - 'a' + 'A';
        }
    }

    // Look for CONFIG keyword (case insensitive)
    char *config_ptr = strstr(upper_content, "CONFIG");

    if (!config_ptr) {
        //GSM_Debug_Print("No CONFIG keyword found\r\n");
        return -1;
    }

    //GSM_Debug_Print("CONFIG keyword found!\r\n");

    // Extract password first and verify it
    char extracted_password[8] = {0};  // 4 digits + null terminator + some buffer
    char *password_ptr = strstr(upper_content, "PASSWORD=");

    if (!password_ptr) {
        //GSM_Debug_Print("No PASSWORD found in SMS\r\n");
        return -1;
    }

    // Extract password from SMS
    char *equals = strchr(password_ptr, '=');
    if (equals) {
        equals++; // Skip '='
        char *end = equals;

        // Find end of password (space, comma, or end of string)
        while (*end && *end != ' ' && *end != ',' && *end != '\r' && *end != '\n') {
            end++;
        }

        size_t len = end - equals;
        if (len != 4) {
           // GSM_Debug_Print("Invalid password length - must be exactly 4 digits\r\n");
            return -1;
        }

        // Copy from original content (not uppercase) to preserve case
        size_t offset = equals - upper_content;
        strncpy(extracted_password, sms_content + offset, len);
        extracted_password[len] = '\0';


        // Verify password contains only digits
        for (int i = 0; i < 4; i++) {
            if (extracted_password[i] < '0' || extracted_password[i] > '9') {
              //  GSM_Debug_Print("Invalid password format - must contain only digits\r\n");
                return -1;
            }
        }

      //  GSM_Debug_Print("4-digit password extracted and validated\r\n");
    }

    // Verify password against default (must be exactly 4 digits)
    if (strcmp(extracted_password, DEFAULT_PASSWORD) != 0) {
      //  GSM_Debug_Print("4-digit password verification FAILED - Config not applied\r\n");
        return -1;  // Password doesn't match, reject the config
    }

    //GSM_Debug_Print("4-digit password verification SUCCESS - Applying config\r\n");

    // Password verified, now extract other parameters
    char *ip_ptr = strstr(upper_content, "IP=");
    if (!ip_ptr) ip_ptr = strstr(upper_content, "SERVER=");
    if (!ip_ptr) ip_ptr = strstr(upper_content, "HOST=");

    char *port_ptr = strstr(upper_content, "PORT=");
    char *refresh1_ptr = strstr(upper_content, "REFRESHRATE1=");
    char *refresh2_ptr = strstr(upper_content, "REFRESHRATE2=");
    char *thresh_ptr = strstr(upper_content, "THRESHOLD=");
    if (!thresh_ptr) thresh_ptr = strstr(upper_content, "THRESH=");

    // Extract IP
    if (ip_ptr) {
        // Find the '=' character
        char *equals = strchr(ip_ptr, '=');
        if (equals) {
            equals++; // Skip '='
            char *end = equals;

            // Find end of IP (space, comma, or end of string)
            while (*end && *end != ' ' && *end != ',' && *end != '\r' && *end != '\n') {
                end++;
            }

            size_t len = end - equals;
            if (len > 0 && len < sizeof(systemConfig.serverIP)) {
                // Copy from original content (not uppercase)
                size_t offset = equals - upper_content;
                strncpy(systemConfig.serverIP, sms_content + offset, len);
                systemConfig.serverIP[len] = '\0';

              //  GSM_Debug_Print("IP found: ");
              //  GSM_Debug_Print(systemConfig.serverIP);
              //  GSM_Debug_Print("\r\n");
            }
        }
    }

    // Extract Port
    if (port_ptr) {
        char *equals = strchr(port_ptr, '=');
        if (equals) {
            systemConfig.serverPort = (uint16_t)atoi(equals + 1);

           // char buf[32];
           // sprintf(buf, "Port found: %d\r\n", systemConfig.serverPort);
           // GSM_Debug_Print(buf);
        }
    }

    // Store the verified password in config (optional)
    strncpy(systemConfig.password, extracted_password, sizeof(systemConfig.password) - 1);
    systemConfig.password[sizeof(systemConfig.password) - 1] = '\0';

    // Extract Refresh Rate 1
    if (refresh1_ptr) {
        char *equals = strchr(refresh1_ptr, '=');
        if (equals) {
            systemConfig.refreshTime1 = (uint16_t)atoi(equals + 1);

          //  char buf[32];
          //  sprintf(buf, "RefreshRate1 found: %d\r\n", systemConfig.refreshTime1);
            //GSM_Debug_Print(buf);
        }
    }

    // Extract Refresh Rate 2
    if (refresh2_ptr) {
        char *equals = strchr(refresh2_ptr, '=');
        if (equals) {
            systemConfig.refreshTime2 = (uint16_t)atoi(equals + 1);

           // char buf[32];
            //sprintf(buf, "RefreshRate2 found: %d\r\n", systemConfig.refreshTime2);
            //GSM_Debug_Print(buf);
        }
    }

    // Extract Threshold
    if (thresh_ptr) {
        char *equals = strchr(thresh_ptr, '=');
        if (equals) {
            systemConfig.threshold = (uint16_t)atoi(equals + 1);

            //char buf[32];
           // sprintf(buf, "Threshold found: %d\r\n", systemConfig.threshold);
           // GSM_Debug_Print(buf);
        }
    }

    return 0; // Success - password verified and config applied
}


// Function to extract sender's phone number from SMS
int Extract_Sender_Number(char* sms_data) {
    //GSM_Debug_Print("Extracting sender number from SMS data...\r\n");

    // Look for +CMGL: pattern which contains sender info
    char *cmgl_ptr = strstr(sms_data, "+CMGL:");
    if (!cmgl_ptr) {
       // GSM_Debug_Print("No +CMGL: pattern found\r\n");
        return -1;
    }

    //GSM_Debug_Print("Found +CMGL: pattern\r\n");


    int comma_count = 0;
    char *current_pos = cmgl_ptr;

    // Find the 3rd comma (after index and status)
    while (*current_pos && comma_count < 2) {
        if (*current_pos == ',') {
            comma_count++;
        }
        current_pos++;
    }

    if (comma_count < 2) {
        //GSM_Debug_Print("Not enough commas found in SMS format\r\n");
        return -1;
    }

    // Now find the opening quote of sender number
    char *quote1 = strchr(current_pos, '"');
    if (!quote1) {
       // GSM_Debug_Print("Opening quote for sender number not found\r\n");
        return -1;
    }
    quote1++; // Skip opening quote

    // Find closing quote
    char *quote2 = strchr(quote1, '"');
    if (!quote2) {
        //GSM_Debug_Print("Closing quote for sender number not found\r\n");
        return -1;
    }

    // Extract phone number
    size_t num_len = quote2 - quote1;
    if (num_len > 0 && num_len < sizeof(sender_number)) {
        strncpy(sender_number, quote1, num_len);
        sender_number[num_len] = '\0';

       // GSM_Debug_Print("Sender number extracted: ");
       // GSM_Debug_Print(sender_number);
       // GSM_Debug_Print("\r\n");
        return 0;
    } else {
       // GSM_Debug_Print("Invalid sender number length\r\n");
        return -1;
    }
}

// Function to send SMS confirmation
void Send_SMS_Confirmation(char* phone_number, int config_result) {
    char resp[256] = {0};
    char sms_cmd[64] = {0};
    char confirmation_msg[200] = {0};

    //GSM_Debug_Print("Starting SMS confirmation process...\r\n");

    // 1. Set SMS text mode (AT+CMGF=1)
    GSM_SendCommand_Extended("AT+CMGF=1\r\n", resp, sizeof(resp), 1000);
//    GSM_Debug_Print("SMS mode set: ");
//    GSM_Debug_Print(resp);
//    GSM_Debug_Print("\r\n");
    HAL_Delay(100);

    // 2. Set destination phone number (AT+CMGS="phone_number")
    snprintf(sms_cmd, sizeof(sms_cmd), "AT+CMGS=\"%s\"\r\n", phone_number);
//    GSM_Debug_Print("Sending AT command: ");
//    GSM_Debug_Print(sms_cmd);
    GSM_SendCommand_Extended(sms_cmd, resp, sizeof(resp), 1000);
//    GSM_Debug_Print("CMGS response: ");
//    GSM_Debug_Print(resp);
//    GSM_Debug_Print("\r\n");
    HAL_Delay(500);

    // Create confirmation message based on result
    if (config_result == 0) {
        // Success message
        snprintf(confirmation_msg, sizeof(confirmation_msg),
                "CONFIG SUCCESS\r\n"
                "Password: VERIFIED\r\n"
                "IP: %s\r\n"
                "Port: %d\r\n"
                "Refresh1: %d\r\n"
                "Refresh2: %d\r\n"
                "Threshold: %d\r\n"
                "Device: %s",
                systemConfig.serverIP,
                systemConfig.serverPort,
                systemConfig.refreshTime1,
                systemConfig.refreshTime2,
                systemConfig.threshold,
                imei);
    } else {
        // Failure message
        snprintf(confirmation_msg, sizeof(confirmation_msg),
                "CONFIG FAILED\r\n"
                "Reason: Invalid password or format\r\n"
                "Required: 4-digit password\r\n"
                "Device: %s",
                imei);
    }

    // 3. Send the actual message content (not an AT command, just text)
//    GSM_Debug_Print("Sending SMS content: ");
//    GSM_Debug_Print(confirmation_msg);
//    GSM_Debug_Print("\r\n");
    HAL_UART_Transmit(&huart5, (uint8_t*)confirmation_msg, strlen(confirmation_msg), 1000);
    HAL_Delay(100);

    // 4. Send Ctrl+Z (ASCII 26) to actually send the SMS
//    GSM_Debug_Print("Sending Ctrl+Z to complete SMS...\r\n");
    char ctrl_z = 0x1A;
    HAL_UART_Transmit(&huart5, (uint8_t*)&ctrl_z, 1, 1000);
    HAL_Delay(2000);

//    GSM_Debug_Print("SMS confirmation sent to: ");
//    GSM_Debug_Print(phone_number);
//    GSM_Debug_Print("\r\n");
}

// Modified Check_Incoming_SMS_Improved function
void Check_Incoming_SMS_Improved(void)
{
    char resp[1024] = {0};

    // First, set up SMS mode
    Setup_SMS_Mode();

    // List all SMS messages with extended timeout
    memset(resp, 0, sizeof(resp));
    GSM_SendCommand_Extended("AT+CMGL=\"REC UNREAD\"\r\n", resp, sizeof(resp), 3000);
    HAL_Delay(1000);

    // Copy response into sms_buffer
    strncpy(sms_buffer, resp, sizeof(sms_buffer) - 1);
    sms_buffer[sizeof(sms_buffer) - 1] = '\0';

    // Debug: Print raw SMS data
//    GSM_Debug_Print("=== SMS CHECK START ===\r\n");
//    GSM_Debug_Print("SMS Buffer Length: ");
//    char len_buf[16];
//    sprintf(len_buf, "%d\r\n", (int)strlen(sms_buffer));
//    GSM_Debug_Print(len_buf);
//    GSM_Debug_Print("SMS Content:\r\n");
//    GSM_Debug_Print(sms_buffer);
//    GSM_Debug_Print("\r\n=== SMS CHECK END ===\r\n");

    // Extract sender's phone number first
    if (Extract_Sender_Number(sms_buffer) == 0) {
        // Try to parse config from SMS
        int config_result = Parse_SMS_Messages(sms_buffer);

        // Send confirmation SMS back to sender
        Send_SMS_Confirmation(sender_number, config_result);

        // Delete SMS after processing (whether successful or not)
        GSM_SendCommand_Extended("AT+CMGD=1,4\r\n", resp, sizeof(resp), 500);
        //GSM_Debug_Print("SMS deleted after processing\r\n");
    } else {
       // GSM_Debug_Print("Could not extract sender number\r\n");
    }
}

// Modified Parse_SMS_Messages function (simplified since confirmation is now handled in Check_Incoming_SMS_Improved)
int Parse_SMS_Messages(char* sms_data)
{
//    GSM_Debug_Print("RAW SMS:\r\n");
//    GSM_Debug_Print(sms_data);
//    GSM_Debug_Print("\r\n");

    int result = Extract_Config_From_SMS_Improved(sms_data);

    if (result == 0)
    {
//        GSM_Debug_Print("Config successfully updated from SMS:\r\n");
//        GSM_Debug_Print("IP: ");
//        GSM_Debug_Print(systemConfig.serverIP);
//
//        GSM_Debug_Print("\r\nPort: ");
//        char buf[32];
//        sprintf(buf, "%d\r\n", systemConfig.serverPort);
//        GSM_Debug_Print(buf);
//
//        GSM_Debug_Print("Password: VERIFIED\r\n");
//
//        sprintf(buf, "Refresh1: %d\r\n", systemConfig.refreshTime1);
//        GSM_Debug_Print(buf);
//
//        sprintf(buf, "Refresh2: %d\r\n", systemConfig.refreshTime2);
//        GSM_Debug_Print(buf);
//
//        sprintf(buf, "Threshold: %d\r\n", systemConfig.threshold);
//        GSM_Debug_Print(buf);

        Test_StringEEPROM();
        return 0; // Success
    }
    else
    {
        //GSM_Debug_Print("Config update REJECTED - Invalid password or format\r\n");
        return -1; // Failed
    }
}
void GPRS_Send_JSON(void)
{
    char resp[512] = {0};

    // Create fresh JSON data
    Create_JSON_String();

    // 1. Set bearer profile
    GSM_SendCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    GSM_SendCommand("AT+SAPBR=3,1,\"APN\",\"internet\"\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    GSM_SendCommand("AT+SAPBR=1,1\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    GSM_SendCommand("AT+SAPBR=2,1\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    // 2. HTTP Init
    GSM_SendCommand("AT+HTTPINIT\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    GSM_SendCommand("AT+HTTPPARA=\"CID\",1\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    // 3. Set URL
    char http_url[256] = {0};
    snprintf(http_url, sizeof(http_url), "AT+HTTPPARA=\"URL\",\"http://%s/api/test\"\r\n",
             systemConfig.serverIP);
    GSM_SendCommand(http_url, resp, sizeof(resp));
    HAL_Delay(5);

    // 4. Set content type to JSON
    GSM_SendCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", resp, sizeof(resp));
    HAL_Delay(5);

    // 5. Send data length
    char data_len_cmd[64];
    snprintf(data_len_cmd, sizeof(data_len_cmd), "AT+HTTPDATA=%d,1000\r\n", (int)strlen(json_string));
    GSM_SendCommand(data_len_cmd, resp, sizeof(resp));
    HAL_Delay(delay);

    // 6. Send actual JSON data
    HAL_UART_Transmit(&huart5, (uint8_t*)json_string, strlen(json_string), HAL_MAX_DELAY);
    HAL_Delay(200);

    // 7. HTTP POST
    GSM_SendCommand("AT+HTTPACTION=1\r\n", resp, sizeof(resp));
    HAL_Delay(500);

    // 8. End HTTP
    GSM_SendCommand("AT+HTTPTERM\r\n", resp, sizeof(resp));
    HAL_Delay(10);
}


