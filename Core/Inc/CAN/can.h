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

typedef struct {
    uint32_t data;      // 4 bytes
    uint16_t time_ms;   // 2 bytes
    uint8_t  reserved;  // 1 byte
    uint8_t  CRC_8;     // 1 byte
} CAN_message_t;

typedef enum {
	CAN_type_collision = 0x008,
	CAN_type_sonar = 0x024,
	CAN_type_accel = 0x02C,
	CAN_type_compass = 0x030
} CAN_tx_message_id;

typedef enum {
	CAN_type_break_led = 0x048
} CAN_rx_message_id;

/* 함수 원형 선언 */
void CAN_init(void);        // 필터 설정 및 CAN 시작 함수
void CAN_task_loop(void const * argument); // FreeRTOS 태스크 함수

#endif /* INC_CAN_CAN_H_ */
