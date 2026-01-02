/*
 * can.c
 *
 * Created on: Jan 2, 2026
 * Author: mokta
 */

#include "can.h"
#include <string.h> // memcpy 사용을 위해 필수

/* -------------------------------------------------------------------------
   1. 초기화 및 설정 함수
   ------------------------------------------------------------------------- */
void User_CAN_Init(void)
{
    CAN_FilterTypeDef sFilterConfig;

    // 1. 필터 설정 (모든 메시지 수신 허용)
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // FIFO 0으로 수신
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
   - Queue에 데이터가 들어오면 깨어나서 CAN 메시지를 전송합니다.
   ------------------------------------------------------------------------- */
void CAN_task_loop(void *argument)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;

    // 큐에서 꺼내온 데이터를 담을 구조체
    CAN_message_t txMsgContainer;
    osStatus_t status;

    // CAN Tx 헤더 설정 (ID는 고정 0x103 예시)
    // 만약 메시지마다 ID가 다르다면 구조체에 ID 필드를 추가해서 동적으로 할당해야 합니다.
    TxHeader.StdId = 0x103;
    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    for(;;)
    {
        /* [핵심] 큐 대기 (Blocking)
         * - 큐가 비어있으면 태스크는 Block state가 되어 CPU를 소모하지 않습니다.
         * - 다른 태스크가 osMessageQueuePut을 하면 즉시 깨어납니다.
         */
        status = osMessageQueueGet(canTxQueueHandle, &txMsgContainer, NULL, osWaitForever);

        if (status == osOK)
        {
            // 1. 구조체 -> 배열 직렬화 (Packing)
            memcpy(TxData, &txMsgContainer, sizeof(CAN_message_t));

            // 2. 메일박스 확인 (꽉 찼으면 빌 때까지 잠시 대기)
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
            {
                osDelay(1);
            }

            // 3. 실제 전송
            if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
            {
                // 전송 에러 처리 (필요 시)
            }
        }
    }
}

/* -------------------------------------------------------------------------
   3. 수신(Rx) 인터럽트 콜백 함수
   ------------------------------------------------------------------------- */
// 수신된 데이터를 확인할 전역 변수 (디버깅용)
CAN_message_t rxMsgDebug;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // CAN1 핸들러인지 확인
    if (hcan->Instance == CAN1)
    {
        // 메시지 가져오기
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            // [Rx 데이터 처리]
            // 8바이트 배열(RxData)을 다시 구조체로 변환 (Unpacking)
            memcpy(&rxMsgDebug, RxData, sizeof(CAN_message_t));

            /* 이제 rxMsgDebug.data, rxMsgDebug.time_ms 등의 값을
               Live Watch 등에서 확인하거나,
               Rx 처리용 Queue로 보내면 됩니다.

               예: xQueueSendFromISR(canRxQueueHandle, &rxMsgDebug, NULL);
            */

            // 여기는 ISR 내부라서 이런거 하면 안되지만, 디버깅용임
            printf("DEBUG - CAN RECEIVED : %02X %02X %02X %02X 02X 02X 02X 02X\r\n", RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7]);
        }
    }
}
