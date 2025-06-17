#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"


// xe bự xe nhỏ khác thông số
//#define XeBu

#ifdef XeBu

#define encoderResolution 580.0f // số xung thay đổi khi xoay 1 vòng
#define diameter 13.5f // đường kính bánh xe (cm

#else

#define encoderResolution 1990.0f // số xung thay đổi khi xoay 1 vòng
#define diameter 7.5f // đường kính bánh xe (cm)

#endif

#define PI 3.14159265359f
#define CIRCUMFERENCE_OF_WHEEL (PI * (diameter)) //chu vi

typedef struct{
  int16_t encodeCnt;
  int16_t preEncoderCnt;

  int16_t position;
  int16_t prePosition;

  int timeIndex;
  float numRoundPerSec;
  float speed; // cm/s

} encoderMotor;

typedef enum{
	STOP,
	START,
	FORWARD,
	REVERSE,
}motor_Status; //BTS7960

typedef struct{
	TIM_HandleTypeDef* 		Timer_Handle;
	uint16_t				Timer_Channel_L;
	uint16_t				Timer_Channel_R;

	GPIO_TypeDef*			En_L_GPIOx;
	uint16_t				En_L_GPIO_Pin;

	GPIO_TypeDef*			En_R_GPIOx;
	uint16_t				En_R_GPIO_Pin;

	struct{
		uint16_t state;
	}motorStatus;

}motorConfig;

/**
  * @brief  Call function HAL_TIM_Encoder_Start_IT
  * @retval void
  */
void startEncoder();
void updateEncoder();
void motorInit (	motorConfig* bts7960_config,
					TIM_HandleTypeDef* 		Timer_Handle,
					uint16_t				Timer_Channel_L,
					uint16_t				Timer_Channel_R,
					GPIO_TypeDef*			En_L_GPIOx,
					uint16_t				En_L_GPIO_Pin,
					GPIO_TypeDef*			En_R_GPIOx,
					uint16_t				En_R_GPIO_Pin);

uint8_t scalePWM(uint8_t PWM); // giới hạn PWM từ 0-100
float speedToPWM(float velocity);
void startMotor(motorConfig* bts7960_config);
void endMotor(motorConfig* bts7960_config);
void stopMotor(motorConfig* bts7960_config);
void goForward(motorConfig* bts7960_config, uint8_t PWM);
void goReverse(motorConfig* bts7960_config, uint8_t PWM);
#endif /* __MOTOR_H__ */
