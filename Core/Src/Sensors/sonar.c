/*
 * sonar.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

// HC-SR04 초음파 센서를 이용해 거리를 측정하기 위한 코드

#include <sonar.h>
#include <utils.h>

extern TIM_HandleTypeDef htim3;

volatile uint8_t Distance[2] = {0, 0};

static uint32_t SN1_IC_val_1 = 0;
static uint32_t SN1_IC_Val_2 = 0;
static uint32_t SN1_difference = 0;
static uint8_t SN1_is_first_captured = 0;  // is the first value captured ?

#ifdef DOUBLE_SONAR
static uint32_t SN2_IC_val_1 = 0;
static uint32_t SN2_IC_Val_2 = 0;
static uint32_t SN2_difference = 0;
static uint8_t SN2_is_first_captured = 0;  // is the first value captured ?
#endif

void HCSR04_Read (Sonar_ID sensor_id)
{
	// 초음파 센서 #0
	if(sensor_id == SONAR_SENSOR_0) {
		HAL_GPIO_WritePin(TRIG_PORT_0, TRIG_PIN_0, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		delay(10);  // wait for 10 us
		HAL_GPIO_WritePin(TRIG_PORT_0, TRIG_PIN_0, GPIO_PIN_RESET);  // pull the TRIG pin low

		__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_CC1);
	}
	// 초음파 센서 #1
	else if (sensor_id == SONAR_SENSOR_1) {
		HAL_GPIO_WritePin(TRIG_PORT_1, TRIG_PIN_1, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		delay(10);  // wait for 10 us
		HAL_GPIO_WritePin(TRIG_PORT_1, TRIG_PIN_1, GPIO_PIN_RESET);  // pull the TRIG pin low

		__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_CC2);
	}
}

void SONAR_Process_Interrupt(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // if the interrupt source is channel1
	{
		if (SN1_is_first_captured==0) // if the first value is not captured
		{
			SN1_IC_val_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // read the first value
			SN1_is_first_captured = 1;  // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (SN1_is_first_captured==1)   // if the first is already captured
		{
			SN1_IC_Val_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // read second value
			__HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter

			if (SN1_IC_Val_2 > SN1_IC_val_1)
			{
				SN1_difference = SN1_IC_Val_2-SN1_IC_val_1;
			}

			else if (SN1_IC_val_1 > SN1_IC_Val_2)
			{
				SN1_difference = (0xffff - SN1_IC_val_1) + SN1_IC_Val_2;
			}

			Distance[0] = SN1_difference * .034/2;
			SN1_is_first_captured = 0; // set it back to false

			// set polarity to rising edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC1);
		}
	}
#ifdef DOUBLE_SONAR
	else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)  // if the interrupt source is channel1
	{
		if (SN2_is_first_captured==0) // if the first value is not captured
		{
			SN2_IC_val_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2); // read the first value
			SN2_is_first_captured = 1;  // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (SN2_is_first_captured==1)   // if the first is already captured
		{
			SN2_IC_Val_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);  // read second value
			__HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter

			if (SN2_IC_Val_2 > SN2_IC_val_1)
			{
				SN2_difference = SN2_IC_Val_2-SN2_IC_val_1;
			}

			else if (SN2_IC_val_1 > SN2_IC_Val_2)
			{
				SN2_difference = (0xffff - SN2_IC_val_1) + SN2_IC_Val_2;
			}

			Distance[1] = SN2_difference * .034/2;
			SN2_is_first_captured = 0; // set it back to false

			// set polarity to rising edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_CC2);
		}
	}
#endif
}

