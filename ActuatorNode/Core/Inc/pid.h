#ifndef INC_PID_H_
#define INC_PID_H_

#include "main.h"

typedef struct{
	float kp; // = 2.2;
	float ki; // = 0.18;
	float kd; // = 0.003;

	float errorIntergral;
	float prevError;
	float error;

	float currT;
	float prevT;
	float deltaT;

	float outputPID;

	int timeCountPID;
} PIDInfor;

/*
 * @brief apply PID algorithm for DC motor
 * @param refSpeed: reference velocity
 * @param currSpeed: current velocity
 * @return velocity control output
 */
float PID(float refSpeed, float currSpeed);

#endif /* INC_PID_H_ */
