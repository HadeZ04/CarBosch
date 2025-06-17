#include "can_project_sensor.h"

extern uint8_t CANRxBuffer[];
extern CAN_RxHeaderTypeDef CANRxHeader;
volatile uint8_t CANDataRcvFlag;


void CANSensorTransmit(CAN_HandleTypeDef *hcan, CANSensorData* controlData)
{
	uint8_t CANTxControl[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderControl;
	uint32_t CANTxMailboxesControl = CAN_TX_MAILBOX1;

	CANTxHeaderControl.StdId = CAN_PROJECT_SENSOR_STDID;
	CANTxHeaderControl.IDE 	 = CAN_ID_STD;
	CANTxHeaderControl.RTR 	 = CAN_RTR_DATA;
	CANTxHeaderControl.DLC 	 = CAN_DATA_LENGTH;

	CANTxControl[0] 	= (controlData->sequence >> 8) & 0xff;
	CANTxControl[1] 	= (controlData->sequence) & 0xff;
	CANTxControl[2] 	= controlData->priority;
	CANTxControl[3]		= controlData->speed;
	CANTxControl[4]		= controlData->direction1;
	CANTxControl[5]		= controlData->direction2;
	CANTxControl[6]		= UNUSED_DATA;
	CANTxControl[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderControl, CANTxControl, &CANTxMailboxesControl);
	HAL_GPIO_TogglePin(GPIO_Port, LEDR_Pin);
}

void CANResponseCheck()
{
//	HAL_GPIO_TogglePin(GPIO_Port, LEDG_Pin);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CANRxHeader, CANRxBuffer) != HAL_OK)
	{
		Error_Handler();
	}

	if (CANRxHeader.StdId == CAN_PROJECT_ACTUATOR_STDID)
	{
		CANDataRcvFlag = 1;
	}
}
