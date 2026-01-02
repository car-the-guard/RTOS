/*
 * compass.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_COMPASS_H_
#define INC_SENSORS_COMPASS_H_
#include "main.h"
#include <math.h>

// I2C 주소 (QMC5883P / HP5883 / HW-127)
#define QMC5883P_ADDR_WRITE  (0x2C << 1)
#define QMC5883P_ADDR_READ   ((0x2C << 1) | 1)

// [핵심] QMC5883P (0x2C) 전용 레지스터 맵
// 주의: 0x0D 주소의 QMC5883L과는 주소가 다릅니다.
#define REG_DATA_X_LSB     0x00
#define REG_STATUS         0x06  // Status Register
#define REG_CONTROL_1      0x0A  // Control Register 1 (Mode, ODR, RNG, OSR) - 0x09가 아님!
#define REG_CONTROL_2      0x0B  // Control Register 2 (Soft Reset, Rollover)
#define REG_PERIOD         0x0B  // SET/RESET Period (Control 2와 같은 주소 공유하는 경우 많음)
#define REG_CHIP_ID        0x0D  // Chip ID (보통 0x60 같은 값이 들어있음)
#define REG_SECRET_29      0x29  // [중요] 이 레지스터를 설정해야 동작함

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    float headingDegrees;
} CompassData_t;

extern CompassData_t compassData;

void COMPASS_init(I2C_HandleTypeDef *hi2c);
void COMPASS_task_loop(void const * argument);

// 출력용 함수 (정수형 변환)
void PrintCompassData(void);
#endif /* INC_SENSORS_COMPASS_H_ */
