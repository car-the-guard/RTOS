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

typedef struct {
    uint32_t data;      // 4 bytes
    uint16_t time_ms;   // 2 bytes
    uint8_t  reserved;  // 1 byte
    uint8_t  CRC_8;     // 1 byte
} CAN_message_t;

/* 함수 원형 선언 */
void User_CAN_Init(void);        // 필터 설정 및 CAN 시작 함수
void CAN_task_loop(void *argument); // FreeRTOS 태스크 함수

#endif /* INC_CAN_CAN_H_ */
