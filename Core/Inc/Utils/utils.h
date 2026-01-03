/*
 * utils.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_UTILS_UTILS_H_
#define INC_UTILS_UTILS_H_

#include <stdint.h>


#define CRC8_POLY 0x07 // 표준 CRC-8 다항식 (x^8 + x^2 + x + 1)
#define CRC8_INIT 0x00 // 초기값 (보통 0x00 또는 0xFF 사용)


void delay (uint16_t time);

uint8_t calculate_CRC8(uint8_t *data, uint8_t len);

#endif /* INC_UTILS_UTILS_H_ */
