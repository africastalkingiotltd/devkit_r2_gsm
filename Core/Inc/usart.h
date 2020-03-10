/**
  ******************************************************************************
  * File Name          : USART.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief Initalize and enable UART1
 * 
 */
void UART1FIFOInit(void);

/**
 * @brief Probe UART1
 * 
 * @return int 
 */
int UART1Probe(void);

/**
 * @brief Return a char array from FIFO buffer
 * 
 * @return uint8_t 
 */
uint8_t UART1GetChar(void);

/**
 * @brief Insert character array for UART transmission
 * 
 * @param data 
 */
void UART1PutChar(uint8_t data);

/**
 * @brief IRQ for handling serial printing
 * 
 */
void UART1IRQHandler(void);

/**
 * @brief Transmit data of given size to the serial
 * 
 * @param data 
 * @param data_size 
 * @return uint32_t 
 */
uint32_t UART1Send(uint8_t *data, int data_size);

/**
 * @brief Initalize UART2 
 * 
 */
void UART2FIFOInit(void);

/**
 * @brief Insert char array into UART2 FiFo buffer
 * 
 * @param data 
 */
void UART2PutChar(uint8_t data);

/**
 * @brief Probe UART2
 * 
 * @return int 
 */
int  UART2Probe(void);

/**
 * @brief Get a character array from UART2 buffer
 * 
 * @return uint8_t 
 */
uint8_t UART2GetChar(void);

/**
 * @brief Insert a char array into to UART2 buffer
 * 
 * @param data 
 */
void UART2PutChar(uint8_t data);

/**
 * @brief UART2 IRQ Handler
 * 
 */
void UART2IRQHandler(void);

/**
 * @brief High-level method for sending data of a 
 * limited size to the UART2 buffer
 * 
 * @param data 
 * @param data_size 
 * @return uint32_t 
 */
uint32_t UART2Send(uint8_t *data, int data_size);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
