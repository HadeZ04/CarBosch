#include "gpio.h"

const int pressTimeThreshold = 500;
const int Button_Pin[NUM_BUTTON] = {BTN1_Pin, BTN2_Pin};
int LED_Blink_Period[] = {500, 500, 500};				/* blink period in different modes */
int LED_Blink_Current_Mode = 0;							/* current mode */
int LED_Blink_Current_Period = 500;						/* current blink period */
volatile uint32_t difTime = 0;
extern uint32_t timeElapsed;
volatile uint32_t pressTime = 0;								/* button push time elapsed */
uint8_t isButtonPressed	= 0;

/**
 * @brief: LED blink effect pointer function
 */
LED_Blink LED_Blink_Mode[NUM_MODE] =
{
	LED_Blink_Mode_1,
	LED_Blink_Mode_2,
	LED_Blink_Mode_3
};
/**
 * @brief: Button short press pointer function
 */
Short_Press Short_Press_Button[NUM_BUTTON] =
{
	Short_Press_Button_1,
	Short_Press_Button_2
};
/**
 * @brief: Button long press pointer function
 */
Long_Press Long_Press_Button[NUM_BUTTON] =
{
	Long_Press_Button_1,
	Long_Press_Button_2
};

/**
 * @brief Config LED blink mode and period
 * @param Current mode
 * @retval None
 */
 void LED_Blink_Mode_Config(int p_mode)
{
	LED_Blink_Current_Mode = p_mode;
	LED_Blink_Current_Period = LED_Blink_Period[LED_Blink_Current_Mode];
}
void convert(uint32_t num, uint8_t* str)
{
	uint8_t _idx = 0;
	while (num > 0)
	{
		str[_idx] = num % 10 + 48;
		num /= 10;
		++ _idx;
	}
}
/**
 * @brief LED blink effect mode 1
 * @operation LED blink effect mode 1
 * @param None
 * @retval None
 */
void LED_Blink_Mode_1(void)
{
	if (timeElapsed >= LED_Blink_Current_Period)
	{
		Monitor_Show();
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 300) {
			if (checkButtonInMode()) return;
		}
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 600) {
			if (checkButtonInMode()) return;
		}
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 900) {
			if (checkButtonInMode()) return;
		}
		timeElapsed = 0;
	}
}

/**
 * @brief LED blink effect mode 2
 * @operation Blink green LED with duration mode 2
 * @param None
 * @retval None
 */
 void LED_Blink_Mode_2(void)
{
	if (timeElapsed >= LED_Blink_Current_Period)
	{
		Monitor_Show();
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 300) {
			if (checkButtonInMode()) return;
		}
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 600) {
			if (checkButtonInMode()) return;
		}
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
		while (timeElapsed < LED_Blink_Current_Period + 900) {
			if (checkButtonInMode()) return;
		}
		timeElapsed = 0;
	}
}

/**
 * @brief LED blink effect mode 3
 * @operation Blink blue LED with duration mode 3
 * @param None
 * @retval None
 */
 void LED_Blink_Mode_3(void)
{
	if (timeElapsed >= LED_Blink_Current_Period)
	{
		Monitor_Show();
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDR_Pin);
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDG_Pin);
		HAL_GPIO_TogglePin(ACTUATOR_GPIO_PORT, LEDB_Pin);
		timeElapsed = 0;
	}
}

/**
 * @brief short press button 1 process
 * @operation decrease period 100ms
 * @param None
 * @retval None
 */
inline void Short_Press_Button_1(void)
{
	difTime = -100;
}

/**
 * @brief short press button 2 process
 * @operation change LED blink effect mode
 * @param None
 * @retval None
 */
inline void Short_Press_Button_2(void)
{
	HAL_GPIO_WritePin(ACTUATOR_GPIO_PORT, LEDR_Pin, SET);
	HAL_GPIO_WritePin(ACTUATOR_GPIO_PORT, LEDG_Pin, SET);
	HAL_GPIO_WritePin(ACTUATOR_GPIO_PORT, LEDB_Pin, SET);
	LED_Blink_Current_Mode = (LED_Blink_Current_Mode + 1) % NUM_MODE;
}

/**
 * @brief long press button 1 process
 * @operation decrease period 100ms every 200ms button pressed
 * @param None
 * @retval None
 */
inline void Long_Press_Button_1(void)
{
	difTime = -(100 * (int)((pressTime - 500) / 200));
}

/**
 * @brief long press button 2 process
 * @operation increase period 100ms every 200ms button pressed
 * @param None
 * @retval None
 */
inline void Long_Press_Button_2(void)
{
	difTime = (100 * (int)((pressTime - 500) / 200));
}

/**
 * @brief buttons check
 * @operation check if any buttons is pressed
 * @param None
 * @retval None
 */
void Buttons_Check(void)
{
	for (int _button = 0; _button < NUM_BUTTON; ++ _button)
	{
		if (!HAL_GPIO_ReadPin(ACTUATOR_GPIO_PORT, Button_Pin[_button]))
		{
			/* software debounce */
			HAL_Delay(DEBOUNCE_DELAY);
			if (!HAL_GPIO_ReadPin(ACTUATOR_GPIO_PORT, Button_Pin[_button]))
			{
				isButtonPressed = 1;
				pressTime = 0;
				difTime = 0;

				/* wait until the button released */
				while (!HAL_GPIO_ReadPin(ACTUATOR_GPIO_PORT, Button_Pin[_button]));
				if (pressTime < pressTimeThreshold)
				{
					Short_Press_Button[_button]();
				}
				else
				{
					Long_Press_Button[_button]();
				}
				for (int _mode = 0; _mode < NUM_MODE; ++ _mode)
				{
				  LED_Blink_Period[_mode] += difTime;
				  if (LED_Blink_Period[_mode] <= 0)
				  {
					  LED_Blink_Period[_mode] = 2000;
				  }
				}
				LED_Blink_Mode_Config(LED_Blink_Current_Mode);

				isButtonPressed = 0;
			}
		}
	}
}
int checkButtonInMode()
{
	for (int _button = 0; _button < NUM_BUTTON; ++ _button)
	{
		if (!HAL_GPIO_ReadPin(ACTUATOR_GPIO_PORT, Button_Pin[_button]))
		{
			/* software debounce */
			HAL_Delay(DEBOUNCE_DELAY);
			if (!HAL_GPIO_ReadPin(ACTUATOR_GPIO_PORT, Button_Pin[_button])) return 1;
		}
	}
	return 0;
}

/**
 * @brief Transmit data through UART to show information to OLED
 */
void Monitor_Show()
{
	printf("\nPeriod: %d\n", LED_Blink_Current_Period);
	printf("\nMode: %d\n", LED_Blink_Current_Mode);
}
