/*
 * GPIO_Handler.c
 *
 *  Created on: Jul 3, 2025
 *      Author: karl.yamashita
 */

#include "main.h"



// called from GPIO falling/rising callback
void GPIO_EXTI_Handler(uint16_t GPIO_Pin)
{
	GPIO_PinState buttonState;

	if(GPIO_Pin == Button_Pin)
	{
		buttonState = HAL_GPIO_ReadPin(Button_GPIO_Port, Button_Pin);
		if(!buttonState) // button pressed
		{
			TimerCallbackTimerStart(&timerCallback, ButtonDebounced, 10, TIMER_NO_REPEAT);
		}
		else // button released
		{
			TimerCallbackDisable(&timerCallback, ButtonDebounced);
		}
	}
}



