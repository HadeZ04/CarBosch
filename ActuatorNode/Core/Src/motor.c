#include "motor.h"

extern TIM_HandleTypeDef htim2;
extern encoderMotor encoderInfo;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
//	if (htim->Instance == htim2.Instance)
//		{
//		encoderInfo.encodeCnt = (int16_t)__HAL_TIM_GET_COUNTER(htim);
////		encoderInfo.position = encoderInfo.encodeCnt/encoderResolution;
//		}
//		else
//		{
//			__NOP();
//		}
//	counter = __HAL_TIM_GET_COUNTER(htim);
//
//	count = (int16_t)counter;
//
//	position = count/4;
}

void startEncoder()
{
//	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

void updateEncoder()
{
	if (encoderInfo.timeIndex >= 100) //100 ms
		{
			encoderInfo.encodeCnt = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
			encoderInfo.numRoundPerSec = ((-(encoderInfo.encodeCnt - encoderInfo.preEncoderCnt)*10)/encoderResolution);  // speed in cm/sec
#ifdef XeBu
			encoderInfo.speed = (float)encoderInfo.numRoundPerSec * CIRCUMFERENCE_OF_WHEEL;
#else
			encoderInfo.speed = - ((float)encoderInfo.numRoundPerSec * CIRCUMFERENCE_OF_WHEEL / 2.8f);
#endif
			encoderInfo.preEncoderCnt = encoderInfo.encodeCnt;
			encoderInfo.timeIndex = 0;
		}
}

void motorInit (		motorConfig* bts7960_config,
						TIM_HandleTypeDef* 		Timer_Handle,
						uint16_t				Timer_Channel_L,
						uint16_t				Timer_Channel_R,
						GPIO_TypeDef*			En_L_GPIOx,
						uint16_t				En_L_GPIO_Pin,
						GPIO_TypeDef*			En_R_GPIOx,
						uint16_t				En_R_GPIO_Pin)
{
	bts7960_config->Timer_Handle	= Timer_Handle;
	bts7960_config->Timer_Channel_L  	= Timer_Channel_L;
	bts7960_config->Timer_Channel_R  	= Timer_Channel_R;

	bts7960_config->En_L_GPIOx		= En_L_GPIOx;
	bts7960_config->En_L_GPIO_Pin	= En_L_GPIO_Pin;

	bts7960_config->En_R_GPIOx		= En_R_GPIOx;
	bts7960_config->En_R_GPIO_Pin	= En_R_GPIO_Pin;
}

uint8_t scalePWM(uint8_t PWM)
{
    return (PWM > 100) ? 100 : (PWM < 0) ? 0 : PWM;
}

float speedToPWM(float velocity)
{
//	return (0.0014f * velocity * velocity  + 0.1236f * velocity + 16.496f);//xe bự
	return (0.1871f * velocity * velocity  + 0.451f* velocity + 42.974f);//xe nhỏ
	// phương trình dựa trên PWM và vận tốc đo được
	// từ encoder khi xe chịu tải
}

void startMotor(motorConfig* bts7960_config)
{
	bts7960_config->motorStatus.state = START;
	HAL_GPIO_WritePin(bts7960_config->En_L_GPIOx, bts7960_config->En_L_GPIO_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(bts7960_config->En_R_GPIOx, bts7960_config->En_R_GPIO_Pin, GPIO_PIN_SET);
	HAL_TIM_PWM_Start_IT(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_L);
	HAL_TIM_PWM_Start_IT(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_R);
}

void endMotor(motorConfig* bts7960_config)
{
	bts7960_config->motorStatus.state = STOP;
	HAL_GPIO_WritePin(bts7960_config->En_L_GPIOx, bts7960_config->En_L_GPIO_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bts7960_config->En_R_GPIOx, bts7960_config->En_R_GPIO_Pin, GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop_IT(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_L);
	HAL_TIM_PWM_Stop_IT(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_R);
}

void stopMotor(motorConfig* bts7960_config)
{
	bts7960_config->motorStatus.state = STOP;
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_L, 1);
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_R, 1);
}

void goForward(motorConfig* bts7960_config, uint8_t PWM )
{
	bts7960_config->motorStatus.state = FORWARD;
	PWM = scalePWM(PWM);
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_L, 0);
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_R, PWM);
}

void goReverse(motorConfig* bts7960_config, uint8_t PWM)
{
	PWM = scalePWM(PWM);
	bts7960_config->motorStatus.state = REVERSE;
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_L, PWM);
	__HAL_TIM_SET_COMPARE(bts7960_config->Timer_Handle, bts7960_config->Timer_Channel_R, 0);
}
