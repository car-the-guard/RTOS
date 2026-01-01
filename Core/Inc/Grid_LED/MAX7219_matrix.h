/*
 * MAX7219_matrix.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_GRID_LED_MAX7219_MATRIX_H_
#define INC_GRID_LED_MAX7219_MATRIX_H_


#include "stm32f4xx_hal.h"
#include <stdbool.h>

// [수정] Init 함수: 배열 포인터를 받습니다.
void MAX7219_MatrixInit(SPI_HandleTypeDef* spi, GPIO_TypeDef* cs_ports[], uint16_t cs_pins[]);

// [수정] Pixel 설정 함수: 인자 4개 (Strip 인덱스, Chip 인덱스, Digit, 데이터)
// 예전: void MAX7219_MatrixSetPixel(uint8_t index, uint8_t digit, uint8_t row);
// 변경 후:
void MAX7219_MatrixSetPixel(uint8_t strip_idx, uint8_t chip_idx, uint8_t digit, uint8_t row_data);

// [수정] 전체 클리어 함수 (인자 없음)
void MAX7219_MatrixClearAll(void);

// 나머지 함수들도 필요하다면 인자를 맞춰야 하지만,
// 현재 사용하지 않는다면 주석 처리하거나 아래처럼 업데이트 필요:
void MAX7219_MatrixSetRow(uint8_t strip_idx, uint8_t chip_idx, uint8_t rows[8]);
bool MAX7219_MatrixUpdate(void);

#endif /* INC_GRID_LED_MAX7219_MATRIX_H_ */
