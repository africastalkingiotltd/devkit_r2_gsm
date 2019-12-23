/*
 * gsm_systick.h
 *
 *  Created on: Dec 13, 2019
 *      Author: kennedyotieno
 */

#ifndef GSM_SYSTICK_H_
#define GSM_SYSTICK_H_

#include "main.h"

extern volatile uint32_t milliSecondsTick;
extern volatile uint32_t gsmTick;

void delay(uint32_t delay_milliseconds);
void systickHandler();

#endif /* GSM_SYSTICK_H_ */
