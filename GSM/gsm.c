/*
 * gsm.c
 *
 *  Created on: Dec 13, 2019
 *      Author: kennedyotieno
 */

#include "gsm.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "../Systick/gsm_systick.h"
#include "string.h"
#include "main.h"
#include "stdlib.h"
#include <stdio.h>

#define MAX_READ_DATA 512

enum gsmState { Idle, MessageReceived, SMSReceived };

char gsm_data_buffer[MAX_READ_DATA] = { 0 };
volatile uint16_t read_data_buffer  = 0;
volatile uint16_t state             = Idle;
volatile uint32_t capture_mask      = 0;

extern tcpObject tcpConnectionObject;
extern tcpObject secondaryConnectionObject;
extern TCP_IP_STATE tcpIpConnectionState;

uint8_t phoneNumber[14];
uint8_t smsMessage[170];

void gsmSplitString(uint8_t *charArray, char delimiter, int length, uint8_t *position)
{
    int counter,pos = 0;
    for(;counter < length ;)
    {
        if(*charArray == delimiter)
        {
            position[pos++] = counter;
        }
        counter++;
        charArray++;
    }
}

void gsmSplitStringWithDelims(uint8_t *charArray, char delimiter_1, char delimiter_2, int length, uint8_t *position)
{
    int counter, pos = 0;
    uint8_t accum = 0;
    for(; counter < length ;)
    {
        if((*charArray == delimiter_2) && (accum == delimiter_1))
        {
            position[pos++] = counter;
        }
        counter++;
        accum = *charArray;
        charArray ++;
    }
}

void gsmCopyString(uint8_t *src, uint8_t *dst, int maximum, int start_pos, int end_pos)
{
    int counter;
    src +=start_pos;
    for(counter = start_pos; counter < end_pos ;counter++)
    {
        *dst = *src;
        src++;
        dst++;
        if((counter - start_pos) > maximum)
        {
            return;
        }
    }
}

uint32_t gsmParseInt(uint8_t *charArray, int start_pos, int end_pos)
{
    char string_value[11] = { 0 };
    int counter,index = 0;
    char *end;

    charArray += start_pos;
    for(counter = start_pos; counter < end_pos; counter++)
    {
        string_value[index] = *charArray++;
        index++;
    }
    string_value[index] = 0;
    index++;
    
    return strtol(string_value, &end, 10);
}

uint8_t initializeSIMModule()
{
    static int counter = 0;
    if (sendATCommand("AT", sizeof("AT"), "OK", 300) == 0)
    {
        counter++;
        if (counter > 20)
        {
            serialPrint("Unable to initialize GSM \r\n");
            serialPrint("Restart device \r\n");
            // TODO: Document this behavior
            //startSIMModule(GPIOC, GPIO_PIN, 3000);
            return 0;
        }   
    }
    serialPrint("SIM Initialized successfully\r\n");
    if (sendATCommand("ATE0", sizeof("ATE0"), "OK\r\n", 20))
    {
        serialPrint("No Echo \r\n");
    }
    if (sendATCommand("AT+CGMF=1", sizeof("AT+CGMF=1"), "OK\r\n", 20))
    {
        serialPrint("Text mode saved as message format \r\n");
    }
    if (sendATCommand("AT&W", sizeof("AT&W"), "OK\r\n", 20))
    {
        serialPrint("Settings saved \r\n");
    }
    if (sendATCommand("AT+GMR", sizeof("AT+GMR"), "OK\r\n", 20))
    {
        serialPrint("TA Software Revision identified successfully\r\n");
    }
    state = Idle;
    return 1;
}

uint8_t gsmKeepAlive()
{
    if (sendATCommand("AT \r", sizeof("AT \r"), "OK", 30))
    {
        command_failed_count = 0;
        if (!checkSIMNetworkState())
        {
            sim_disconnected++;
            int net_state = checkSIMNetworkState();
            serialPrint("Network state: %i \r\n", net_state);
        }
        else
        {
            sim_disconnected = 0;
        }
        if (sim_disconnected > 20)
        {
            gsmModuleState = On;
            sim_disconnected = 0;
            return 0;
        } 
    }else {
        command_failed_count++;
        if (command_failed_count > 10)
        {
            serialPrint("Can not communicate with module \r\n");
            gsmModuleState = Off;
            command_failed_count = 0;
            return 0;
        }
    }
    return 0;
}

