/*
 * can_bridge.h
 *
 *  Created on: Jan 3, 2026
 *      Author: mokta
 */

#ifndef INC_CAN_CAN_BRIDGE_H_
#define INC_CAN_CAN_BRIDGE_H_

#include <stdint.h>

typedef enum {
	CAN_type_collision = 0x008,
	CAN_type_sonar = 0x024,
	CAN_type_accel = 0x02C,
	CAN_type_compass = 0x030
} CAN_tx_message_id;

typedef enum {
	CAN_type_break_led = 0x048
} CAN_rx_message_id;


void CAN_send_collision(void);
void CAN_send_sonar(uint16_t, uint16_t);
void CAN_send_accel(uint16_t, uint16_t);
void CAN_send_compass(uint16_t);

void CAN_consume_rx_message(CAN_RxHeaderTypeDef RxHeader, CAN_payload_t payload);
void CAN_receive_led_signal(uint8_t);

#endif /* INC_CAN_CAN_BRIDGE_H_ */
