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

// 메시지큐 관리를 위해서 실제 패킷 정보를 담을 풀을 사용
osPoolDef(CanTxPool, 16, CAN_queue_pkt_t); // 16개짜리 풀 정의
osPoolId  CanTxPoolHandle;

/* -------------------------------------------------------------------------
   1. 초기화 및 설정 함수
   ------------------------------------------------------------------------- */
void CAN_init(void)
{
	// 풀 초기화
	CanTxPoolHandle = osPoolCreate(osPool(CanTxPool));

    // [V1] 큐 정의: V1 큐는 32비트 포인터를 저장합니다.
    // 구조체 자체(8바이트 이상)는 큐에 직접 못 넣으므로 포인터 타입을 명시합니다.
    osMessageQDef(CanTxQueue, 10, CAN_message_t*);
    canTxQueueHandle = osMessageCreate(osMessageQ(CanTxQueue), NULL);

    CAN_FilterTypeDef sFilterConfig;

    // 1. 필터 설정
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;

    // CAN 에서 ID 11bit -> 16bit 중에서 상위 11bit를 비교를 위해 사용한다
    // 그래서 왼쪽으로 5bit 만큼 Shift가 필요함 + 이렇게 표기하는게 직관적임

    // Mask = 내가 확인하려는 bit
    sFilterConfig.FilterMaskIdHigh = (0x7FF << 5);
    sFilterConfig.FilterMaskIdLow  = 0x0000;

    // 내가 수신하려는 메시지는 CAN_type_break_led 하나니깐 이거랑 ID가 일치하는지 확인하기
    sFilterConfig.FilterIdHigh = (CAN_type_break_led << 5);
    sFilterConfig.FilterIdLow  = 0x0000;

    // 수신 성공하면 CAN_RX_FIFO0 으로 넣기 (
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
void CAN_task_loop(void const * argument)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;

    CAN_queue_pkt_t *rxPacket;

    osEvent event;

    // [공통 설정] ID를 제외한 나머지 설정은 고정
    TxHeader.IDE = CAN_ID_STD;       // Standard ID
    TxHeader.RTR = CAN_RTR_DATA;     // Data Frame
    TxHeader.DLC = 8;                // Data Length 8
    TxHeader.TransmitGlobalTime = DISABLE;

    for(;;)
    {
        // 1. 큐 대기 (Blocking)
        event = osMessageGet(canTxQueueHandle, osWaitForever);

        if (event.status == osEventMessage)
        {
            // 2. 포인터 형변환 (새로운 구조체 타입으로 캐스팅)
            rxPacket = (CAN_queue_pkt_t *)event.value.p;

            if (rxPacket != NULL)
            {
                TxHeader.StdId = rxPacket->id;

                // 여기에서 TimeStamp + CRC 계산해야함
                rxPacket->body.field.time_ms = 0;
                // CRC 계산
                rxPacket->body.field.CRC_8 = calculate_CRC8(rxPacket->body.raw, 7);

                // 3. 메일박스 빈 공간 대기
                while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
                {
                    osDelay(1);
                }

                // 4. 실제 전송
                if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, rxPacket->body.raw, &TxMailbox) != HAL_OK)
                {
                    // 전송 에러 처리 (Error_Handler() 등)
                	printf("CAN Message Send Failed\r\n");
                }
                // 전송이 잘 진행되었다면 pool 할당 해제
                else
                {
                	osPoolFree(CanTxPoolHandle, rxPacket);
                }
            }
        }
    }
}

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

            switch(RxHeader.StdId >> 5) {
            case CAN_type_break_led:
                CAN_receive_led_signal(RxData[0]);
            	break;
            default:
            	break;
            }
        }
    }
}
