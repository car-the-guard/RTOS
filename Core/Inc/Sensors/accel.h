/*
 * accel.h
 * Created on: Jan 1, 2026
 * Author: mokta
 */

#ifndef INC_SENSORS_ACCEL_H_
#define INC_SENSORS_ACCEL_H_

#include "main.h"

// MPU6050 I2C Address (AD0 = Low)
#define MPU6050_ADDR 0xD0

// [수정] 가속도 데이터만 담는 구조체 (Gyro, Temp 제거)
typedef struct {
    // Raw Data (int16_t)
    int16_t Raw_X;
    int16_t Raw_Y;
    int16_t Raw_Z;

    // Converted Data (g 단위)
    double Ax;
    double Ay;
    double Az;
} AccelData_t;

// 함수 원형
void ACCEL_init(I2C_HandleTypeDef* hi2c);
void ACCEL_task_loop(void const * argument);

// [추가] 외부 Task에서 안전하게 데이터를 가져가는 함수
void ACCEL_get_data(AccelData_t *pOutData);

#endif /* INC_SENSORS_ACCEL_H_ */
