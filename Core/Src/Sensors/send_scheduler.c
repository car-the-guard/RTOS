/*
 * send_scheduler.c
 *
 *  Created on: Jan 3, 2026
 *      Author: mokta
 */

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "send_scheduler.h"
#include "can_bridge.h"
#include "accel.h"

void SCHEDULER_init(void)
{

}

void SCHEDULER_task_loop(void const * argument)
{
	// 1. [초기화] 각 센서의 마지막 실행 시간을 저장할 변수들
	uint32_t last_accel     = 0;
	uint32_t last_sonar     = 0;
	uint32_t last_compass   = 0;

	// 현재 시스템 시간
	uint32_t current_tick;



	for(;;) {
		// 2. 현재 시간 가져오기 (HAL_GetTick은 ms 단위 리턴)
		current_tick = HAL_GetTick();


		/* ============================================================
		   [센서 2] 가속도 센서 (주기: 100ms)
		   ============================================================ */
		if ((current_tick - last_accel) >= SCHEDULER_PERIOD_ACCEL_MS)
		{
			AccelData_report_t data;
			ACCEL_get_data(&data);

			CAN_send_accel(data.instant_Ax, data.avg_Ax);

			last_accel = current_tick;
		}

		/* ============================================================
		   [센서 3] 초음파 센서 (주기: 200ms)
		   ============================================================ */
		if ((current_tick - last_sonar) >= SCHEDULER_PERIOD_SONAR_MS)
		{
			uint16_t dist = Sensor_Get_Sonar_Distance();

			Send_Sonar(dist);

			last_sonar = current_tick;
		}

		/* ============================================================
		   [센서 4] 나침반 센서 (주기: 500ms)
		   ============================================================ */
		if ((current_tick - last_compass) >= SCHEDULER_PERIOD_COMPASS_MS)
		{
			uint16_t angle = Sensor_Get_Compass_Angle();

			// 나침반 메시지는 보통 1byte 모드+2byte 값이므로 그에 맞춰 보냄
			Send_Motor_Status(0, angle); // 예시 함수

			last_compass = current_tick;
		}

		/* ============================================================
		   [중요] CPU 양보 (Context Switch)
		   루프를 너무 빨리 돌 필요가 없으므로 잠시 대기합니다.
		   이 값이 '시간 분해능(Resolution)'이 됩니다.
		   ============================================================ */
		osDelay(SCHEDULER_TICK_MS);
	}
}
