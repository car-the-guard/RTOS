/*
 * grid_led.c
 *
 * Created on: Jan 1, 2026
 * Author: mokta
 */

// MAX7219 GRID LED를 이용

#include "grid_led.h"
#include "max7219_matrix.h"
#include "max7219.h"
#include "cmsis_os.h"

// 3차원 버퍼 참조 (Strip, Chip, Row)
extern volatile uint8_t FrameBuffer[MAX7219_NUM][MAX7219_IC_NUM][8];

GPIO_TypeDef* cs_ports[] = { BREAK_LED_CS0_GPIO_Port, BREAK_LED_CS1_GPIO_Port };
uint16_t      cs_pins[]  = { BREAK_LED_CS0_Pin,       BREAK_LED_CS1_Pin };

volatile GridLed_State_t current_led_state = GRID_LED_OFF;

// 화살표 패턴 (8x8)
const uint8_t arrow_bmp[8] = {
	0b00001111,
	0b00011110,
	0b00111100,
	0b01111000,
	0b01111000,
	0b00111100,
	0b00011110,
	0b00001111
};

// 헬퍼: 특정 Strip의 모든 칩 화면 지우기
static void Clear_Strip(uint8_t strip_idx) {
    for(int c = 0; c < MAX7219_IC_NUM; c++) {
        for(int r = 0; r < 8; r++) {
            MAX7219_MatrixSetPixel(strip_idx, c, r, 0x00);
        }
    }
}


void GRIDLED_init(SPI_HandleTypeDef* hspi)
{
    // [수정] 4개의 칩이 달린 2개의 Strip 초기화
    MAX7219_MatrixInit(hspi, cs_ports, cs_pins);
    MAX7219_MatrixUpdate();

    Clear_Strip(0);
    Clear_Strip(1);
}

void GRIDLED_Task_Loop(void const * argument)
{
    for(;;)
    {
        GRIDLED_Process();
        // Process 함수 내부에 osDelay가 있으므로 여기선 생략 가능하지만,
        // 안전을 위해 최소한의 Delay를 두는 것도 좋습니다.
        // osDelay(1);
    }
}

void GRIDLED_SetState(GridLed_State_t state)
{
	HAL_GPIO_TogglePin(GPIOB, GPIO_Pin_1);
    current_led_state = state;
}

// 내부용: Strip 0 (왼쪽 덩어리)의 모든 칩을 왼쪽으로 Shift
static void Shift_Strip0_Left(void) {
    // Strip 0번의 모든 Chip(0~3)에 대해 수행
    for(int c = 0; c < MAX7219_IC_NUM; c++) {
        for(int r = 0; r < 8; r++) {
            // [Strip 0][Chip c][Row r] 접근
            uint8_t msb = (FrameBuffer[0][c][r] & 0x80) ? 1 : 0;
            FrameBuffer[0][c][r] = (FrameBuffer[0][c][r] << 1) | msb;
        }
    }
}

// 내부용: Strip 1 (오른쪽 덩어리)의 모든 칩을 오른쪽으로 Shift
static void Shift_Strip1_Right(void) {
    // Strip 1번의 모든 Chip(0~3)에 대해 수행
    for(int c = 0; c < MAX7219_IC_NUM; c++) {
        for(int r = 0; r < 8; r++) {
            // [Strip 1][Chip c][Row r] 접근
            uint8_t msb = (FrameBuffer[1][c][r] & 0x80) ? 1 : 0;
            FrameBuffer[1][c][r] = (FrameBuffer[1][c][r] << 1) | msb;
        }
    }
}


// 헬퍼: 특정 Strip의 모든 칩에 패턴 채우기
static void Fill_Strip_Pattern(uint8_t strip_idx, const uint8_t* pattern) {
    for(int c = 0; c < MAX7219_IC_NUM; c++) {
        for(int r = 0; r < 8; r++) {
            MAX7219_MatrixSetPixel(strip_idx, c, r, pattern[r]);
        }
    }
}

// 헬퍼: 특정 Strip의 모든 칩 켜기/끄기 (Shutdown 제어)
static void Control_Strip_Power(uint8_t strip_idx, uint8_t is_on) {
    for(int c = 0; c < MAX7219_IC_NUM; c++) {
        MAX7219_ShutDown(strip_idx, c, is_on);
    }
}

void GRIDLED_Process(void)
{
    static GridLed_State_t prev_state = -1;
    static uint8_t blink_flag = 0;

    // 1. 상태 변경 시 초기화 로직
    if (prev_state != current_led_state)
    {
        // 일단 화면 내용은 다 지움
        MAX7219_MatrixClearAll();

        // 상태 변경 시 모든 칩을 WakeUp (혹시 Blink에서 꺼졌을까봐)
        Control_Strip_Power(0, 1);
        Control_Strip_Power(1, 1);

        if (current_led_state == GRID_LED_ANIMATE) {
            // 애니메이션 초기 패턴 로드
        	// TODO 주석 수정 필요
            Fill_Strip_Pattern(0, arrow_bmp);  // 왼쪽 Strip: 왼쪽 화살표
            Fill_Strip_Pattern(1, arrow_bmp); // 오른쪽 Strip: 오른쪽 화살표
        }

        MAX7219_MatrixUpdate();
        prev_state = current_led_state;
    }

    // 2. 주기적 동작 로직
    switch (current_led_state)
    {
        case GRID_LED_OFF:
            // 0: 끄기 (화면 내용을 비우고 업데이트)
            // 이미 위에서 ClearAll 했으므로 유지
            osDelay(200);
            break;

        case GRID_LED_ON:
            // 1: 켜기 (모든 픽셀 On)
            for(int s=0; s<MAX7219_NUM; s++) {
                for(int c=0; c<MAX7219_IC_NUM; c++) {
                    for(int r=0; r<8; r++) {
                        MAX7219_MatrixSetPixel(s, c, r, 0xFF);
                    }
                }
            }
            MAX7219_MatrixUpdate();
            osDelay(200);
            break;

        case GRID_LED_BLINK:
            // 2: 점멸 (Shutdown 기능 활용)
            // 2개의 Strip, 각각 4개의 Chip 모두 제어
            if (blink_flag) {
                Control_Strip_Power(0, 1); // 켜기
                Control_Strip_Power(1, 1);
            } else {
                Control_Strip_Power(0, 0); // 끄기 (절전)
                Control_Strip_Power(1, 0);
            }

            // Blink 모드일 때는 내용은 꽉 채워둠 (켜졌을 때 보이게)
            if(blink_flag) {

				Fill_Strip_Pattern(0, arrow_bmp);
				Fill_Strip_Pattern(1, arrow_bmp);
				MAX7219_MatrixUpdate();
            }

            blink_flag = !blink_flag;
            osDelay(200); // 200ms 주기
            break;

        case GRID_LED_ANIMATE:
            // 3: 애니메이션
            Shift_Strip0_Left();  // 왼쪽 덩어리 회전
            Shift_Strip1_Right(); // 오른쪽 덩어리 회전

            MAX7219_MatrixUpdate();
            osDelay(50); // 속도 조절
            break;
    }
}
