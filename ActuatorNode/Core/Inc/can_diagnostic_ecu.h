#ifndef __CAN_DIAGNOSTIC_H__
#define __CAN_DIAGNOSTIC_H__

#include "main.h"

#define CAN_DIAGNOSTIC_REQUEST_ID		0x712
#define CAN_DIAGNOSTIC_RESPONSE_ID		0x7a2

#define CAN_PCI_SINGLE_FRAME 			0x00
#define CAN_PCI_FIRST_FRAME				0x01
#define CAN_PCI_CONSECUTIVE_FRAME		0x02
#define CAN_PCI_FLOW_CONTROL			0x03
#define UNUSED_DATA						0x55

#define NEGATIVE_RESPONSE				0x7f
#define NEGATIVE_RESPONSE_LENGTH 		0x03
#define POSITIVE_RESPONSE_SID_ADD		0x40
#define READ_DATA_BY_ID_SID 			0x22
#define WRITE_DATA_BY_ID_SID			0x2e
#define SECURITY_ACCESS_SID				0x27
#define SEED_LEVEL						0x01
#define KEY_LEVEL						0x02
//#define SERCU_POSITIVE_RESPONSE_LENGTH	0x01

#define READ_INVALID_LENGTH				0x13
#define WRITE_INVALID_LENGTH			0x13
#define WRITE_DID_NOT_SUPPORT			0x31
#define SECURITY_INVALID_LENGTH			0x13
#define SECURITY_INVALID_KEY			0x35


int SAE_J1850_Calc(int data[], int len);

void CAN_Transmit(CAN_HandleTypeDef *hcan, const CAN_TxHeaderTypeDef *pHeader,
        		const uint8_t aData[], uint32_t *pTxMailbox);

uint8_t readDataByIdentifierResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[]);

uint8_t writeDataByIdentifierResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[]);

uint8_t CANNegativeResponse(CAN_HandleTypeDef *hcan, uint8_t CANPCI, uint8_t CANNegativeResponseCode);

uint8_t securityAccessSeedGenerate(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[]);

uint8_t flowControlResponse(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *CANRxHeader, uint8_t CANRxData[]);

uint8_t keyCheck();

uint8_t securityAccessKeyResponse(CAN_HandleTypeDef *hcan);

#endif /* __CAN_DIAGNOSTIC_H__ */
