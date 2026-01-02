/* compass.c */
#include "compass.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static I2C_HandleTypeDef *ph_compass_i2c = NULL;
CompassData_t compassData;

// QMC5883P (Addr 0x2C -> Shifted 0x58)
#define QMC5883P_ADDR_WRITE   0x58
#define QMC5883P_ADDR_READ    0x59
#define REG_DATA_START        0x01
#define REG_CONFIG_1          0x09
#define REG_RESET_PERIOD      0x0B

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

    /* 2. Control Register 1 Setting */
    data[0] = REG_CONFIG_1;
    data[1] = 0x1D;

    if (HAL_I2C_Master_Transmit(ph_compass_i2c, QMC5883P_ADDR_WRITE, data, 2, 100) == HAL_OK)
    {
        printf(">> [INIT] QMC5883P Configured.\r\n");
    }
    HAL_Delay(50);
}

void COMPASS_task_loop(void const * argument)
{
    uint8_t buffer[6];

    // 캘리브레이션용 변수
    int16_t x_min = 32000, x_max = -32000;
    int16_t y_min = 32000, y_max = -32000;

    float offset_x = 0.0f;
    float offset_y = 0.0f;

    if (ph_compass_i2c == NULL) return;

    for(;;)
    {
        if (HAL_I2C_Mem_Read(ph_compass_i2c, QMC5883P_ADDR_READ, REG_DATA_START, I2C_MEMADD_SIZE_8BIT, buffer, 6, 100) == HAL_OK)
        {
            // Little Endian
            int16_t raw_x = (int16_t)((buffer[1] << 8) | buffer[0]);
            int16_t raw_y = (int16_t)((buffer[3] << 8) | buffer[2]);
            int16_t raw_z = (int16_t)((buffer[5] << 8) | buffer[4]);

            if (raw_x == 0 && raw_y == 0 && raw_z == 0) {
                osDelay(10);
                continue;
            }

            // [1] 실시간 캘리브레이션 (Min/Max 갱신)
            if (raw_x < x_min) x_min = raw_x;
            if (raw_x > x_max) x_max = raw_x;
            if (raw_y < y_min) y_min = raw_y;
            if (raw_y > y_max) y_max = raw_y;

            offset_x = (float)(x_max + x_min) / 2.0f;
            offset_y = (float)(y_max + y_min) / 2.0f;

            // [2] 보정된 값 계산
            float calibrated_x = (float)raw_x - offset_x;
            float calibrated_y = (float)raw_y - offset_y;

            // [3] 방위각(Heading) 계산
            double heading = atan2((double)calibrated_y, (double)calibrated_x);

            if (heading < 0) heading += 2 * M_PI;
            if (heading > 2 * M_PI) heading -= 2 * M_PI;

            // float 각도 (0.0 ~ 360.0)
            float headingFloat = heading * 180.0 / M_PI;

            // [핵심 변경] float -> int 변환 (x100)
            // 예: 123.45도 -> 12345
            int32_t headingInt = (int32_t)(headingFloat * 100.0f);

            // 구조체 저장 (구조체 멤버 타입에 따라 다름, 여기선 int 값을 우선 출력)
            compassData.x = raw_x;
            compassData.y = raw_y;
            compassData.z = raw_z;

            // 만약 compassData.headingDegrees가 float라면 그대로 넣고,
            // 나중에 통신할 때는 headingInt를 쓰시면 됩니다.
            compassData.headingDegrees = headingFloat;

            // 출력 확인 (Integer 값 출력)
            printf("Heading(x100): %ld | Deg: %ld.%02ld | Offset(X:%ld Y:%ld)\r\n",
                   headingInt,
                   headingInt / 100, headingInt % 100,   // 예: 12345 -> 123.45 로 출력
                   (int32_t)offset_x, (int32_t)offset_y); // float 변수여도 강제로 int로 변환 출력
        }

        osDelay(100);
    }
}
