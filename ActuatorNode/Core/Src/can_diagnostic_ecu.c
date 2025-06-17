#include "can_diagnostic_ecu.h"
#include "can_node_1.h"
#include "gpio.h"

extern uint8_t CANRxBuffer[];
extern CAN_RxHeaderTypeDef CANRxHeader;
uint8_t CANDiagnosticRequestRcvFlag = 0;
uint8_t seed[4], key[16];
uint8_t rcv_key[16];

uint8_t readDataByIdentifierResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[])
{
	uint8_t CAN_PCI = CANRxData[0] >> 4;
	uint8_t CAN_SID = CANRxData[1];
	if ((CAN_PCI_SINGLE_FRAME != CAN_PCI) || (READ_DATA_BY_ID_SID != CAN_SID)) return 0;

	uint8_t CAN_DL = CANRxData[0] & 0x0f;
	uint16_t CAN_DID = (CANRxData[2] << 8) | (CANRxData[3] << 0);
	if (CAN_DL != 0x03)
	{
		CANNegativeResponse(hcan, CAN_SID, READ_INVALID_LENGTH);
		return 1;
	}
//	if (CAN_DID != 0x0123)
//	{
//	}

	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[0] = CAN_SID + POSITIVE_RESPONSE_SID_ADD;
	CANTxResponse[1] = CANRxData[2];
	CANTxResponse[2] = CANRxData[3];
	CANTxResponse[3] = CANRxHeader->StdId >> 8;
	CANTxResponse[4] = CANRxHeader->StdId & 0xff;
	CANTxResponse[5] = UNUSED_DATA;
	CANTxResponse[6] = UNUSED_DATA;
	CANTxResponse[7] = UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);

	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);

	return 1;
}

uint8_t writeDataByIdentifierResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[])
{
	uint8_t CAN_PCI = CANRxData[0] >> 4;
	uint8_t CAN_SID = CANRxData[1];
	if ((CAN_PCI_SINGLE_FRAME != CAN_PCI) || (WRITE_DATA_BY_ID_SID != CAN_SID)) return 0;

	uint8_t CAN_DL = CANRxData[0] & 0x0f;
	uint16_t CAN_DID = (CANRxData[2] << 8) | (CANRxData[3] << 0);
	if (CAN_DL < 0x03)
	{
		CANNegativeResponse(hcan, CAN_SID, WRITE_INVALID_LENGTH);
		return 1;
	}
	if (CAN_DID != 0x0123)
	{
		CANNegativeResponse(hcan, CAN_SID, WRITE_DID_NOT_SUPPORT);
		return 1;
	}

	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[0] = CAN_SID + POSITIVE_RESPONSE_SID_ADD;
	CANTxResponse[1] = CANRxData[2];
	CANTxResponse[2] = CANRxData[3];
	CANTxResponse[3] = UNUSED_DATA;
	CANTxResponse[4] = UNUSED_DATA;
	CANTxResponse[5] = UNUSED_DATA;
	CANTxResponse[6] = UNUSED_DATA;
	CANTxResponse[7] = UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);

	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);

	return 1;
}

uint8_t securityAccessSeedGenerate(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[])
{
	uint8_t CAN_PCI = CANRxData[0] >> 4;
	uint8_t CAN_SID = CANRxData[1];

	if ((CAN_PCI_SINGLE_FRAME != CAN_PCI) || (SECURITY_ACCESS_SID != CAN_SID)) return 0;

	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[0] = CAN_SID + POSITIVE_RESPONSE_SID_ADD;
	CANTxResponse[1] = SEED_LEVEL;
	srand(time(NULL));
	/* Generate SEED */
	CANTxResponse[2] = rand() % 256;
	CANTxResponse[3] = rand() % 256;
	CANTxResponse[4] = rand() % 256;
	CANTxResponse[5] = rand() % 256;
	CANTxResponse[6] = UNUSED_DATA;
	CANTxResponse[7] = UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);

	/* Save SEED and calculate KEY*/
	seed[0] = CANTxResponse[2];
	seed[1] = CANTxResponse[3];
	seed[2] = CANTxResponse[4];
	seed[3] = CANTxResponse[5];

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

