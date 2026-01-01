/*
 * utils.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#include <utils.h>
#include <stm32f4xx_hal.h>

extern TIM_HandleTypeDef htim3;

void delay (uint16_t time)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	while (__HAL_TIM_GET_COUNTER (&htim3) < time);
}

