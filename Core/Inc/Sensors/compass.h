/*
 * compass.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_COMPASS_H_
#define INC_SENSORS_COMPASS_H_

#include "main.h"

// 데이터 구조체 (모두 정수형)
typedef struct {
    int16_t raw_x;       // X축 Raw 데이터
    int16_t raw_y;       // Y축 Raw 데이터
    int16_t raw_z;       // Z축 Raw 데이터

    int32_t heading_int; // 각도 x 100 (예: 12345 = 123.45도)
    int32_t heading_avg_30s;
} COMPASS_data_t;


void COMPASS_init(I2C_HandleTypeDef *hi2c);
void COMPASS_task_loop(void const * argument);

// 외부에서 안전하게 데이터를 복사해가는 함수
void COMPASS_get_data(COMPASS_data_t *pOutData);


// 이거 어떻게 쓸거냐면
/**
void StartOtherTask(void const * argument)
{
    CompassGlobal_t currentData; // 데이터를 받아올 지역 변수

    for(;;)
    {
        // 1. 안전하게 데이터 가져오기 (Critical Section 내부 처리됨)
        COMPASS_GetData(&currentData);

        // 2. 데이터 사용
        // 예: Heading 값이 180.50도 이상이면...
        if (currentData.heading_int > 18050)
        {
             // ... 동작 수행 ...
        }

        // Raw 데이터도 사용 가능
        // printf("X: %d\r\n", currentData.raw_x);

        osDelay(50);
    }
}
 */

#endif /* INC_SENSORS_COMPASS_H_ */