uint8_t flowControlResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[])
{
	uint8_t CAN_PCI = CANRxData[0] >> 4;
	uint8_t CAN_SID = CANRxData[2];

	if ((CAN_PCI_FIRST_FRAME != CAN_PCI) || (SECURITY_ACCESS_SID != CAN_SID)) return 0;

	rcv_key[0] = CANRxData[4];
	rcv_key[1] = CANRxData[5];
	rcv_key[2] = CANRxData[6];
	rcv_key[3] = CANRxData[7];

	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[0] 	= 0x30;
	CANTxResponse[1] 	= 0x08;
	CANTxResponse[2] 	= 0x25;
	CANTxResponse[3] 	= UNUSED_DATA;
	CANTxResponse[4] 	= UNUSED_DATA;
	CANTxResponse[5] 	= UNUSED_DATA;
	CANTxResponse[6] 	= UNUSED_DATA;
	CANTxResponse[7] 	= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);
}

uint8_t keyCheck()
{
	uint8_t CAN_PCI = CANRxBuffer[0] >> 4;
	uint8_t CAN_SID = CANRxBuffer[1];

	if (CAN_PCI_CONSECUTIVE_FRAME != CAN_PCI) return 0;

	rcv_key[4] = CANRxBuffer[1];
	rcv_key[5] = CANRxBuffer[2];
	rcv_key[6] = CANRxBuffer[3];
	rcv_key[7] = CANRxBuffer[4];
	rcv_key[8] = CANRxBuffer[5];
	rcv_key[9] = CANRxBuffer[6];
	rcv_key[10] = CANRxBuffer[7];

	while (CANDiagnosticRequestRcvFlag != 1);
	CANDiagnosticRequestRcvFlag = 0;

	CAN_PCI = CANRxBuffer[0] >> 4;
	CAN_SID = CANRxBuffer[1];

	if (CAN_PCI_CONSECUTIVE_FRAME != CAN_PCI) return 0;

	rcv_key[11] = CANRxBuffer[1];
	rcv_key[12] = CANRxBuffer[2];
	rcv_key[13] = CANRxBuffer[3];
	rcv_key[14] = CANRxBuffer[4];
	rcv_key[15] = CANRxBuffer[5];

	for (int _key = 0; _key < 16; ++ _key)
	{
		if (rcv_key[_key] != key[_key]) return 2;
	}

	return 1;
}

uint8_t securityAccessKeyResponse(CAN_HandleTypeDef *hcan)
{
	uint8_t key_checker =	keyCheck();

	if (key_checker == 0)
	{
		return 0;
	}
	if (key_checker == 2)
	{
		CANNegativeResponse(hcan, SECURITY_ACCESS_SID, SECURITY_INVALID_KEY);

//		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
		return 1;
	}

	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

//		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);

	CANTxResponse[0] = SECURITY_ACCESS_SID + POSITIVE_RESPONSE_SID_ADD;
	CANTxResponse[1] = KEY_LEVEL;
	CANTxResponse[2] = UNUSED_DATA;
	CANTxResponse[3] = UNUSED_DATA;
	CANTxResponse[4] = UNUSED_DATA;
	CANTxResponse[5] = UNUSED_DATA;
	CANTxResponse[6] = UNUSED_DATA;
	CANTxResponse[7] = UNUSED_DATA;

	HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);

	return 1;
}

uint8_t CANNegativeResponse(CAN_HandleTypeDef *hcan, uint8_t CANSID, uint8_t CANNegativeResponseCode)
{
	uint8_t CANTxResponse[CAN_DATA_LENGTH];
	CAN_TxHeaderTypeDef CANTxHeaderResponse;
	uint32_t CANTxMailboxesResponse = CAN_TX_MAILBOX1;

	CANTxHeaderResponse.StdId 	= CAN_DIAGNOSTIC_RESPONSE_ID;
	CANTxHeaderResponse.IDE 	= CAN_ID_STD;
	CANTxHeaderResponse.RTR 	= CAN_RTR_DATA;
	CANTxHeaderResponse.DLC 	= CAN_DATA_LENGTH;

	CANTxResponse[0] 	= NEGATIVE_RESPONSE;
	CANTxResponse[1] 	= CANSID;
	CANTxResponse[2]	= CANNegativeResponseCode;
	CANTxResponse[3] 	= UNUSED_DATA;
	CANTxResponse[4] 	= UNUSED_DATA;
	CANTxResponse[5] 	= UNUSED_DATA;
	CANTxResponse[6] 	= UNUSED_DATA;
	CANTxResponse[7] 	= UNUSED_DATA;

	CAN_Transmit(hcan, &CANTxHeaderResponse, CANTxResponse, &CANTxMailboxesResponse);
}

//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CANRxHeader, CANRxBuffer) != HAL_OK)
//	{
//		Error_Handler();
//	}
//
//	if (CANRxHeader.StdId == CAN_DIAGNOSTIC_REQUEST_ID)
//	{
//		CANDiagnosticRequestRcvFlag = 1;
//	}
//}
