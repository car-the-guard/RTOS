/* compass.c */

#include "compass.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// QMC5883P 설정
#define QMC5883P_ADDR_WRITE   0x58
#define QMC5883P_ADDR_READ    0x59
#define REG_DATA_START        0x01
#define REG_CONFIG_1          0x09
#define REG_RESET_PERIOD      0x0B

// [설정] 평균 필터 계수 (Alpha)
// 30초 평균 효과를 내기 위한 계수 계산:
// Alpha = 1 / (샘플링횟수) = 1 / (30초 * 10Hz) = 1/300 ≈ 0.0033
// 값이 작을수록 변화가 느리고(더 긴 시간 평균), 클수록 빠릅니다.
#define EMA_TARGET_AVG_SECOND 5.0f
#define EMA_ALPHA             1 / (EMA_TARGET_AVG_SECOND * 10)

static I2C_HandleTypeDef *ph_compass_i2c = NULL;
static COMPASS_data_t g_compass_data = {0, 0, 0, 0, 0};

// [수정] 배열 대신 누적 평균을 담을 변수 2개만 있으면 됩니다.
static float avg_vector_x = 0.0f; // 평균 Cos 성분
static float avg_vector_y = 0.0f; // 평균 Sin 성분
static uint8_t is_first_sample = 1; // 첫 샘플 초기화용 플래그

void COMPASS_init(I2C_HandleTypeDef *hi2c)
{
    ph_compass_i2c = hi2c;
    uint8_t data[2];

    HAL_Delay(100);

    /* 1. Soft Reset */
    data[0] = REG_RESET_PERIOD;
    data[1] = 0x01;
    HAL_I2C_Master_Transmit(ph_compass_i2c, QMC5883P_ADDR_WRITE, data, 2, 100);
    HAL_Delay(10);

    /* 2. Control Register 1 */
    data[0] = REG_CONFIG_1;
    data[1] = 0x1D;

    if (HAL_I2C_Master_Transmit(ph_compass_i2c, QMC5883P_ADDR_WRITE, data, 2, 100) == HAL_OK)
    {
        printf(">> [INIT] QMC5883P Configured (EMA Mode).\r\n");
    }
    HAL_Delay(50);
}

void COMPASS_task_loop(void const * argument)
{
    uint8_t buffer[6];
    int16_t x_min = 32000, x_max = -32000;
    int16_t y_min = 32000, y_max = -32000;
    int32_t offset_x = 0, offset_y = 0;

    if (ph_compass_i2c == NULL) return;

    for(;;)
    {
    	// 지자기 센서는 우선 mock 데이터를 사용
//        if (HAL_I2C_Mem_Read(ph_compass_i2c, QMC5883P_ADDR_READ, REG_DATA_START, I2C_MEMADD_SIZE_8BIT, buffer, 6, 100) == HAL_OK)
//        {
//            int16_t raw_x = (int16_t)((buffer[1] << 8) | buffer[0]);
//            int16_t raw_y = (int16_t)((buffer[3] << 8) | buffer[2]);
//            int16_t raw_z = (int16_t)((buffer[5] << 8) | buffer[4]);
//
//            if (raw_x == 0 && raw_y == 0) { osDelay(10); continue; }
//
//            // 1. Min/Max 갱신 및 오프셋 계산 (동일)
//            if (raw_x < x_min) x_min = raw_x; if (raw_x > x_max) x_max = raw_x;
//            if (raw_y < y_min) y_min = raw_y; if (raw_y > y_max) y_max = raw_y;
//
//            offset_x = (int32_t)(x_max + x_min) / 2;
//            offset_y = (int32_t)(y_max + y_min) / 2;
//
//            int32_t cal_x = (int32_t)raw_x - offset_x;
//            int32_t cal_y = (int32_t)raw_y - offset_y;
//
//            // --------------------------------------------------------
//            // [계산 Zone] Critical Section 밖에서 무거운 연산 수행
//            // --------------------------------------------------------
//
//            // (1) 현재 각도 벡터 계산
//            double current_rad = atan2((double)cal_y, (double)cal_x);
//            float current_cos = (float)cos(current_rad); // X 성분 (Unit Vector)
//            float current_sin = (float)sin(current_rad); // Y 성분 (Unit Vector)
//
//            // (2) EMA (지수 이동 평균) 계산 - 여기가 핵심!
//            // 공식: Avg = (Old_Avg * (1 - Alpha)) + (New_Val * Alpha)
//            if (is_first_sample)
//            {
//                // 첫 데이터는 바로 평균값으로 설정 (초기 지연 방지)
//                avg_vector_x = current_cos;
//                avg_vector_y = current_sin;
//                is_first_sample = 0;
//            }
//            else
//            {
//                // 벡터 X, Y 각각에 대해 필터링 적용
//                avg_vector_x = (avg_vector_x * (1.0f - EMA_ALPHA)) + (current_cos * EMA_ALPHA);
//                avg_vector_y = (avg_vector_y * (1.0f - EMA_ALPHA)) + (current_sin * EMA_ALPHA);
//            }
//
//            // (3) 평균 벡터 -> 각도 변환
//            double avg_rad = atan2((double)avg_vector_y, (double)avg_vector_x);
//
//            // (4) Radian -> Degree -> Int 변환 준비
//            if (current_rad < 0) current_rad += 2 * M_PI;
//            if (avg_rad < 0) avg_rad += 2 * M_PI;
//
//            int32_t final_curr = (int32_t)(current_rad * (180.0 / M_PI) * 100.0);
//            int32_t final_avg  = (int32_t)(avg_rad * (180.0 / M_PI) * 100.0);
//
//            // --------------------------------------------------------
//            // [대입 Zone] Critical Section: 값 복사만 수행
//            // --------------------------------------------------------
//            taskENTER_CRITICAL();
//
//            g_compass_data.raw_x = raw_x;
//            g_compass_data.raw_y = raw_y;
//            g_compass_data.raw_z = raw_z;
//            g_compass_data.heading_int = final_curr;    // 순간값
//            g_compass_data.heading_avg_30s = final_avg; // 30초 필터값
//
//            taskEXIT_CRITICAL();
//
//        }

    	g_compass_data.heading_int = (15 + g_compass_data.heading_int) % 360;    // 순간값
		g_compass_data.heading_avg_30s = (15 + g_compass_data.heading_avg_30s) % 360; // 30초 필터값


        osDelay(30000);
    }
}

void COMPASS_get_data(COMPASS_data_t *pOutData)
{
    taskENTER_CRITICAL();
    *pOutData = g_compass_data;
    taskEXIT_CRITICAL();
}
