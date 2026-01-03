/*
 * can_bridge.c
 *
 *  Created on: Jan 3, 2026
 *      Author: mokta
 */

#include "can_bridge.h"
#include "can.h"
#include "grid_led.h"

extern osPoolId  CanTxPoolHandle;

void CAN_send_collision(void)
{
	CAN_queue_pkt_t *pPacket = (CAN_queue_pkt_t *)osPoolAlloc(CanTxPoolHandle);

	if (pPacket != NULL) {
		pPacket->id = CAN_type_collision;

		pPacket->body.field.data.u8_val = 0xFF;

		osMessagePut(canTxQueueHandle, (uint32_t)pPacket, 0);
	}
}

void CAN_send_sonar(uint16_t distance0, uint16_t distance1)
{
	CAN_queue_pkt_t *pPacket = (CAN_queue_pkt_t *)osPoolAlloc(CanTxPoolHandle);

	if (pPacket != NULL) {
		pPacket->id = CAN_type_sonar;

		pPacket->body.field.data.dual_u16.val_A = distance0;
		pPacket->body.field.data.dual_u16.val_B = distance1;

		osMessagePut(canTxQueueHandle, (uint32_t)pPacket, 0);
	}
}

void CAN_send_accel(uint16_t accel_moment, uint16_t accel_filtered)
{
	CAN_queue_pkt_t *pPacket = (CAN_queue_pkt_t *)osPoolAlloc(CanTxPoolHandle);

	if (pPacket != NULL) {
		pPacket->id = CAN_type_accel;

		pPacket->body.field.data.dual_u16.val_A = accel_moment;
		pPacket->body.field.data.dual_u16.val_B = accel_filtered;

		osMessagePut(canTxQueueHandle, (uint32_t)pPacket, 0);
	}
}

void CAN_send_compass(uint16_t heading)
{
	CAN_queue_pkt_t *pPacket = (CAN_queue_pkt_t *)osPoolAlloc(CanTxPoolHandle);

	if (pPacket != NULL) {
		pPacket->id = CAN_type_compass;

		pPacket->body.field.data.u16_val = heading;

		osMessagePut(canTxQueueHandle, (uint32_t)pPacket, 0);
	}
}

void CAN_consume_rx_message(CAN_RxHeaderTypeDef RxHeader, CAN_payload_t payload)
{
	uint32_t id = RxHeader->StdId;

	switch(id) {
	case CAN_type_break_led:
		CAN_receive_led_signal(payload.field.data.u8_val);
		break;
	default:
		break;
	}
}

void CAN_receive_led_signal(uint8_t type)
{
	GRIDLED_SetState((GridLed_State_t) type);
}
