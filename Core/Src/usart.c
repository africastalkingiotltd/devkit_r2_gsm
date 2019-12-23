/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN 0 */
#define _BV(bit) (1 << (bit))


#define UART1_TXB	128
#define UART1_RXB	512

static volatile struct UART1Fifo {
	uint16_t tri, twi, tct;
	uint16_t rri, rwi, rct;
	uint8_t tbuf[UART1_TXB];
	uint8_t rbuf[UART1_RXB];
}UART1Fifo;


#define UART2_TXB	1024
#define UART2_RXB	512

static volatile struct UART2Fifo {
	uint16_t tri, twi, tct;
	uint16_t rri, rwi, rct;
	uint8_t tbuf[UART2_TXB];
	uint8_t rbuf[UART2_RXB];
}UART2Fifo;

volatile static uint8_t controllerSegmentIndex;
/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration  
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration  
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX 
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART2_IRQn);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);

}

/* USER CODE BEGIN 1 */

void UART1FIFOInit(void)
{

	UART1Fifo.tri = 0;
	UART1Fifo.twi = 0;
	UART1Fifo.tct = 0;

	UART1Fifo.rri = 0;
	UART1Fifo.rwi = 0;
	UART1Fifo.rct = 0;

	USART1->CR1  |= _BV(5);
	LL_USART_Enable(USART1);
}

int UART1Probe(void)
{
	return UART1Fifo.rct;
}

uint8_t UART1GetChar(void)
{
	uint8_t d;
	int i;

	while(!UART1Fifo.rct) ;

	i = UART1Fifo.rri;
	d = UART1Fifo.rbuf[i];
	UART1Fifo.rri = ++i % UART1_RXB;
	__disable_irq();
	UART1Fifo.rct--;
	__enable_irq();

	return d;
}

void UART1PutChar(uint8_t d)
{

	int i;
	while(UART1Fifo.tct >= UART1_TXB) ;

	i = UART1Fifo.twi;
	UART1Fifo.tbuf[i] = d;
	UART1Fifo.twi     = ++i % UART1_TXB;
	__disable_irq();
	UART1Fifo.tct++;
	USART1->CR1 |= _BV(7);
	__enable_irq();

}

void debugUART1IRQHandler(void)
{
	uint32_t sr = USART1->SR;
	uint8_t  d;
	int i;

	if(sr & _BV(5))
	{
		d = USART1->DR;
		i = UART1Fifo.rct;
		if(i < UART1_RXB)
		{
			UART1Fifo.rct = ++i;
			i = UART1Fifo.rwi;
			UART1Fifo.rbuf[i] = d;
			UART1Fifo.rwi = ++i % UART1_RXB;
		}
	}
	if(sr & _BV(7))
	{
		i = UART1Fifo.tct;
		if(i--)
		{
			UART1Fifo.tct = (uint16_t)i;
			i = UART1Fifo.tri;
			USART1->DR =  UART1Fifo.tbuf[i];
			UART1Fifo.tri = ++i % UART1_TXB;
		} else {
			USART1->CR1 &= ~_BV(7);
		}
	}
}

uint32_t UART1Send(uint8_t *data, int data_size)
{
	int i;
	for(i=0; i < data_size; i++)
	{
		if(*data != '\0') {
		UART1_putc(*data++);
		}
	}
	return i;
}


// USART 2 GSM
uint8_t UART2GetChar(void)
{
	uint8_t d;
	int i;

	while(!UART2Fifo.rct)
	{
		;
	}
	i = UART2Fifo.rri;
	d = UART2Fifo.rbuf[i];
	UART2Fifo.rri = ++i % UART2_RXB;
	__disable_irq();
	UART2Fifo.rct--;
	__enable_irq();

	return d;
}

int UART2Probe(void)
{
	return UART2Fifo.rct;
}

void UART2FIFOInit(void)
{
	UART2Fifo.tri = 0;
	UART2Fifo.twi = 0;
	UART2Fifo.tct = 0;

	UART2Fifo.rri = 0;
	UART2Fifo.rwi = 0;
	UART2Fifo.rct = 0;

	USART2->CR1  |= _BV(5);
	LL_USART_Enable(USART2);
}

void UART2PutChar(uint8_t data)
{
	int i;

	if(UART2Fifo.tct >= UART2_TXB)
	{
		UART2Fifo.tri = 0;
		UART2Fifo.twi = 0;
		UART2Fifo.tct = 0;

		UART2Fifo.rri = 0;
		UART2Fifo.rwi = 0;
		UART2Fifo.rct = 0;
		return;
	}
	i = UART2Fifo.twi;
	UART2Fifo.tbuf[i] = d;
	UART2Fifo.twi = ++i % UART2_TXB;
	__disable_irq();
	UART2Fifo.tct++;
	USART2->CR1 |= _BV(7);
	__enable_irq();
}

uint32_t UART2Send(uint8_t *data, int data_size)
{
	int i;
	for(i=0; i<data_size; i++)
	{
		UART2_putc(*data++);
	}
	return i;
}

void UART2IRQHandler(void)
{
	uint32_t usart_sr = USART2->SR;
	uint32_t status_register;
	uint8_t d;
	int i;

    if(usart_sr &_BV(5))
    {
    	d = USART2->DR;
    	USART1->DR = d;
    	i = UART2Fifo.rct;
    	if(i < UART2_RXB)
    	{
    		UART2Fifo.rct = ++i;
    		i = UART2Fifo.rwi;
    		UART2Fifo.rbuf[i] = d;
    		UART2Fifo.rwi = ++i % UART2_RXB;
    	}
    }

    if(usart_sr  & _BV(7))
    {
    	i = UART2Fifo.tct;
    	if(i--)
    	{
    		UART2Fifo.tct = (uint16_t)i;
    		i = UART2Fifo.tri;
    		USART2->DR = UART2Fifo.tbuf[i];
    		USART1->DR = UART2Fifo.tbuf[i];
    		UART2Fifo.tri = ++i % UART2_TXB;
    	} else {
    		USART2->CR1 &= ~_BV(7);
    	}
    }

    status_register = USART2->SR;
	if(status_register & _BV(3))
	{
		d = USART2->DR;
	}
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
