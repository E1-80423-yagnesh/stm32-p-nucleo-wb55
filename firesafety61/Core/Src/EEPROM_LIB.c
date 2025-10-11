//---------------------------------------------------------------------------------------------------------------
/*!
 * @File          	EEPROM_LIB.c
 *
 * @Copyright    		(c)	2023 Aartronix Innovation Private Limited(AIPL). All rights resenved. This sotware
 *		      				constitutes the trade secrets and confudential and proprierty information of AIPL.
 *		      				It is intended solely for use by AIPL.This code not be copied or redistributed to
 *                	third parties without prior written authorization from AIPL.
 *
 * @Discription     STM32F410RBt6 interface with EEPROM using I2C.
 *
 * @Revision 				History
 *
 * Date             Author              Project           Reason for change
 * -------------------------------------------------------------------------------------------------------------
 * 29/04/2020       Kalpesh Gajera      POC               Created
 * 01/05/2024       Shyam Parmar        POC               for standard format
 */
//---------------------------------------------------------------------------------------------------------------

/*includes-------------------------------------------------------------------------------------------------*/
#include "main.h"
#include "EEPROM_LIB.h"

/*global variable------------------------------------------------------------------------------------------*/

uint8_t gTxB[2]    = {'\0'};      //To store Transmit data
uint8_t gRxB[2]    = {'\0'};      //To store Receive data


//to store calibration config
char A0_A9_factStr[10];
char V_factStr[10];
char I_factStr[10];

char readA0_A9_fact[10];
char readV_fact[10];
char readI_fact[10];

//to store sms config
char serverIPstr[20];
char serverPortStr[6];
char refreshTime1Str[6];
char refreshTime2Str[6];
char thresholdStr[6];
char passwordStr[20];

char READIP[16] = {0};
char readpassword[16]={0};
char READPORT[5] = {0};
char time1[3] = {0};
char time2[3] = {0};
char readthreshold[4] = {0};

EEPROM_ID eepromId;

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for write EEPROM byte.
 *  @note        none
 *  @param       byteAdress,Eeprombyte
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void WriteEepromByte(uint8_t byteAddress, uint8_t eepromByte)
{
	gTxB[0] = byteAddress;   //Store byte address
	gTxB[1] = eepromByte;    //Store data

  I2C_TRANSMIT;               //Transmit data
}

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for Read EEPROM byte.
 *  @note        none
 *  @param       byteAddress
 *  @return      return received data(gRxBuff[0])
 -----------------------------------------------------------------------------------------------------*/
uint8_t ReadEepromByte(uint8_t byteAddress)
{
	gTxB[0] = byteAddress;   //Store byte address

	I2C_TRANSMIT2;              //Transmit data
  I2C_RECEIVE;                //Receive data

	return gRxB[0];
}

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for write EEPROM multibyte.
 *  @note        none
 *  @param       byteAddress, eeprombytepointer, nobyte
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void WriteEepromMultiByte(uint8_t byteAddress, uint8_t* eepromBytePointer, uint8_t noByte)
{
	gTxB[0] = byteAddress;                          //Store byte address

	for(uint8_t Index = 0; Index < noByte; Index++)
		gTxB[Index + 1] = eepromBytePointer[Index];   //Store multibyte data

	I2C_TRANSMIT3;	                                   //Transmit data

	__NOP();
}

/*!-----------------------------------------------------------------------------------------------------
 *  @brief       This function use for Read EEPROM multibyte.
 *  @note        none
 *  @param       byteAddress, eeprombytepointer, nobyte
 *  @return      none
 -----------------------------------------------------------------------------------------------------*/
void ReadEepromMultiByte(uint8_t byteAddress, uint8_t* eepromBytePointer, uint8_t noByte)
{

	gTxB[0] = byteAddress;     //Store byte address

  I2C_TRANSMIT2;	              //Transmit data

  I2C_RECEIVE2;                 //Receive data

}



//////////////EEPROM function ///////////////////////////////////////////////////

