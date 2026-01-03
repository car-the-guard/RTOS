/*
 * utils.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#include <utils.h>
#include <stm32f4xx_hal.h>

extern TIM_HandleTypeDef htim3;

void delay (uint16_t time)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	while (__HAL_TIM_GET_COUNTER (&htim3) < time);
}

/**
 * @brief  데이터 배열의 CRC-8 값을 계산합니다.
 * @param  data: 데이터 포인터 (uint8_t 배열)
 * @param  len: 계산할 바이트 길이 (여기서는 7)
 * @return 계산된 1byte CRC 값
 */
uint8_t calculate_CRC8(uint8_t *data, uint8_t len)
{
    uint8_t crc = CRC8_INIT;
    uint8_t i, j;

    for (i = 0; i < len; i++) {
        crc ^= data[i]; // 현재 데이터와 XOR

        for (j = 0; j < 8; j++) {
            if (crc & 0x80) { // 최상위 비트가 1이면
                crc = (crc << 1) ^ CRC8_POLY; // Shift 후 다항식과 XOR
            } else {
                crc <<= 1; // 아니면 그냥 Shift
            }
        }
    }
    return crc;
}
