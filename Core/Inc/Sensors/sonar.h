/*
 * sonar.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_SONAR_H_
#define INC_SENSORS_SONAR_H_

#include "main.h"
#include <stm32f4xx_hal.h>

// 만약 초음파 센서 2개 사용한다면 아래 주석을 해제해주기
//#define DOUBLE_SONAR

#define TRIG_PORT_0 SONAR0_TRIGGER_GPIO_Port
#define TRIG_PIN_0  SONAR0_TRIGGER_Pin

#define TRIG_PORT_1 SONAR1_TRIGGER_GPIO_Port
#define TRIG_PIN_1  SONAR1_TRIGGER_Pin

typedef enum {
    SONAR_SENSOR_0 = 0,
    SONAR_SENSOR_1 = 1
} Sonar_ID;

typedef struct {
	uint16_t dist_0_cm; // 0번 센서 거리
	uint16_t dist_1_cm; // 1번 센서 거리
} SONAR_report_t;

//extern volatile uint8_t Distance[2];

void SONAR_init(TIM_HandleTypeDef *htim);

void SONAR_Process_Interrupt(TIM_HandleTypeDef *htim);

void HCSR04_Read (Sonar_ID);

void SONAR_Task_Loop(void const * argument);

void SONAR_get_data(SONAR_report_t *pOutData);

#endif /* INC_SENSORS_SONAR_H_ */
