/*
 * collision.h
 * Created on: Jan 2, 2026
 * Author: mokta
 */

#ifndef INC_SENSORS_COLLISION_H_
#define INC_SENSORS_COLLISION_H_

#include "main.h"
#include "cmsis_os.h" // 세마포어 사용을 위해 필수
#include <stdbool.h>

// [설정] 충돌 센서가 연결된 핀 정의
// 예: PA0 핀을 EXTI로 설정했다고 가정
#define COLLISION_PIN       GPIO_PIN_13
#define COLLISION_PORT      GPIOF

// 함수 원형
void COLLISION_Init(void);

// timeout: 기다릴 시간 (osWaitForever면 무한 대기)
BaseType_t COLLISION_WaitForSignal(uint32_t timeout);

#endif /* INC_SENSORS_COLLISION_H_ */