void Test_StringEEPROM(void)
{
    uint8_t address1 = 0x30;
    uint8_t address2 = 0x40;
    uint8_t address3 = 0x50;
    uint8_t address4 = 0x60;
    uint8_t address5 = 0x70;
    uint8_t address6 = 0x80;  // Password

    sprintf(serverIPstr,"%s",systemConfig.serverIP);
    sprintf(serverPortStr,"%d",systemConfig.serverPort);
    sprintf(refreshTime1Str,"%d",systemConfig.refreshTime1);
    sprintf(refreshTime2Str,"%d",systemConfig.refreshTime2);
    sprintf(thresholdStr,"%d",systemConfig.threshold);
    sprintf(passwordStr,"%s",systemConfig.password);

    uint8_t length1 = strlen(serverIPstr) + 1;
    uint8_t length2 = strlen(serverPortStr) + 1;
    uint8_t length3 = strlen(refreshTime1Str) + 1;
    uint8_t length4 = strlen(refreshTime2Str) + 1;
    uint8_t length5 = strlen(thresholdStr) + 1;
    uint8_t length6 = strlen(passwordStr) + 1;

    // Clear read buffers
   // memset(READIP, 0, sizeof(READIP));
    memset(READPORT, 0, sizeof(READPORT));
    memset(time1, 0, sizeof(time1));
    memset(time2, 0, sizeof(time2));
    memset(readthreshold, 0, sizeof(readthreshold));
    memset(readpassword, 0, sizeof(readpassword));


    WriteEepromMultiByte(address1, (uint8_t*)serverIPstr, length1);
    ReadEepromMultiByte(address1, (uint8_t*)READIP, length1);

    // Write and Read Server Port
    WriteEepromMultiByte(address2, (uint8_t*)serverPortStr, length2);
    HAL_Delay(10);
    ReadEepromMultiByte(address2, (uint8_t*)READPORT, length2);
    HAL_Delay(10);

    // Write and Read Refresh Time 1
    WriteEepromMultiByte(address3, (uint8_t*)refreshTime1Str, length3);
    HAL_Delay(10);
    ReadEepromMultiByte(address3, (uint8_t*)time1, length3);
    HAL_Delay(10);

    // Write and Read Refresh Time 2
    WriteEepromMultiByte(address4, (uint8_t*)refreshTime2Str, length4);
    HAL_Delay(10);
    ReadEepromMultiByte(address4, (uint8_t*)time2, length4);
    HAL_Delay(10);

    // Write and Read Threshold
    WriteEepromMultiByte(address5, (uint8_t*)thresholdStr, length5);
    HAL_Delay(10);
    ReadEepromMultiByte(address5, (uint8_t*)readthreshold, length5);
    HAL_Delay(10);

    WriteEepromMultiByte(address6, (uint8_t*)passwordStr, length6);
    HAL_Delay(10);
    ReadEepromMultiByte(address6, (uint8_t*)readpassword, length6);
    HAL_Delay(10);

    // Convert strings back to structure values
    // Check if IP address is valid (not empty)
    if (strlen(READIP) > 0) {
        strcpy(systemConfig.serverIP, READIP);
    }

    // Convert port string to integer
    if (strlen(READPORT) > 0) {
        systemConfig.serverPort = atoi(READPORT);
        // Validate port range
        if (systemConfig.serverPort < 1 || systemConfig.serverPort > 65535) {
            systemConfig.serverPort = 4300; // Default value
        }
    }

    // Convert refresh time 1 string to integer
    if (strlen(time1) > 0)
    {
        systemConfig.refreshTime1 = atoi(time1);
        // Validate refresh time (should be positive)
        if (systemConfig.refreshTime1 <= 0)
        {
            systemConfig.refreshTime1 = 10; // Default value
        }
    }

    // Convert refresh time 2 string to integer
    if (strlen(time2) > 0)
    {
        systemConfig.refreshTime2 = atoi(time2);
        // Validate refresh time (should be positive)
        if (systemConfig.refreshTime2 <= 0)
        {
            systemConfig.refreshTime2 = 20; // Default value
        }
    }

    // Convert threshold string to integer
    if (strlen(readthreshold) > 0)
    {
        systemConfig.threshold = atoi(readthreshold);
        // Validate threshold (should be positive)
        if (systemConfig.threshold <= 0)
        {
            systemConfig.threshold = 5; // Default value
        }
    }

    // Copy password
     if (strlen(readpassword) > 0) {
         strcpy(systemConfig.password, readpassword);
     }
}
// Add this function to load configuration from EEPROM back to structure
int Load_Config_From_EEPROM(void)
{
    uint8_t address1 = 0x30;
    uint8_t address2 = 0x40;
    uint8_t address3 = 0x50;
    uint8_t address4 = 0x60;
    uint8_t address5 = 0x70;
    uint8_t address6 = 0x80;

    // Clear read buffers
    memset(READIP, 0, sizeof(READIP));
    memset(READPORT, 0, sizeof(READPORT));
    memset(time1, 0, sizeof(time1));
    memset(time2, 0, sizeof(time2));
    memset(readthreshold, 0, sizeof(readthreshold));
    memset(readpassword, 0, sizeof(readpassword));

    // Read values from EEPROM
    ReadEepromMultiByte(address1, (uint8_t*)READIP, sizeof(READIP)-1);
    READIP[sizeof(READIP)-1] = '\0';

    ReadEepromMultiByte(address2, (uint8_t*)READPORT, sizeof(READPORT)-1);
    READPORT[sizeof(READPORT)-1] = '\0';

    ReadEepromMultiByte(address3, (uint8_t*)time1, sizeof(time1)-1);
    time1[sizeof(time1)-1] = '\0';

    ReadEepromMultiByte(address4, (uint8_t*)time2, sizeof(time2)-1);
    time2[sizeof(time2)-1] = '\0';

    ReadEepromMultiByte(address5, (uint8_t*)readthreshold, sizeof(readthreshold)-1);
    readthreshold[sizeof(readthreshold)-1] = '\0';

    // Read password from EEPROM (new)
    ReadEepromMultiByte(address6, (uint8_t*)readpassword, sizeof(readpassword)-1);
    readpassword[sizeof(readpassword)-1] = '\0';

    // Convert strings back to structure values
    if (strlen(READIP) > 0) {
        strcpy(systemConfig.serverIP, READIP);
    }

    if (strlen(READPORT) > 0)
    {
        systemConfig.serverPort = atoi(READPORT);
        if (systemConfig.serverPort < 1 || systemConfig.serverPort > 65535)
        {
            systemConfig.serverPort = 4300;
        }
    }

    if (strlen(time1) > 0)
    {
        systemConfig.refreshTime1 = atoi(time1);
        if (systemConfig.refreshTime1 <= 0)
        {
            systemConfig.refreshTime1 = 1;
        }
    }

    if (strlen(time2) > 0)
    {
        systemConfig.refreshTime2 = atoi(time2);
        if (systemConfig.refreshTime2 <= 0)
        {
            systemConfig.refreshTime2 = 1;
        }
    }

    if (strlen(readthreshold) > 0)
    {
        systemConfig.threshold = atoi(readthreshold);
        if (systemConfig.threshold <= 0)
        {
            systemConfig.threshold = 1;
        }
    }

    // Copy password back to structure (new)
     if (strlen(readpassword) > 0)
     {
         strcpy(systemConfig.password, readpassword);
     }
     else
     {
         // Set default password
         strcpy(systemConfig.password, "default123");
     }

    return 1; // Success
}




