/*
 * MAX7219.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#include "MAX7219.h"

SPI_HandleTypeDef* Max7219_SPI;

// CS 핀 정보 저장 (Strip 개수만큼)
GPIO_TypeDef* Max7219_SS_Ports[MAX7219_NUM];
uint16_t Max7219_SS_Pins[MAX7219_NUM];

static void SS_Select(uint8_t strip_idx);
static void SS_Deselect(uint8_t strip_idx);
static bool SPI_Tx(uint8_t data);
static void DelayInit(void);
static void DelayUS(uint32_t);

void MAX7219_Init(SPI_HandleTypeDef* spi, GPIO_TypeDef* ss_ports[], uint16_t ss_pins[])
{
    Max7219_SPI = spi;
    DelayInit();

    for(int i = 0; i < MAX7219_NUM; i++) {
        Max7219_SS_Ports[i] = ss_ports[i];
        Max7219_SS_Pins[i] = ss_pins[i];
        // 초기화: 모든 CS High (선택 해제)
        HAL_GPIO_WritePin(Max7219_SS_Ports[i], Max7219_SS_Pins[i], GPIO_PIN_SET);
    }
}

// [핵심 로직] Hybrid 방식: Parallel Select + Daisy Chain Data Shift
bool MAX7219_Write(uint8_t strip_idx, uint8_t chip_idx, uint8_t reg, uint8_t data)
{
    if(strip_idx >= MAX7219_NUM) return false;
    if(chip_idx >= MAX7219_IC_NUM) return false;

    // 1. 원하는 Strip의 CS 핀만 내림 (Select)
    SS_Select(strip_idx);

    // 2. 타겟 칩보다 "뒤에 있는" 칩들에게 NO-OP 전송
    // 예: 4개 중 2번째(idx 1)에 쓰려면, 칩3, 칩2에게 NO-OP을 보내서 밀어줘야 함
    for(int i = MAX7219_IC_NUM - 1; i > chip_idx; i--)
    {
        SPI_Tx(MAX7219_REG_NOOP); // Reg
        SPI_Tx(0x00);             // Data
    }

    // 3. 타겟 칩에게 진짜 데이터 전송
    SPI_Tx(reg);
    SPI_Tx(data);

    // 4. 타겟 칩보다 "앞에 있는" 칩들에게 NO-OP 전송
    // 예: 칩 0에게 NO-OP
    for(int i = 0; i < chip_idx; i++)
    {
        SPI_Tx(MAX7219_REG_NOOP);
        SPI_Tx(0x00);
    }

    // 5. CS 핀 올림 (Latch) - 이때 모든 칩이 데이터를 받아들임
    SS_Deselect(strip_idx);

    return true;
}

/* 래퍼 함수들 업데이트 */
bool MAX7219_Digit(uint8_t strip_idx, uint8_t chip_idx, uint8_t digit, uint8_t value){
	return MAX7219_Write(strip_idx, chip_idx, digit, value); // digit은 1~8 사용
}
bool MAX7219_ShutDown(uint8_t strip_idx, uint8_t chip_idx, uint8_t value) {
    return MAX7219_Write(strip_idx, chip_idx, MAX7219_REG_SHUTDOWN, value);
}
bool MAX7219_Test(uint8_t strip_idx, uint8_t chip_idx, uint8_t value) {
    return MAX7219_Write(strip_idx, chip_idx, MAX7219_REG_TEST, value);
}
bool MAX7219_Decode(uint8_t strip_idx, uint8_t chip_idx, uint8_t value) {
    return MAX7219_Write(strip_idx, chip_idx, MAX7219_REG_DECODE, value);
}
bool MAX7219_Intensity(uint8_t strip_idx, uint8_t chip_idx, uint8_t value) {
    return MAX7219_Write(strip_idx, chip_idx, MAX7219_REG_INTENSITY, value);
}
bool MAX7219_ScanLimit(uint8_t strip_idx, uint8_t chip_idx, uint8_t value) {
    return MAX7219_Write(strip_idx, chip_idx, MAX7219_REG_SCANLIMIT, value);
}

/* 내부 함수들 */
static void SS_Select(uint8_t strip_idx) {
    HAL_GPIO_WritePin(Max7219_SS_Ports[strip_idx], Max7219_SS_Pins[strip_idx], GPIO_PIN_RESET);
}

static void SS_Deselect(uint8_t strip_idx) {
    HAL_GPIO_WritePin(Max7219_SS_Ports[strip_idx], Max7219_SS_Pins[strip_idx], GPIO_PIN_SET);
}

static bool SPI_Tx(uint8_t data) {
    return (HAL_SPI_Transmit(Max7219_SPI, &data, 1, 10) == HAL_OK);
}

static void DelayInit(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}
static void DelayUS(uint32_t us) { /* 사용 안함 (SPI 속도로 충분) */ }
