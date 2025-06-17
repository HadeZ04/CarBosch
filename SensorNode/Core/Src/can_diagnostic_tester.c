#include "can_diagnostic_tester.h"
#include "can_node_2.h"

extern uint8_t CANRxBuffer[];
extern CAN_RxHeaderTypeDef CANRxHeader;
uint8_t CANDiagnosticResponseRcvFlag = 0;
uint8_t seed[4], key[16];

void writeShortDataByIdentifierRequest(CAN_HandleTypeDef *hcan)
{
	uint8_t CANTxRequest[READ_WRITE_REQUEST_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderRequest;
	uint32_t CANTxMailboxesRequest = CAN_TX_MAILBOX1;

	CANTxHeaderRequest.StdId = CAN_DIAGNOSTIC_REQUEST_ID;
	CANTxHeaderRequest.IDE 	= CAN_ID_STD;
	CANTxHeaderRequest.RTR 	= CAN_RTR_DATA;
	CANTxHeaderRequest.DLC 	= READ_WRITE_REQUEST_LENGTH;

	CANTxRequest[0] 	= 0x03;
	CANTxRequest[1] 	= 0x2e;
	CANTxRequest[2]		= 0x01;
	CANTxRequest[3]		= 0x23;
	CANTxRequest[4]		= UNUSED_DATA;
	CANTxRequest[5]		= UNUSED_DATA;
	CANTxRequest[6]		= UNUSED_DATA;
	CANTxRequest[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

	HAL_GPIO_TogglePin(GPIO_Port, LEDR_Pin);
}

void writeDataByIdenfierResponseCheck(uint8_t CANRxBuffer[])
{
	if (CANRxBuffer[0] == WRITE_DATA_BY_ID_RESPONSE_SID)
	{
		HAL_GPIO_TogglePin(GPIO_Port, LEDB_Pin);
	}
}

void readDataByIdenfierRequest(CAN_HandleTypeDef *hcan)
{
	uint8_t CANTxRequest[READ_WRITE_REQUEST_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderRequest;
	uint32_t CANTxMailboxesRequest = CAN_TX_MAILBOX1;

	CANTxHeaderRequest.StdId 	= CAN_DIAGNOSTIC_REQUEST_ID;
	CANTxHeaderRequest.IDE 	= CAN_ID_STD;
	CANTxHeaderRequest.RTR 	= CAN_RTR_DATA;
	CANTxHeaderRequest.DLC 	= READ_WRITE_REQUEST_LENGTH;

	CANTxRequest[0] 	= 0x03;
	CANTxRequest[1] 	= 0x22;
	CANTxRequest[2]		= 0x01;
	CANTxRequest[3]		= 0x23;
	CANTxRequest[4]		= UNUSED_DATA;
	CANTxRequest[5]		= UNUSED_DATA;
	CANTxRequest[6]		= UNUSED_DATA;
	CANTxRequest[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

	HAL_GPIO_TogglePin(GPIO_Port, LEDR_Pin);
}

void readDataByIdenfierResponseCheck(uint8_t CANRxBuffer[])
{
	if (CANRxBuffer[0] == READ_DATA_BY_ID_RESPONSE_SID)
	{
		if ((CANRxBuffer[1] == ((READ_DATA_BY_ID_RECORD >> 8) & 0xff)) &&
				(CANRxBuffer[2] = (READ_DATA_BY_ID_RECORD & 0xff)))
		{
			if ((CANRxBuffer[3] == ((CAN_DIAGNOSTIC_REQUEST_ID >> 8) & 0xff)) &&
					(CANRxBuffer[4] = (CAN_DIAGNOSTIC_REQUEST_ID & 0xff)))
			{
				HAL_GPIO_TogglePin(GPIO_Port, LEDB_Pin);
			}
		}
	}
}

void securityAccessSeedRequest(CAN_HandleTypeDef *hcan)
{
	uint8_t CANTxRequest[CAN_SEED_REQUEST_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderRequest;
	uint32_t CANTxMailboxesRequest = CAN_TX_MAILBOX1;

	CANTxHeaderRequest.StdId 	= CAN_DIAGNOSTIC_REQUEST_ID;
	CANTxHeaderRequest.IDE 	= CAN_ID_STD;
	CANTxHeaderRequest.RTR 	= CAN_RTR_DATA;
	CANTxHeaderRequest.DLC 	= CAN_SEED_REQUEST_LENGTH;

	CANTxRequest[0] 	= 0x02;
	CANTxRequest[1] 	= 0x27;
	CANTxRequest[2]		= 0x01;
	CANTxRequest[3]		= UNUSED_DATA;
	CANTxRequest[4]		= UNUSED_DATA;
	CANTxRequest[5]		= UNUSED_DATA;
	CANTxRequest[6]		= UNUSED_DATA;
	CANTxRequest[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

	HAL_GPIO_TogglePin(GPIO_Port, LEDR_Pin);
}

uint8_t securityAccessSeedResponseCheck(uint8_t CANRxBuffer[])
{
	if (CANRxBuffer[0] == SECURITY_ACCESS_RESPONSE_SID)
	{
		if (CANRxBuffer[1] == SEED_LEVEL)
		{
			seed[0] = CANRxBuffer[2];
			seed[1] = CANRxBuffer[3];
			seed[2] = CANRxBuffer[4];
			seed[3] = CANRxBuffer[5];

			key[0] 	= seed[0] ^ seed[1];
			key[1] 	= seed[1] + seed[2];
			key[2] 	= seed[2] ^ seed[3];
			key[3] 	= seed[3] + seed[0];

			key[4] 	= seed[0] | seed[1];
			key[5] 	= seed[1] + seed[2];
			key[6] 	= seed[2] | seed[3];
			key[7] 	= seed[3] + seed[1];

			key[8] 	= seed[0] & seed[1];
			key[9] 	= seed[1] ^ seed[2];
			key[10] = seed[2] & seed[3];
			key[11] = seed[3] ^ seed[0];

			key[12] = seed[0] - seed[1];
			key[13] = seed[1] ^ seed[2];
			key[14] = seed[2] - seed[3];
			key[15] = seed[3] ^ seed[0];

			return 1;
		}
	}
}

void securityAccessUnlockRequest(CAN_HandleTypeDef *hcan)
{
	uint8_t CANTxRequest[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderRequest;
	uint32_t CANTxMailboxesRequest = CAN_TX_MAILBOX1;

	CANTxHeaderRequest.StdId 	= CAN_DIAGNOSTIC_REQUEST_ID;
	CANTxHeaderRequest.IDE 	= CAN_ID_STD;
	CANTxHeaderRequest.RTR 	= CAN_RTR_DATA;
	CANTxHeaderRequest.DLC 	= CAN_DATA_LENGTH;

	CANTxRequest[0] 	= 0x10;
	CANTxRequest[1] 	= 0x12;
	CANTxRequest[2] 	= 0x27;
	CANTxRequest[3]		= 0x02;
	CANTxRequest[4]		= key[0];
	CANTxRequest[5]		= key[1];
	CANTxRequest[6]		= key[2];
	CANTxRequest[7]		= key[3];

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

	HAL_GPIO_TogglePin(GPIO_Port, LEDG_Pin);
}

uint8_t flowControlCheck(uint8_t CANRxData[])
{
	uint8_t CAN_PCI = CANRxData[0] >> 4;
	uint8_t CAN_FS 	= CANRxData[0] & 0x0f;

	if ((CAN_PCI_FLOW_CONTROL != CAN_PCI) || (0x00!= CAN_FS)) return 0;

	return 1;
}

void securityRemainKeySend(CAN_HandleTypeDef *hcan)
{
	uint8_t CANTxRequest[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderRequest;
	uint32_t CANTxMailboxesRequest = CAN_TX_MAILBOX1;

	CANTxHeaderRequest.StdId 	= CAN_DIAGNOSTIC_REQUEST_ID;
	CANTxHeaderRequest.IDE 	= CAN_ID_STD;
	CANTxHeaderRequest.RTR 	= CAN_RTR_DATA;
	CANTxHeaderRequest.DLC 	= CAN_DATA_LENGTH;

	CANTxRequest[0] 	= 0x21;
	CANTxRequest[1] 	= key[4];
	CANTxRequest[2] 	= key[5];
	CANTxRequest[3]		= key[6];
	CANTxRequest[4]		= key[7];
	CANTxRequest[5]		= key[8];
	CANTxRequest[6]		= key[9];
	CANTxRequest[7]		= key[10];

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

	HAL_Delay(100);

	CANTxRequest[0] 	= 0x22;
	CANTxRequest[1] 	= key[11];
	CANTxRequest[2] 	= key[12];
	CANTxRequest[3]		= key[13];
	CANTxRequest[4]		= key[14];
	CANTxRequest[5] 	= key[15];
	CANTxRequest[6]		= UNUSED_DATA;
	CANTxRequest[7]		= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderRequest, CANTxRequest, &CANTxMailboxesRequest);

}

//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CANRxHeader, CANRxBuffer) != HAL_OK)
//	{
//		Error_Handler();
//	}
//
//	if (CANRxHeader.StdId == CAN_DIAGNOSTIC_RESPONSE_ID)
//	{
//		CANDiagnosticResponseRcvFlag = 1;
//	}
//}
