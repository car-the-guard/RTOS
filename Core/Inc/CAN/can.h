/*
 * can.h
 *
 *  Created on: Jan 2, 2026
 *      Author: mokta
 */

#ifndef INC_CAN_CAN_H_
#define INC_CAN_CAN_H_

#include "main.h"
#include "cmsis_os.h"

/* 외부에서 참조할 핸들러 (main.c에 정의되어 있다고 가정) */
extern CAN_HandleTypeDef hcan1;
extern osMessageQId canTxQueueHandle; // Gatekeeper용 큐 핸들

#pragma pack(push, 1) // 패딩 방지 (필수)
typedef union {
    struct {
        union {
            uint32_t u32_all;    // 전체 4바이트 (디버깅용 또는 32비트 값)
            uint8_t  u8_val;     // [Case 1] 1 byte (Byte 0 사용)
            uint16_t u16_val;    // [Case 2] 2 bytes (Byte 0~1 사용)
            struct {             // [Case 3] 2 bytes x 2개 (Byte 0~1, Byte 2~3)
                int16_t val_A;   // Byte 0~1 (예: X축 가속도)
                int16_t val_B;   // Byte 2~3 (예: Y축 가속도)
            } dual_u16;
        } data;

        uint16_t time_ms;   // 2 bytes (공통)
        uint8_t  reserved;  // 1 byte  (공통)
        uint8_t  CRC_8;     // 1 byte  (공통)
    } field;

    uint8_t raw[8];
} CAN_payload_t;
#pragma pack(pop)

typedef struct {
    uint16_t 		id;
	CAN_payload_t 	body;
} CAN_queue_pkt_t;

/* 함수 원형 선언 */
void CAN_init(void);        				// 필터 설정 및 CAN 시작 함수
void CAN_task_loop(void const * argument); 	// FreeRTOS 태스크 함수

#endif /* INC_CAN_CAN_H_ */
