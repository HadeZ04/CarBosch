#include "timer.h"

volatile uint32_t timeElapsed;
extern TIM_HandleTypeDef htim3;
extern uint8_t isButtonPressed;
extern uint32_t pressTime;
extern bool isPrint;
/**
 * @brief Timer ISR
 * @param htim Timer_HandleTypeDef pointer
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == htim3.Instance)
	{
		timeElapsed += 100;
		if (isButtonPressed) pressTime += 100;
		isPrint = true;
	}
	else
	{
		__NOP();
	}
}
