/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_SS_Pin GPIO_PIN_4
#define SPI_SS_GPIO_Port GPIOA
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOB
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOB
#define R_EN_Pin GPIO_PIN_14
#define R_EN_GPIO_Port GPIOB
#define L_EN_Pin GPIO_PIN_15
#define L_EN_GPIO_Port GPIOB
#define L_PWM_Pin GPIO_PIN_8
#define L_PWM_GPIO_Port GPIOA
#define R_PWM_Pin GPIO_PIN_11
#define R_PWM_GPIO_Port GPIOA
#define LEDR_Pin GPIO_PIN_3
#define LEDR_GPIO_Port GPIOB
#define LEDG_Pin GPIO_PIN_4
#define LEDG_GPIO_Port GPIOB
#define LEDB_Pin GPIO_PIN_5
#define LEDB_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define KP 2.0	//2.2
#define KI 0.05	//0.18
#define KD 0.01		//0.03
#define ACTUATOR_GPIO_PORT GPIOB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
