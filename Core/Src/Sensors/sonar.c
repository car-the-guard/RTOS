/*
 * sonar.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

// HC-SR04 초음파 센서를 이용해 거리를 측정하기 위한 코드

#include <stdio.h>
#include "sonar.h"
#include "utils.h"
#include "cmsis_os.h"

static TIM_HandleTypeDef* SN_htim;

volatile uint8_t Distance[2] = {0, 0};

static uint32_t SN1_IC_val_1 = 0;
static uint32_t SN1_IC_Val_2 = 0;
static uint32_t SN1_difference = 0;
static uint8_t SN1_is_first_captured = 0;  // is the first value captured ?

uint32_t sensing_delay = 1000;

#ifdef DOUBLE_SONAR
static uint32_t SN2_IC_val_1 = 0;
static uint32_t SN2_IC_Val_2 = 0;
static uint32_t SN2_difference = 0;
static uint8_t SN2_is_first_captured = 0;  // is the first value captured ?
#endif

/**
 * @brief  소나 센서 전용 Task 루프
 * @note   이 함수는 절대 리턴하지 않습니다 (Infinite Loop)
 */
void SONAR_Task_Loop(void const * argument)
{
	for(;;)
	{
		HCSR04_Read(SONAR_SENSOR_0);

		// 측정 대기 및 간섭 방지 (50ms)
		// HAL_Delay(50) 대신 osDelay(50)을 써야 Context Switching이 일어남

#ifdef DOUBLE_SONAR
		osDelay(50);
		HCSR04_Read(SONAR_SENSOR_1);
		osDelay(50);
#endif
		// (선택) 디버깅 출력
		// printf는 Thread-safe하지 않을 수 있으니 주의하거나 Mutex 사용 필요
		printf("SONAR #0: %lu cm, SONAR #1: %lu cm \r\n", Distance[SONAR_SENSOR_0], Distance[SONAR_SENSOR_1]);

		osDelay(sensing_delay);
	}
}

void SONAR_init(TIM_HandleTypeDef *htim)
{
	SN_htim = htim;
	  HAL_TIM_IC_Start(htim, TIM_CHANNEL_1);
#ifdef DOUBLE_SONAR
	  HAL_TIM_IC_Start(htim, TIM_CHANNEL_2);
#endif
}

void HCSR04_Read (Sonar_ID sensor_id)
{
	// 초음파 센서 #0
	if(sensor_id == SONAR_SENSOR_0) {
		HAL_GPIO_WritePin(TRIG_PORT_0, TRIG_PIN_0, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		delay(10);  // wait for 10 us
		HAL_GPIO_WritePin(TRIG_PORT_0, TRIG_PIN_0, GPIO_PIN_RESET);  // pull the TRIG pin low

		__HAL_TIM_ENABLE_IT(SN_htim, TIM_IT_CC1);
	}
	// 초음파 센서 #1
	else if (sensor_id == SONAR_SENSOR_1) {
		HAL_GPIO_WritePin(TRIG_PORT_1, TRIG_PIN_1, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		delay(10);  // wait for 10 us
		HAL_GPIO_WritePin(TRIG_PORT_1, TRIG_PIN_1, GPIO_PIN_RESET);  // pull the TRIG pin low

		__HAL_TIM_ENABLE_IT(SN_htim, TIM_IT_CC2);
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
			__HAL_TIM_DISABLE_IT(SN_htim, TIM_IT_CC1);
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
			__HAL_TIM_DISABLE_IT(SN_htim, TIM_IT_CC2);
		}
	}
#endif
}

void SONAR_get_data(SONAR_report_t *pOutData)
{
	taskENTER_CRITICAL();

	pOutData->dist_0_cm = Distance[0];
	pOutData->dist_1_cm = Distance[1];

	taskEXIT_CRITICAL();
}
