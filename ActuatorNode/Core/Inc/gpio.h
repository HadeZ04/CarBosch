#ifndef __GPIO_H__
#define __GPIO_H__

#include "main.h"

#define ACTUATOR_GPIO_PORT 		GPIOB

#define NUM_MODE 				3								/* number of mode */
#define NUM_BUTTON				2								/* number of user button */
#define DEBOUNCE_DELAY			50U								/* software debounce delay */

typedef void (*LED_Blink)(void);
typedef void (*Short_Press)(void);
typedef void (*Long_Press)(void);

extern LED_Blink LED_Blink_Mode[];
extern Short_Press Short_Press_Button[];
extern Long_Press Long_Press_Button[];

void LED_Blink_Mode_Config(int p_mode);
void LED_Blink_Mode_1(void);
void LED_Blink_Mode_2(void);
void LED_Blink_Mode_3(void);
void Short_Press_Button_1(void);
void Short_Press_Button_2(void);
void Long_Press_Button_1(void);
void Long_Press_Button_2(void);
void Buttons_Check(void);
void Monitor_Show();
int checkButtonInMode();

#endif /* __GPIO_H__ */
