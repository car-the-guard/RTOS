/*
 * collision.c
 * Created on: Jan 2, 2026
 * Author: mokta
 */

#include "collision.h"
#include <stdio.h>

// 디바운싱 시간 (ms)
#define DEBOUNCE_DELAY  200

// 세마포어 핸들
static osSemaphoreId sem_collisionHandle = NULL;

// 디바운싱용 변수
static volatile uint32_t last_interrupt_time = 0;

// 1. 초기화 함수
void COLLISION_Init(void)
{
    // [1] 바이너리 세마포어 생성
    // CMSIS RTOS v1 기준 매크로 사용
    osSemaphoreDef(SEM_COLLISION);
    sem_collisionHandle = osSemaphoreCreate(osSemaphore(SEM_COLLISION), 1);

    if(sem_collisionHandle == NULL) {
    	printf("NONONONO\r\n");
    }
    // 처음에는 세마포어를 가져가서(Take) '비어있는(Empty)' 상태로 만듦
    // 그래야 Task가 Wait 할 때 바로 잠들 수 있음.
    if (sem_collisionHandle != NULL) {
        osSemaphoreWait(sem_collisionHandle, 0);
    }

    last_interrupt_time = 0;

    printf(">> [INIT] Collision Sensor Ready (Semaphore Mode).\r\n");
}

// 2. [핵심] Task가 충돌을 기다릴 때 호출하는 함수
// 이 함수를 호출하면 충돌이 올 때까지 Task가 멈춥니다(Blocked). CPU 사용률 0%
BaseType_t COLLISION_WaitForSignal(uint32_t timeout)
{
    if (sem_collisionHandle == NULL) return osErrorOS;

    // 세마포어가 올 때까지 대기 (Sleep)
    return osSemaphoreWait(sem_collisionHandle, timeout);
}

// =============================================================
// [EXTI ISR] 하드웨어 인터럽트가 발생하면 호출됨
// =============================================================
/* stm32fxxx_it.c 상단에 include 필요 */
#include "FreeRTOS.h"
#include "semphr.h"  // FreeRTOS 세마포어 헤더
#include "cmsis_os.h" // osSemaphoreId 정의

// ...

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // 문맥 전환 요청 여부를 저장할 변수 (초기값 False)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == COLLISION_PIN)
    {
        uint32_t current_time = HAL_GetTick();

        if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY)
        {
            last_interrupt_time = current_time;

            if (sem_collisionHandle != NULL)
            {
                /* * [수정] 일반 함수 대신 FromISR 버전 사용
                 * (SemaphoreHandle_t)로 캐스팅 필요 (CMSIS V1 핸들과 호환됨)
                 */
                xSemaphoreGiveFromISR((SemaphoreHandle_t)sem_collisionHandle, &xHigherPriorityTaskWoken);

                /* * [중요] 세마포어를 받고 깨어난 Task가 현재 Task보다 우선순위가 높다면,
                 * ISR이 끝나는 즉시 그 Task로 화면 전환(Context Switch)을 요청함.
                 */
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}
