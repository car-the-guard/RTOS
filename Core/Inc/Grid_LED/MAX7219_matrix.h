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

#define CHR(x)	Atoz[(x)- 65]

extern const uint64_t symbol[];
extern const uint64_t numbers[];
extern const uint64_t Atoz[];

void MAX7219_MatrixInit(SPI_HandleTypeDef* spi, GPIO_TypeDef* cs_port, uint16_t cs_pin);
void MAX7219_MatrixClear(uint8_t index);
void MAX7219_MatrixSetRow(uint8_t index,  uint8_t rows[8]);
void MAX7219_MatrixSetRow64(uint8_t index,  uint64_t rows);
void MAX7219_MatrixSetPixel(uint8_t index, uint8_t row, uint8_t col);
void MAX7219_MatrixLShift(uint8_t step);
void MAX7219_MatrixRShift(uint8_t step);
bool MAX7219_MatrixUpdate();

#endif /* INC_GRID_LED_MAX7219_MATRIX_H_ */
