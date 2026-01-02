/*
 * grid_led.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_GRID_LED_GRID_LED_H_
#define INC_GRID_LED_GRID_LED_H_

#include "main.h"
#include "max7219.h"
#include "max7219_matrix.h"

typedef enum {
    GRID_LED_OFF      = 0, // 모두 끄기
    GRID_LED_ON       = 1, // 모두 켜기
    GRID_LED_BLINK    = 2, // 빠르게 점멸
    GRID_LED_ANIMATE  = 3  // 좌우 화살표 애니메이션
} GridLed_State_t;

// 전역 변수 (외부에서 변경 가능)
extern volatile GridLed_State_t current_led_state;

void GRIDLED_SetState(GridLed_State_t state);
void GRIDLED_Process(void); // Task 루프 안에서 호출

void GRIDLED_init(SPI_HandleTypeDef* hspi);

void GRIDLED_Task_Loop(void const * argument);


#endif /* INC_GRID_LED_GRID_LED_H_ */
