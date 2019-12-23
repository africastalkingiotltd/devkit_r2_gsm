/*
 * gsm_systick.c
 *
 *  Created on: Dec 13, 2019
 *      Author: kennedyotieno
 */

#include "main.h"
#include "gsm_systick.h"

volatile uint32_t milliSecondsTick;
volatile uint32_t gsmTick;

void delay(uint32_t delay_milliseconds)
{
    milliSecondsTick = 0;
    while(milliSecondsTick < delay_milliseconds)
    {
;
    }
}

void systickHandler()
{
    gsmTick ++;
}