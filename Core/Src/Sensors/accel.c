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
	// [이동 평균용 변수 선언]
	// 1. 과거 데이터를 저장할 버퍼
	static double buf_Ax[ACCEL_AVG_WINDOW_SIZE] = {0.0};
	static double buf_Ay[ACCEL_AVG_WINDOW_SIZE] = {0.0};
	static double buf_Az[ACCEL_AVG_WINDOW_SIZE] = {0.0};

	// 2. 현재 버퍼 인덱스 (0 ~ 9 반복)
	static int buf_idx = 0;

	// 3. 현재까지 쌓인 데이터 개수 (초기 1초 동안 평균값 오류 방지용)
	static int fill_count = 0;

	// 4. 빠른 계산을 위한 누적 합 (Sum)
	static double sum_Ax = 0.0;
	static double sum_Ay = 0.0;
	static double sum_Az = 0.0;

    // [수정] 6바이트만 읽으면 됩니다 (X_H, X_L, Y_H, Y_L, Z_H, Z_L)
    uint8_t buffer[6];

    for(;;)
    {
        // 0x3B번지부터 6바이트 읽기 (Gyro, Temp 건너뜀)
        if (HAL_I2C_Mem_Read(phmpu_i2c, MPU6050_ADDR, REG_ACCEL_XOUT, 1, buffer, 6, 100) == HAL_OK)
        {
        	// 1. Raw 데이터 결합
			int16_t temp_raw_x = (int16_t)(buffer[0] << 8 | buffer[1]);
			int16_t temp_raw_y = (int16_t)(buffer[2] << 8 | buffer[3]);
			int16_t temp_raw_z = (int16_t)(buffer[4] << 8 | buffer[5]);

			// 2. 물리량 변환 (g 단위)
			double temp_ax = temp_raw_x / 16384.0;
			double temp_ay = temp_raw_y / 16384.0;
			double temp_az = temp_raw_z / 16384.0;

			/* =========================================================
			 * [핵심] 이동 평균 (Moving Average) 계산 로직
			 * 방식: (현재 합) - (가장 오래된 값) + (새로운 값)
			 * ========================================================= */

			// 1) 합계에서 버퍼의 가장 오래된 값 빼기
			sum_Ax -= buf_Ax[buf_idx];
			sum_Ay -= buf_Ay[buf_idx];
			sum_Az -= buf_Az[buf_idx];

			// 2) 버퍼에 새로운 값 덮어쓰기
			buf_Ax[buf_idx] = temp_ax;
			buf_Ay[buf_idx] = temp_ay;
			buf_Az[buf_idx] = temp_az;

			// 3) 합계에 새로운 값 더하기
			sum_Ax += temp_ax;
			sum_Ay += temp_ay;
			sum_Az += temp_az;

			// 4) 인덱스 이동 (원형 큐)
			buf_idx++;
			if (buf_idx >= ACCEL_AVG_WINDOW_SIZE) {
				buf_idx = 0;
			}

			// 5) 데이터 개수 카운트 (초반 N개 채워질 때까지 나누는 수 조절)
			if (fill_count < ACCEL_AVG_WINDOW_SIZE) {
				fill_count++;
			}

			// 6) 평균 계산
			double calc_avg_x = sum_Ax / fill_count;
			double calc_avg_y = sum_Ay / fill_count;
			double calc_avg_z = sum_Az / fill_count;

			/* ========================================================= */

			// 3. 전역 구조체 업데이트 (Critical Section)
			taskENTER_CRITICAL();

			g_accel_data.Raw_X = temp_raw_x;
			g_accel_data.Raw_Y = temp_raw_y;
			g_accel_data.Raw_Z = temp_raw_z;

			g_accel_data.Ax = temp_ax;
			g_accel_data.Ay = temp_ay;
			g_accel_data.Az = temp_az;

			// [추가] 계산된 평균값 저장
			g_accel_data.Avg_Ax = calc_avg_x;
			g_accel_data.Avg_Ay = calc_avg_y;
			g_accel_data.Avg_Az = calc_avg_z;

			taskEXIT_CRITICAL();

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

void ACCEL_get_data(AccelData_report_t *pOutData)
{
	// 1. 계산에 사용할 임시 변수 (복사본 저장용)
	double temp_Ax;
	double temp_Avg_Ax;

	/* ============================================================
	 * [Critical Section 진입]
	 * 여기서는 최대한 빨리 "값 복사"만 하고 빠져나와야 합니다.
	 * 인터럽트가 잠겨있는 시간을 최소화하기 위함입니다.
	 * ============================================================ */
	taskENTER_CRITICAL();

	temp_Ax     = g_accel_data.Ax;
	temp_Avg_Ax = g_accel_data.Avg_Ax;

	taskEXIT_CRITICAL();
	/* ============================================================
	 * [Critical Section 탈출]
	 * 이제 인터럽트가 다시 동작하므로, 시간이 걸리는 연산을 해도 안전합니다.
	 * ============================================================ */

	// 2. 형식 변환 및 연산 수행 (g -> m/s^2 -> x1000)
	// 소수점 연산은 여기서 수행됩니다.
	int16_t conv_inst = (int16_t)(temp_Ax * GRAVITY_MSS_SCALE);
	int16_t conv_avg  = (int16_t)(temp_Avg_Ax * GRAVITY_MSS_SCALE);

	// 3. 결과 구조체에 대입 (int16_t -> uint16_t 비트 유지 캐스팅)
	pOutData->instant_Ax = (uint16_t)conv_inst;
	pOutData->avg_Ax     = (uint16_t)conv_avg;
}
