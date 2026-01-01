/*
 * accel.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_ACCEL_H_
#define INC_SENSORS_ACCEL_H_

#include "main.h"

// MPU6050 I2C Address (AD0 = Low 기준)
// 0x68 << 1 = 0xD0
#define MPU6050_ADDR 0xD0

// 데이터 저장용 구조체
typedef struct {
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    double  Ax, Ay, Az; // 단위: g

    int16_t Temp_Raw;
    float   Temperature; // 단위: C

    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
    double  Gx, Gy, Gz; // 단위: deg/s
} MPU6050_Data_t;

// 함수 원형
void ACCEL_init(I2C_HandleTypeDef* hi2c);
void ACCEL_task_loop(void const * argument);

#endif /* INC_SENSORS_ACCEL_H_ */
