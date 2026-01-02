/*
 * MAX7219_matrix.c
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */


#include "max7219_matrix.h"
#include "max7219.h"
#include <string.h>
// [변경] 버퍼 구조: [Strip개수 2] x [Chip개수 4] x [Row개수 8]
volatile uint8_t FrameBuffer[MAX7219_NUM][MAX7219_IC_NUM][8];

void MAX7219_MatrixInit(SPI_HandleTypeDef* spi, GPIO_TypeDef* cs_ports[], uint16_t cs_pins[])
{
    MAX7219_Init(spi, cs_ports, cs_pins);

    // 2개의 Strip 반복
    for(int s = 0; s < MAX7219_NUM; s++)
    {
        // 각 Strip 내의 4개 Chip 반복
        for(int c = 0; c < MAX7219_IC_NUM; c++)
        {
            MAX7219_ShutDown(s, c, 1);    // Wake up
            MAX7219_Test(s, c, 0);        // No test
            MAX7219_Decode(s, c, 0);      // No decode
            MAX7219_Intensity(s, c, 1);   // Brightness (0~15)
            MAX7219_ScanLimit(s, c, 7);   // All digits

            // 버퍼 및 화면 클리어
            for(int r=0; r<8; r++) {
                FrameBuffer[s][c][r] = 0;
                MAX7219_Digit(s, c, r+1, 0);
            }
        }
    }
}

// 전체 업데이트 함수
bool MAX7219_MatrixUpdate()
{
    for(int s = 0; s < MAX7219_NUM; s++)
    {
        for(int c = 0; c < MAX7219_IC_NUM; c++)
        {
            for(int r = 0; r < 8; r++)
            {
                // Digit 주소는 1부터 8
                if(!MAX7219_Digit(s, c, r+1, FrameBuffer[s][c][r]))
                    return false;
            }
        }
    }
    return true;
}

// 픽셀 설정 함수 (좌표 계산 필요 시 여기서 처리)
// digit: 0~7 (Row)
void MAX7219_MatrixSetPixel(uint8_t strip_idx, uint8_t chip_idx, uint8_t digit, uint8_t row_data)
{
    if(strip_idx < MAX7219_NUM && chip_idx < MAX7219_IC_NUM && digit < 8) {
        FrameBuffer[strip_idx][chip_idx][digit] = row_data;
    }
}

// 전체 클리어
void MAX7219_MatrixClearAll()
{
    for(int s=0; s<MAX7219_NUM; s++)
        for(int c=0; c<MAX7219_IC_NUM; c++)
            for(int r=0; r<8; r++)
                FrameBuffer[s][c][r] = 0;
}

