#include "can_project_actuator.h"

extern uint8_t CANRxBuffer[];
extern CAN_RxHeaderTypeDef CANRxHeader;
volatile uint8_t CANDataRcvFlag;
extern float outPID;
extern uint8_t PWM;
volatile uint8_t urgent_mode = 0;
extern uint32_t timeElapsed;

void CANActuatorResponse(CAN_HandleTypeDef *hcan, CANActuatorData* responseData)
{
	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_PROJECT_ACTUATOR_STDID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[CAN_DATA_SEQ_IDX] 			= (responseData->sequence >> 8) & 0xff;
	CANTxResponse[CAN_DATA_SEQ_IDX+1]			= (responseData->sequence) & 0xff;
	CANTxResponse[CAN_ACTUATOR_DATA_SPEED_IDX] 	= responseData->speed;
	CANTxResponse[3]		= UNUSED_DATA;
	CANTxResponse[4]		= UNUSED_DATA;
	CANTxResponse[4]		= UNUSED_DATA;
	CANTxResponse[5]		= UNUSED_DATA;
	CANTxResponse[6]		= UNUSED_DATA;
	CANTxResponse[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);

}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CANRxHeader, CANRxBuffer) != HAL_OK)
	{
		Error_Handler();
	}

	if (CANRxHeader.StdId == CAN_PROJECT_SENSOR_STDID)
	{
		CANDataRcvFlag = 1;
		timeElapsed = 0;
		CONTROL_PRIORITY priority = CANRxBuffer[CAN_SENSOR_DATA_PRI_IDX];
		if (priority == CONTROL_PRIOR_URGENT)
		{
			urgent_mode = 1;
		}
	}
}
