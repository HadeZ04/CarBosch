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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LedOnBoard_Pin GPIO_PIN_13
#define LedOnBoard_GPIO_Port GPIOC
#define GPIO1_MCU1_0_Pin GPIO_PIN_14
#define GPIO1_MCU1_0_GPIO_Port GPIOC
#define XSHUT_MCU1_0_Pin GPIO_PIN_15
#define XSHUT_MCU1_0_GPIO_Port GPIOC
#define ADC_TEMPT_Pin GPIO_PIN_4
#define ADC_TEMPT_GPIO_Port GPIOA
#define BTN2_Pin GPIO_PIN_0
#define BTN2_GPIO_Port GPIOB
#define BTN1_Pin GPIO_PIN_1
#define BTN1_GPIO_Port GPIOB
#define LEDB_Pin GPIO_PIN_10
#define LEDB_GPIO_Port GPIOB
#define LEDG_Pin GPIO_PIN_11
#define LEDG_GPIO_Port GPIOB
#define LEDR_Pin GPIO_PIN_12
#define LEDR_GPIO_Port GPIOB
#define MPU_INT_Pin GPIO_PIN_13
#define MPU_INT_GPIO_Port GPIOB
#define MPU_BOOT_Pin GPIO_PIN_14
#define MPU_BOOT_GPIO_Port GPIOB
#define MPU_RST_Pin GPIO_PIN_15
#define MPU_RST_GPIO_Port GPIOB
#define SPI_CS_Pin GPIO_PIN_8
#define SPI_CS_GPIO_Port GPIOA
#define XSHUT_MCU1_1_Pin GPIO_PIN_8
#define XSHUT_MCU1_1_GPIO_Port GPIOB
#define GPIO1_MCU1_1_Pin GPIO_PIN_9
#define GPIO1_MCU1_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define GPIO_Port GPIOB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