uint8_t checkSIMNetworkState()
{
    if ((sendATCommand("AT+CREG?",sizeof("AT+CREG?"),"+CREG: 0,1", 20)))
    {
        serialPrint("Network state: Registered to home network\r\n");
        return 1;
    }
    if ((sendATCommand("AT+CREG?",sizeof("AT+CREG?"), "+CREG: 0,5", 20)))
    {
        serialPrint("Network state: Registered but roaming\r\n");
        return 1;
    }
    return 0;
}

uint8_t sendATCommand(void *command, int data_size, char *response, int timeout)
{
    char control_character = "\r";
    char *control_character_ptr = &control_character;
    read_data_buffer = 0;
    memset(&gsm_data_buffer[0], 0, MAX_READ_DATA);
    UART2Send((uint8_t *)command, (data_size - 1));
    UART2Send((uint8_t *)control_character, 1);
    state = Idle;
    gsmTick = 0;
    while (gsmTick < timeout)
    {
        while (UART2Probe())
        {
            if (read_data_buffer < MAX_READ_DATA)
            {
                gsm_data_buffer[read_data_buffer++] =  UART2GetChar();
            }
            else
            {
                read_data_buffer = 0;
            }
            state = MessageReceived;
            HAL_Delay(10);
        }
        if (state == MessageReceived)
        {
            state = Idle;
            if (strstr(gsm_data_buffer, response) != NULL)
            {
                read_data_buffer = 0;
                return 1;
            }
        }
    }
    read_data_buffer = 0;
    return 0;
}

uint8_t getATCommandReply(char *response, int timeout) 
{
    read_data_buffer = 0;
    memset(&gsm_data_buffer[0], 0, MAX_READ_DATA);
    state = Idle;
    gsmTick = 0;
    while (gsmTick < timeout)
    {
        while (UART2Probe())
        {
            gsm_data_buffer[read_data_buffer++] =  UART2GetChar();
            state = MessageReceived;
            HAL_Delay(10);
        }
        if (state == MessageReceived)
        {
            state = Idle;
            if (strstr(gsm_data_buffer, response) != NULL)
            {
                read_data_buffer = 0;
                return 1;
            }
        }
    }
    read_data_buffer = 0;
    return 0;
}

void resetSIMModule(GPIO_TypeDef *pin_peripheral, uint16_t gsm_pin, int duration)
{
    // Maybe Unused
    HAL_GPIO_WritePin(pin_peripheral, gsm_pin, GPIO_PIN_RESET);
    HAL_Delay(duration);
    HAL_GPIO_WritePin(pin_peripheral,gsm_pin,GPIO_PIN_SET);
    serialPrint("GSM Module Restarted \r\n");
}

void startSIMModule(GPIO_TypeDef *pin_peripheral, uint16_t gsm_pin, int duration)
{
    // Maybe Unused
    HAL_GPIO_WritePin(pin_peripheral, gsm_pin, GPIO_PIN_SET);
    HAL_Delay(duration);
    HAL_GPIO_WritePin(pin_peripheral,gsm_pin,GPIO_PIN_RESET);
    serialPrint("GSM Module Started \r\n");
}

