/*
 * can.c
 *
 * Created on: Jan 2, 2026
 * Author: mokta
 * Modified for: CMSIS-RTOS V1 Compliance
 */

#include "can.h"
#include "cmsis_os.h" // CMSIS V1 헤더
#include <string.h>   // memcpy
#include <stdio.h>    // printf

/* -------------------------------------------------------------------------
   전역 변수 및 핸들 정의
   ------------------------------------------------------------------------- */
extern CAN_HandleTypeDef hcan1; // main.c 등에서 정의된 핸들 가져오기

extern osMessageQId canTxQueueHandle;

/* -------------------------------------------------------------------------
   1. 초기화 및 설정 함수
   ------------------------------------------------------------------------- */
void User_CAN_Init(void)
{
    // [V1] 큐 정의: V1 큐는 32비트 포인터를 저장합니다.
    // 구조체 자체(8바이트 이상)는 큐에 직접 못 넣으므로 포인터 타입을 명시합니다.
    osMessageQDef(CanTxQueue, 10, CAN_message_t*);
    canTxQueueHandle = osMessageCreate(osMessageQ(CanTxQueue), NULL);

    CAN_FilterTypeDef sFilterConfig;

    // 1. 필터 설정 (모든 메시지 수신 허용)
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    // 2. CAN 동작 시작
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    // 3. Rx 인터럽트 활성화
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }
}

/* -------------------------------------------------------------------------
   2. FreeRTOS 송신(Tx) 태스크 - Gatekeeper Implementation
   ------------------------------------------------------------------------- */
void CAN_task_loop(void const * argument) // [V1] 매개변수 타입이 void const * 입니다.
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;

    // 큐에서 꺼낸 포인터를 받을 변수
    CAN_message_t *rxMsgPtr;

    // [V1] 결과를 받기 위한 이벤트 구조체
    osEvent event;

    // CAN Tx 헤더 기본 설정
    TxHeader.StdId = 0x103;
    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    for(;;)
    {
        /* [핵심] 큐 대기 (Blocking) - V1 스타일
         * osMessageGet은 osEvent 구조체를 반환합니다.
         */
        event = osMessageGet(canTxQueueHandle, osWaitForever);

        // 이벤트가 메시지 도착인지 확인
        if (event.status == osEventMessage)
        {
            // [V1] 값 꺼내기 (포인터로 형변환)
            // 주의: 보내는 쪽(Sender)에서도 반드시 '주소'를 보냈어야 합니다.
            rxMsgPtr = (CAN_message_t *)event.value.p;

            // 1. 데이터 복사 (구조체 -> 배열 직렬화)
            // rxMsgPtr이 가리키는 유효한 메모리에서 데이터를 가져옵니다.
            if (rxMsgPtr != NULL) {
                memcpy(TxData, rxMsgPtr, sizeof(CAN_message_t));
            }

            // 2. 메일박스 확인
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
            {
                osDelay(1);
            }

            // 3. 실제 전송
            if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
            {
                // 전송 에러 처리
            }
        }
    }
}

/* -------------------------------------------------------------------------
   3. 수신(Rx) 인터럽트 콜백 함수
   ------------------------------------------------------------------------- */
CAN_message_t rxMsgDebug;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if (hcan->Instance == CAN1)
    {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            memcpy(&rxMsgDebug, RxData, sizeof(CAN_message_t));

            /* * [주의] ISR 내부에서의 printf 사용
             * 실제 제품 코드에서는 ISR 내 printf 사용을 권장하지 않습니다 (Blocking 유발 가능성).
             * 디버깅용으로만 사용하시고, 추후 삭제를 권장합니다.
             */
            printf("DEBUG - CAN RECEIVED : %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                   RxData[0], RxData[1], RxData[2], RxData[3],
                   RxData[4], RxData[5], RxData[6], RxData[7]);
        }
    }
}