int Load_Calib_From_EEPROM(void)
{
    uint8_t addrA0 = 0x90;
    uint8_t addrV  = 0xA0;
    uint8_t addrI  = 0xB0;

    // Clear read buffers
    memset(readA0_A9_fact, 0, sizeof(readA0_A9_fact));
    memset(readV_fact, 0, sizeof(readV_fact));
    memset(readI_fact, 0, sizeof(readI_fact));

    // Read FULL buffer size (not strlen of empty strings!)
    ReadEepromMultiByte(addrA0, (uint8_t*)readA0_A9_fact, sizeof(readA0_A9_fact)-1);
    HAL_Delay(10);
    ReadEepromMultiByte(addrV, (uint8_t*)readV_fact, sizeof(readV_fact)-1);
    HAL_Delay(10);
    ReadEepromMultiByte(addrI, (uint8_t*)readI_fact, sizeof(readI_fact)-1);
    HAL_Delay(10);

    // Validate and convert with defaults
    float temp_A0 = atof(readA0_A9_fact);
    float temp_V = atof(readV_fact);
    float temp_I = atof(readI_fact);

    // Use only if valid (non-zero and reasonable range)
    if (temp_A0 > 0.0 && temp_A0 < 10.0)
        calib.A0_A9_fact = temp_A0;
    else
        calib.A0_A9_fact = 0.966; // Default if invalid

    if (temp_V > 0.0 && temp_V < 10.0)
        calib.V_fact = temp_V;
    else
        calib.V_fact = 0.188; // Default if invalid

    if (temp_I > 0.0 && temp_I < 10.0)
        calib.I_fact = temp_I;
    else
        calib.I_fact = 0.051; // Default if invalid

    return 1;
}




void Save_CalibrationEEPROM(void)
{
    uint8_t addrA0 = 0x90;
    uint8_t addrV  = 0xA0;
    uint8_t addrI  = 0xB0;

    // Convert float values to strings
    sprintf(A0_A9_factStr, "%.6f", calib.A0_A9_fact);
    sprintf(V_factStr, "%.6f", calib.V_fact);
    sprintf(I_factStr, "%.6f", calib.I_fact);

    uint8_t lenA0 = strlen(A0_A9_factStr) + 1;
    uint8_t lenV  = strlen(V_factStr) + 1;
    uint8_t lenI  = strlen(I_factStr) + 1;

    // ONLY WRITE - no reading!
    WriteEepromMultiByte(addrA0, (uint8_t*)A0_A9_factStr, lenA0);
    HAL_Delay(10);
    WriteEepromMultiByte(addrV, (uint8_t*)V_factStr, lenV);
    HAL_Delay(10);
    WriteEepromMultiByte(addrI, (uint8_t*)I_factStr, lenI);
    HAL_Delay(10);
}


