/*
 * sonar.h
 *
 *  Created on: Jan 1, 2026
 *      Author: mokta
 */

#ifndef INC_SENSORS_SONAR_H_
#define INC_SENSORS_SONAR_H_

#include <main.h>
#include <stm32f4xx_hal.h>

#define TRIG_PORT_1 SONAR1_TRIGGER_GPIO_Port
#define TRIG_PIN_1  SONAR1_TRIGGER_Pin

#define TRIG_PORT_2 SONAR2_TRIGGER_GPIO_Port
#define TRIG_PIN_2  SONAR2_TRIGGER_Pin

void HCSR04_Read (void);

#endif /* INC_SENSORS_SONAR_H_ */
