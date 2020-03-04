/*
 * gsm.h
 *
 *  Created on: Dec 13, 2019
 *      Author: kennedyotieno
 */

#ifndef GSM_H_
#define GSM_H_

#include "gpio.h"
#include <stdint.h>

typedef enum { 
    INITIAL        = 0,
    CONNECTING     = 1,
    CONNECTED      = 2,
    REMOTE_CLOSING = 3,
    CLOSING        = 4,
    CLOSED         = 5
 } TCP_CLIENT_STATE;

typedef enum {
    IP_INITIAL 		= 0,
	IP_START		= 1,
	IP_CONFIG		= 2,
	IP_GPRSACT		= 3,
	IP_STATUS		= 4,
	IP_PROCESSING	= 5,
	PDP_DEACT 		= 9
 } TCP_IP_STATE;

typedef struct tcpObject {
    uint8_t initialize;
    uint8_t domain[20];
    uint16_t port;
    TCP_CLIENT_STATE state;
} tcpObject;

extern tcpObject tcpConnectionObject;
extern TCP_IP_STATE tcpIpConnectionState;

// Critical SIM Functions

/**
 * @brief Initialize SIM Module
 * 
 * ATE0 Check command echo
 * AT+CMGF=1 Set SMS Format to text
 * AT&W Save settings
 * AT+GMR Check module software revision
 * 
 * @return uint8_t 
 */
uint8_t initializeSIMModule();

/**
 * @brief Reset GSM module
 * 
 * @param pin_peripheral GPIOA|GPIOB|GPIOC
 * @param gsm_pin GSM Pin to be toglled
 * @param duration duration in milliseconds
 */
void resetSIMModule(GPIO_TypeDef *pin_peripheral, uint16_t gsm_pin, int duration);

/**
 * @brief Switch ON GSM module
 * 
 * @param pin_peripheral GPIOA|GPIOB|GPIOC
 * @param gsm_pin GSM Pin to be toglled
 * @param duration duration in milliseconds
 */
void startSIMModule(GPIO_TypeDef *pin_peripheral, uint16_t gsm_pin, int duration);

/**
 * @brief Check Network Registration state
 * 
 * Shows whether the Network has currently indicated Registration
 * AT+CREG=?
 * 
 * +CREG:0,0 Not registered
 * +CREG:0,1 Registered 
 * +CREG:0,2 Not yet registered, searching for operator
 * +CREG:0,3 Registration denied
 * +CREG:0,4 Unknown
 * +CREG:0,5 Registered but roaming
 * 
 * @return uint8_t 
 */
uint8_t checkSIMNetworkState();

// TCP Handlers

/**
 * @brief Initializes a new TCP connection 
 * 
 * Enables data, registers APN credentials, initialize
 * wireless connectivity and IP issuance.
 * 
 * AT+CIPRXGET=1 Enable manual data retrieval mode 
 * AT+CGATT=1 Attaches to network 
 * AT+CIPMUX=1 Enable multi-IP connection 
 * AT+CIPSHUT See -> https://www.tutorialspoint.com/gprs/gprs_pdp_context.htm
 * AT+CSTT Configures APN 
 * AT+CIICR enable wireless connectivity
 * AT+CIFSR obtain local IP 
 * 
 * @return uint8_t 
 */
uint8_t setupTCP();

/**
 * @brief Send data through a TCP connection 
 * 
 * Tests if it is possible to send data through a TCP connection.
 * 
 * @return uint8_t 
 */
uint8_t writeToTCPSocket();

/**
 * @brief Get current connection status
 * 
 * AT+CIPSTATUS returns a STATE: <state> response for Multi-IP connection.
 * 
 * @param timeout 
 * @return uint8_t 
 */
uint8_t getTCPStatus(int timeout);

// /**
//  * @brief 
//  * 
//  * @param index 
//  * @return uint8_t 
//  */
// uint8_t deleteMesssage(int index);

// /**
//  * @brief 
//  * 
//  */
// void initilizeSMS();

/**
 * @brief Checks for the current network Registration status.
 * 
 * Attempts to register to netwrok if state is disconnected
 * 
 * @return uint8_t 
 */
uint8_t gsmKeepAlive();

/**
 * @brief Send AT prepended commands to the SIM module
 * 
 * Each command has a timeout within which an expected response is expected
 * 
 * @param command 
 * @param data_size 
 * @param response 
 * @param timeout 
 * @return uint8_t 
 */
uint8_t sendATCommand(void *command, int data_size, char *response, int timeout);

/**
 * @brief Checks the status of a sent command's response
 * 
 * Check for ocurrence of an a particular set of characters 
 * in the GSM module response message body. If the characters exist
 * then true , false otherwise.
 * 
 * @param response The series of characters expected in the response body
 * @param timeout Timeout in milliseconds
 * @return uint8_t 
 */
uint8_t getATCommandReply(char *response, int timeout);

/**
 * @brief gsmSplitString
 * 
 * @param charArray 
 * @param delimiter 
 * @param length 
 * @param position 
 */
void gsmSplitString(uint8_t *charArray, char delimiter, int length, uint8_t *position);

/**
 * @brief gsmSplitStringWithDelims
 * 
 * @param charArray 
 * @param delimiter_1 
 * @param delimiter_2 
 * @param length 
 * @param position 
 */
void gsmSplitStringWithDelims(uint8_t *charArray, char delimiter_1, char delimiter_2, int length, uint8_t *position);

/**
 * @brief Copy strings 
 * 
 * Copies a string from the source to the destination buckets.
 * The amounnt of characters to be copied are limited
 * 
 * @param src source character array
 * @param dst destination charachter array bucket
 * @param maximum maximum length of string to copy
 * @param start_pos start posistion of the character array to copy
 * @param end_pos end postion of the array to be copied
 */
void gsmCopyString(uint8_t *src, uint8_t *dst, int maximum, int start_pos, int end_pos);

/**
 * @brief Return an integer from  an array of characters
 * 
 * Extracts an usigned integer from a string using a 
 * starting and end postion.
 * 
 * @param charArray the character array bucket to be converted
 * @param start_pos the starting position from which the integer is
 * to extracted
 * @param end_pos the end position to limit the parsing.
 * @return result the unsigned to interger as the result
 */
uint32_t gsmParseInt(uint8_t *charArray, int start_pos, int end_pos);


#endif /* GSM_H_ */
