/*
 * accel.c
 * Created on: Jan 1, 2026
 * Author: mokta
 */

#include "accel.h"
#include "FreeRTOS.h" // Critical Section
#include "task.h"     // Critical Section
#include <stdio.h>

// 레지스터 정의
#define REG_WHO_AM_I    0x75
#define REG_PWR_MGMT_1  0x6B
#define REG_ACCEL_XOUT  0x3B // 가속도 데이터 시작 주소

static I2C_HandleTypeDef *phmpu_i2c;

// [핵심] 내부 전역 변수 (static으로 숨김)
static AccelData_t g_accel_data = {0, 0, 0, 0.0, 0.0, 0.0};

void ACCEL_init(I2C_HandleTypeDef* hi2c)
{
    phmpu_i2c = hi2c;
    uint8_t check_val = 0;
    uint8_t data = 0;

    HAL_Delay(100);
    printf("\r\n=== MPU6050 Init Start (Accel Only) ===\r\n");

    if(HAL_I2C_Mem_Read(phmpu_i2c, MPU6050_ADDR, REG_WHO_AM_I, 1, &check_val, 1, 100) == HAL_OK)
    {
        if (check_val == 0x68 || check_val == 0x72)
        {
            printf("Device Found! WHO_AM_I: 0x%02X\r\n", check_val);

            // 칩 깨우기
            data = 0x00;
            HAL_I2C_Mem_Write(phmpu_i2c, MPU6050_ADDR, REG_PWR_MGMT_1, 1, &data, 1, 100);
            printf("MPU6050 Waked Up.\r\n");
        }
        else
        {
            printf("WARNING: Unknown Device ID: 0x%02X\r\n", check_val);
        }
    }
    else
    {
        printf("Error! I2C Communication Failed\r\n");
    }
}

void ACCEL_task_loop(void const * argument)
{
    // [수정] 6바이트만 읽으면 됩니다 (X_H, X_L, Y_H, Y_L, Z_H, Z_L)
    uint8_t buffer[6];

    for(;;)
    {
        // 0x3B번지부터 6바이트 읽기 (Gyro, Temp 건너뜀)
        if (HAL_I2C_Mem_Read(phmpu_i2c, MPU6050_ADDR, REG_ACCEL_XOUT, 1, buffer, 6, 100) == HAL_OK)
        {
            // 1. Raw 데이터 결합 (Big Endian)
            int16_t temp_raw_x = (int16_t)(buffer[0] << 8 | buffer[1]);
            int16_t temp_raw_y = (int16_t)(buffer[2] << 8 | buffer[3]);
            int16_t temp_raw_z = (int16_t)(buffer[4] << 8 | buffer[5]);

            // 2. 물리량 변환 (Local 변수에서 계산 수행)
            // LSB/g = 16384.0 (기본 설정 ±2g)
            double temp_ax = temp_raw_x / 16384.0;
            double temp_ay = temp_raw_y / 16384.0;
            double temp_az = temp_raw_z / 16384.0;

            // ---------------------------------------------------------
            // [핵심] Critical Section: 전역 변수 업데이트
            // ---------------------------------------------------------
            taskENTER_CRITICAL();

            g_accel_data.Raw_X = temp_raw_x;
            g_accel_data.Raw_Y = temp_raw_y;
            g_accel_data.Raw_Z = temp_raw_z;
            g_accel_data.Ax = temp_ax;
            g_accel_data.Ay = temp_ay;
            g_accel_data.Az = temp_az;

            taskEXIT_CRITICAL();
            // ---------------------------------------------------------

            // 디버그 출력 (옵션: 정수형 x100 변환하여 출력)
             printf("Accel: %d, %d, %d\r\n",
                    (int)(temp_ax * 100), (int)(temp_ay * 100), (int)(temp_az * 100));
        }
        else
        {
            // printf("[MPU6050] I2C Read Error\r\n");
        }

        osDelay(100); // 10Hz Sampling
    }
}

void ACCEL_get_data(AccelData_t *pOutData)
{
    // 읽는 도중에 값이 변하지 않도록 잠금
    taskENTER_CRITICAL();

    *pOutData = g_accel_data; // 구조체 복사

    taskEXIT_CRITICAL();
}
