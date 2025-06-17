#include "can_node_1.h"
#include "gpio.h"

//volatile uint8_t CANDataRcvFlag = 0;
volatile uint8_t CANRxBuffer[CAN_DATA_LENGTH];
CAN_RxHeaderTypeDef CANRxHeader;

int SAE_J1850_Calc(int data[], int len)
{
	int crc, temp1, temp2;
	crc 	= 0;
	temp1 	= 0;
	temp2 	= 0;
	for (int _idx = 0; _idx < len; ++ _idx)
	{
		if (0 == _idx)
		{
			temp1 = 0;
		}
		else
		{
			temp1 = data[len - _idx];
		}
		crc = crc ^ temp1;
		for (int _idy = 8; _idy > 0; -- _idy)
		{
			temp2 = crc;
			crc = crc << 1;
			if (0 != (temp2 & 128))
			{
				crc = crc ^ 0x1d;
			}
		}
	}
	return crc;
}

void CAN_Transmit(CAN_HandleTypeDef *hcan, const CAN_TxHeaderTypeDef *pHeader,
        		const uint8_t aData[], uint32_t *pTxMailbox)
{
	HAL_CAN_AddTxMessage(hcan, pHeader, aData, pTxMailbox);
}

//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
//	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
//	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);
//
//	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CANRxHeader, CANRxBuffer) != HAL_OK)
//	{
//		Error_Handler();
//	}
//
//	if (CANRxHeader.StdId == CAN_RX_STD_ID)
//	{
//		CANDataRcvFlag = 1;
//	}
//}
