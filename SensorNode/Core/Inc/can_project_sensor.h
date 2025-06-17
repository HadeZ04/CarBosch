#ifndef __CAN_PROJECT_H__
#define __CAN_PROJECT_H__

#include "can_node_2.h"

#define CAN_PROJECT_ACTUATOR_STDID 	0x712
#define CAN_PROJECT_SENSOR_STDID 	0x7a2

/* format: 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX
 * 		   SEQ		 PRI  SPD  DIR1 DIR2 */
#define CAN_DATA_SEQ_IDX			0x00
#define CAN_SENSOR_DATA_PRI_IDX		0x02
#define CAN_SENSOR_DATA_SPEED_IDX	0x03
#define CAN_SENSOR_DATA_DIRECT1_IDX	0x04
#define CAN_SENSOR_DATA_DIRECT2_IDX	0x05

#define CAN_SPEED_MIN 				0x07
#define CAN_SPEED_NORMAL			0x0d //0x28
#define CAN_SPEED_MAX 				0x3c
#define UNUSED_DATA					0x55

/* format: 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX
 * 		   SEQ		 SPD */
#define CAN_ACTUATOR_DATA_SPEED_IDX	0x02

typedef enum
{
	CONTROL_PRIOR_URGENT = 0,
//	CONTROL_PRIOR
	CONTROL_PRIOR_NORMAL = 2
} CONTROL_PRIORITY;

typedef enum
{
	CONTROL_DIR_FORWARD = 0,
	CONTROL_DIR_BACKWARD
} CONTROL_DIRECTION_1;

typedef enum
{
	CONTROL_DIR_LEFT = 0,
	CONTROL_DIR_RIGHT,
	CONTROL_DIR_STRAIGHT
} CONTROL_DIRECTION_2;

typedef struct
{
	uint16_t sequence;
	CONTROL_PRIORITY priority;
	uint8_t speed;
	CONTROL_DIRECTION_1 direction1;
	CONTROL_DIRECTION_2 direction2;
} CANSensorData;

void CANSensorTransmit(CAN_HandleTypeDef *hcan, CANSensorData* controlData);
void CANResponseCheck();

#endif /* __CAN_PROJECT_H__ */
