/*
 * MAX7219.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_GRID_LED_MAX7219_H_
#define INC_GRID_LED_MAX7219_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#define MAX7219_NUM     2  /* CS 핀의 개수 (Strip 개수) - 좌/우 */
#define MAX7219_IC_NUM  4  /* 한 CS 라인에 연결된 칩(모듈) 개수 (4-in-1 모듈) */

/* Register Addresses */
#define MAX7219_REG_NOOP        0x00
#define MAX7219_REG_DIGIT0      0x01
#define MAX7219_REG_DECODE      0x09
#define MAX7219_REG_INTENSITY   0x0A
#define MAX7219_REG_SCANLIMIT   0x0B
#define MAX7219_REG_SHUTDOWN    0x0C
#define MAX7219_REG_TEST        0x0F

/* Functions */
void MAX7219_Init(SPI_HandleTypeDef* spi, GPIO_TypeDef* ss_ports[], uint16_t ss_pins[]);

/* 핵심 쓰기 함수: (어느 Strip의, 몇 번째 Chip에, 무슨 레지스터, 무슨 값) */
bool MAX7219_Write(uint8_t strip_idx, uint8_t chip_idx, uint8_t reg, uint8_t data);

/* 설정 함수들도 strip/chip index를 받도록 변경 */
bool MAX7219_ShutDown(uint8_t strip_idx, uint8_t chip_idx, uint8_t value);
bool MAX7219_Test(uint8_t strip_idx, uint8_t chip_idx, uint8_t value);
bool MAX7219_Decode(uint8_t strip_idx, uint8_t chip_idx, uint8_t value);
bool MAX7219_Intensity(uint8_t strip_idx, uint8_t chip_idx, uint8_t value);
bool MAX7219_ScanLimit(uint8_t strip_idx, uint8_t chip_idx, uint8_t value);
bool MAX7219_Digit(uint8_t strip_idx, uint8_t chip_idx, uint8_t digit, uint8_t value);

#endif /* INC_GRID_LED_MAX7219_H_ */
