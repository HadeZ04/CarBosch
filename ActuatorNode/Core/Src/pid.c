#include "pid.h"
#include "motor.h"

//float kp = 2.2;
//float ki = 0.18;
//float kd = 0.003;
//float errorIntergral = 0, prevError = 0, error;
//float outputPID;
//float currT = 0, prevT = 0, deltaT;
//extern int timeCountPID;
extern PIDInfor pidInfor;

float PID(float refSpeed, float currSpeed)
{
	if (pidInfor.timeCountPID >= 100)
	{
		pidInfor.error = refSpeed - currSpeed;
		pidInfor.currT = HAL_GetTick();
		pidInfor.deltaT = (pidInfor.currT - pidInfor.prevT) / 1000; // convert millisecond to second
		pidInfor.prevT = pidInfor.currT;

		pidInfor.errorIntergral += pidInfor.error*pidInfor.deltaT;
		pidInfor.outputPID = pidInfor.kp*pidInfor.error + pidInfor.ki*pidInfor.errorIntergral + pidInfor.kd*(pidInfor.error - pidInfor.prevError)/pidInfor.deltaT;
		pidInfor.prevError = pidInfor.error;
		pidInfor.timeCountPID = 0;
	}
	return pidInfor.outputPID;
}
