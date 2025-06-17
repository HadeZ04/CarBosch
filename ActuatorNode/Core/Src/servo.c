#include "servo.h"

extern TIM_HandleTypeDef htim4;

void initServo()
{
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}

uint8_t convertToPWM(uint8_t angle) {
    if (angle <= 90) {
        return (uint8_t)((angle * (249 - 49)) / 90 + 49); // [0, 90] sang [99, 249]
    } else {
        return 249;
    }
}

uint8_t convertToAngle(uint8_t pwm) {
    if (pwm >= 99 && pwm <= 249) {
        return (uint8_t)((90 * (pwm - 49)) / (249 - 49)); // [99, 249] sang [0, 90]
    } else {
        return 0;
    }
}
void setAngle(uint8_t Angle)
{
	if (Angle < 0)
	{
		Angle = 0;
	}
	else if (Angle > 90)
	{
		Angle = 90;
	}

	htim4.Instance->CCR1 = convertToPWM(Angle);;
}

uint8_t getAngle()
{
	return convertToAngle(htim4.Instance->CCR1);
}
