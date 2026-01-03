/*
 * send_scheduler.h
 *
 *  Created on: Jan 3, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_SEND_SCHEDULER_H_
#define INC_SENSORS_SEND_SCHEDULER_H_

/* 센서별 전송 주기 설정 (단위: ms) */
#define SCHEDULER_PERIOD_ACCEL_MS       100  // 10Hz
#define SCHEDULER_PERIOD_SONAR_MS       200  // 5Hz
#define SCHEDULER_PERIOD_COMPASS_MS     500  // 2Hz (여유)

/* 스케줄러 루프 주기 (Tick Resolution) */
#define SCHEDULER_TICK_MS     10   // 10ms마다 검사

void SCHEDULER_init(void);
void SCHEDULER_task_loop(void const * argument);

#endif /* INC_SENSORS_SEND_SCHEDULER_H_ */
