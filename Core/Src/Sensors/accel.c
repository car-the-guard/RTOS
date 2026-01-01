/*
 * accel.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#include "accel.h"
#include <stdio.h>

static I2C_HandleTypeDef *phmpu_i2c;
MPU6050_Data_t mpuData;

// 레지스터 정의
#define REG_WHO_AM_I    0x75
#define REG_PWR_MGMT_1  0x6B
#define REG_ACCEL_XOUT  0x3B // 여기서부터 14바이트를 연속으로 읽음

/* accel.c 수정 */

void ACCEL_init(I2C_HandleTypeDef* hi2c)
{
    phmpu_i2c = hi2c;
    uint8_t check_val = 0;
    uint8_t data = 0;

    HAL_Delay(100);
    printf("\r\n=== MPU6050 Init Start ===\r\n");

    // 1. 연결 확인 (WHO_AM_I 읽기)
    if(HAL_I2C_Mem_Read(phmpu_i2c, MPU6050_ADDR, REG_WHO_AM_I, 1, &check_val, 1, 100) == HAL_OK)
    {
        // 0x68(정품) 혹은 0x72(호환칩) 모두 허용
        if (check_val == 0x68 || check_val == 0x72)
        {
            printf("Device Found! WHO_AM_I: 0x%02X\r\n", check_val);

            // 2. 칩 깨우기 (매우 중요: 이 코드가 실행되어야 센서가 동작함)
            data = 0x00;
            HAL_I2C_Mem_Write(phmpu_i2c, MPU6050_ADDR, REG_PWR_MGMT_1, 1, &data, 1, 100);
            printf("MPU6050 Waked Up (Sleep Mode Disabled).\r\n");
        }
        else
        {
            // 경고만 띄우고 강제로 초기화 시도 (선택 사항)
            printf("WARNING: Unknown Device ID: 0x%02X (Expected: 0x68 or 0x72)\r\n", check_val);
            printf("Trying to wake up anyway...\r\n");

            data = 0x00;
            HAL_I2C_Mem_Write(phmpu_i2c, MPU6050_ADDR, REG_PWR_MGMT_1, 1, &data, 1, 100);

            HAL_Delay(10);
        }
    }
    else
    {
        printf("Error! I2C Communication Failed (Check wiring or Pull-up resistors)\r\n");
    }
}

void ACCEL_task_loop(void const * argument)
{
    uint8_t buffer[14]; // Accel(6) + Temp(2) + Gyro(6)

    for(;;)
    {
        // 14바이트를 한 번에 읽어옵니다. (효율적)
        if (HAL_I2C_Mem_Read(phmpu_i2c, MPU6050_ADDR, REG_ACCEL_XOUT, 1, buffer, 14, 100) == HAL_OK)
        {
            // 데이터 결합 (MPU6050은 High Byte가 먼저 옵니다! << 8 | Low)
            // [중요] 아까 나침반(QMC)과는 반대입니다.
            mpuData.Accel_X = (int16_t)(buffer[0] << 8 | buffer[1]);
            mpuData.Accel_Y = (int16_t)(buffer[2] << 8 | buffer[3]);
            mpuData.Accel_Z = (int16_t)(buffer[4] << 8 | buffer[5]);

            mpuData.Temp_Raw = (int16_t)(buffer[6] << 8 | buffer[7]);

            mpuData.Gyro_X  = (int16_t)(buffer[8] << 8 | buffer[9]);
            mpuData.Gyro_Y  = (int16_t)(buffer[10] << 8 | buffer[11]);
            mpuData.Gyro_Z  = (int16_t)(buffer[12] << 8 | buffer[13]);

            // 물리량 변환 (기본 설정 기준)
            // Accel: LSB/g = 16384 (±2g range)
            mpuData.Ax = mpuData.Accel_X / 16384.0;
            mpuData.Ay = mpuData.Accel_Y / 16384.0;
            mpuData.Az = mpuData.Accel_Z / 16384.0;

            // Temperature: (Raw / 340.0) + 36.53
            mpuData.Temperature = (mpuData.Temp_Raw / 340.0f) + 36.53f;

            // Gyro: LSB/(deg/s) = 131.0 (±250dps range)
            mpuData.Gx = mpuData.Gyro_X / 131.0;
            mpuData.Gy = mpuData.Gyro_Y / 131.0;
            mpuData.Gz = mpuData.Gyro_Z / 131.0;

            // 출력
			// [수정된 출력부]
			// 모든 값을 100배 곱해서 정수로 출력합니다.
			// 예: 1.00g -> 100, -0.56g -> -56, 24.5도 -> 2450
			printf("Ax:%d Ay:%d Az:%d | Gx:%d Gy:%d Gz:%d | T:%d\r\n",
				   (int)(mpuData.Ax * 100),
				   (int)(mpuData.Ay * 100),
				   (int)(mpuData.Az * 100),
				   (int)(mpuData.Gx * 100),
				   (int)(mpuData.Gy * 100),
				   (int)(mpuData.Gz * 100),
				   (int)(mpuData.Temperature * 100));
        }
        else
        {
            printf("[MPU6050] I2C Read Error\r\n");
            // 에러 시 재초기화 시도 (선택)
            // MPU6050_Init(phmpu_i2c);
        }

        osDelay(1000); // 10Hz
    }
}