uint8_t setupTCP()
{
    char data_bucket[64];
    uint8_t length;
    if (checkSIMNetworkState())
    {
        if (sendATCommand("AT+CIPRXGET=1", sizeof("AT+CIPRXGET=1"), "OK\r\n", 20))
        {
            serialPrint("Manually gettting data from network enabled \r\n");
        }
        if(sendATCommand("AT+CGATT=1", sizeof("AT+CGATT=1"), "OK\r\n", 3000))
        {
            serialPrint("Successfully attached to network \r\n");
            if (sendATCommand("AT+CIPMUX=1", sizeof("AT+CIPMUX=1"), "OK\r\n", 300))
            {
                serialPrint("Multi-IP Connection started \r\n");
            }
            if (sendATCommand("AT+CIPSHUT", sizeof("AT+CIPSHUT"), "SHUT OK", 300))
            {
                serialPrint("PDP Context decativated \r\n");
                // See -> https://www.tutorialspoint.com/gprs/gprs_pdp_context.htm
            }
            length = snprintf(data_bucket, 64, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r", apn_netw, apn_user, apn_pass);
            if (sendATCommand(data_bucket, length, "OK\r\n", 300))
            {
                serialPrint("APN config set successfully \r\n");
            }
            if (sendATCommand("AT+CIIR", sizeof("AT+CIIR"), "OK\r\n", 10000))
            {
                serialPrint("Wireless connection brought up successfully within 10 seconds \r\n");
                if (sendATCommand("AT+CIFSR", sizeof("AT+CIFSR"), "ERROR", 3000))
                {
                    serialPrint("Unable to get local IP \r\n");
                    return 0;
                } else {
                    serialPrint("Local IP issued \r\n");
                    return 1;
                }
            } else {
                serialPrint("Unable establish wireless connection \r\n");
            }
        }
    }
    return 0;
}

uint8_t writeToTCPSocket()
{
    serialPrint("Sending data \r\n");
    if ("AT+CIPSEND=0,7",sizeof("AT+CIPSEND=0,7"),  ">", 300)
    {
        serialPrint("..\r\n");
        UART2Send((uint8_t *)"HELLO\r\n",7);
        if (getATCommandReply("SEND", 5000))
        {
            serialPrint("Sent data\r\n");
            return 1;
        }else{
            serialPrint("Unable to send data \r\n");
        }
        
    }
    return 0;
}

uint8_t getTCPStatus(int timeout)
{
    uint8_t delimiter[20];
    uint8_t position[20];
    uint8_t accum_max   = 32;
    uint8_t accum[accum_max];
    char *data;

    read_data_buffer = 0;
    memset(&gsm_data_buffer[0], 0, MAX_READ_DATA);
    UART2Send((uint8_t *)"AT+CIPSTATUS \r", sizeof("AT+CIPSTATUS \r"));
    state = Idle;
    gsmTick = 0;
    while (gsmTick < timeout)
    {
        while (UART2Probe())
        {
            if (read_data_buffer < MAX_READ_DATA)
            {
                gsm_data_buffer[read_data_buffer++] = UART2GetChar();
            } else {
                read_data_buffer = 0;
            }
            state = MessageReceived;
            HAL_Delay(10);
        }
        if(state == MessageReceived)
        {
            gsmSplitStringWithDelims((uint8_t *)gsm_data_buffer, '\r', '\n', MAX_READ_DATA, delimiter);
            gsmCopyString((uint8_t *)gsm_data_buffer,accum, accum_max, (delimiter[2]), (delimiter[3]));
            if (strstr((char*)accum, "IP INITIAL"))
            {
                tcpIpConnectionState = IP_INITIAL;
            }
            if (strstr((char*)accum, "IP START"))
            {
                tcpIpConnectionState = IP_START;
            }
            if (strstr((char*)accum, "IP CONFIG"))
            {
                tcpIpConnectionState = IP_CONFIG;
            }
            if (strstr((char*)accum, "IP GPRSACT"))
            {
                tcpIpConnectionState = IP_GPRSACT;
            }
            if (strstr((char*)accum, "IP STATUS"))
            {
                tcpIpConnectionState = IP_STATUS;
            }
            if (strstr((char*)accum, "IP PROCESSING"))
            {
                tcpIpConnectionState = IP_PROCESSING;
            }
            if (strstr((char*)accum, "PDP DEACT"))
            {
                tcpIpConnectionState = PDP_DEACT;
            } 

            data = &gsm_data_buffer[delimiter[4]];
            gsmSplitString((uint8_t *)data, ',', (delimiter[5] - delimiter[4]), position);
            gsmCopyString((uint8_t *)gsm_data_buffer, accum,  accum_max, (delimiter[4] + delimiter[4]), (delimiter[5]));
            if (strstr((char*)accum, "INITIAL"))
            {
                tcpConnectionObject.state = INITIAL;
            }
            if (strstr((char*)accum, "CONNECTING"))
            {
                tcpConnectionObject.state = CONNECTING;
            }
            if (strstr((char*)accum, "CONNECTED"))
            {
                tcpConnectionObject.state = CONNECTED;
            }
            if (strstr((char*)accum, "REMOTE CLOSING"))
            {
                tcpConnectionObject.state = REMOTE_CLOSING;
            }
            if (strstr((char*)accum, "CLOSING"))
            {
                tcpConnectionObject.state = CLOSING;
            }  
            if (strstr((char*)accum, "CLOSED"))
            {
                tcpConnectionObject.state = CLOSED;
            }  

            state = Idle;   
        }
    }
    read_data_buffer = 0;
    return 0;
}

// TODO: Limitation... SIM Pin can not be set
// TODO: SMS ... Add SMS functionalities