/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart.h"
#include "can_project_sensor.h"
#include "VL53L0X.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define VL53L0X
// #define VL6180X_SECOND
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* CAN Tx variables */
const uint8_t first_value = 0x01;
const uint8_t second_value = 0x02;
CAN_TxHeaderTypeDef CANTxHeader;
uint8_t CANTxBuffer[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t CANTxMailboxes = CAN_TX_MAILBOX0;
/* CAN Rx variables */
extern uint8_t CANRxBuffer[];
extern uint8_t CANDiagnosticResponseRcvFlag;
extern CAN_RxHeaderTypeDef CANRxHeader;
CANSensorData CANTxData;
uint8_t last_seq = 0, cur_seq;
uint16_t left_dis = 0, right_dis = 0;
const uint16_t forward_threshold = 1100;
const uint16_t turn_threshold = 600;
const uint16_t diff_LeftRight = 100;
extern uint8_t CANDataRcvFlag;
extern uint32_t timeElapsed;
uint8_t dif = 5;
uint8_t cur_speed = CAN_SPEED_MIN;
uint8_t urgent_out = 0;
uint8_t step = 0;
uint8_t temp = 0;
CONTROL_DIRECTION_2 direction2;
VL53L0X_t lox1;
VL53L0X_t lox2;

void init2Sensor(VL53L0X_t *lox1, VL53L0X_t *lox2, I2C_HandleTypeDef *i2cx);

int timeCountTest = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim3);
  if (HAL_CAN_Start(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO1_MSG_PENDING);

  printf("Sensor\n");
  // Sensor
  init2Sensor(&lox1, &lox2, &hi2c1);

  // End sensor

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    left_dis = readRangeSingleMillimeters(&lox2);
    right_dis = readRangeSingleMillimeters(&lox1);
    CANTxData.sequence++;
    CANTxData.priority = CONTROL_PRIOR_NORMAL;
    if (CANDataRcvFlag == 1) // timeout
    {
      CANDataRcvFlag = 0;
      cur_seq = CANRxBuffer[CAN_DATA_SEQ_IDX] << 8 |
                CANRxBuffer[CAN_DATA_SEQ_IDX + 1];
      cur_speed = CANRxBuffer[CAN_ACTUATOR_DATA_SPEED_IDX];
      if (cur_seq == last_seq)
      {
        // do nothing
      }
      else
      {
        HAL_GPIO_TogglePin(GPIO_Port, LEDG_Pin);
        HAL_GPIO_TogglePin(GPIO_Port, LEDB_Pin);
        CANResponseCheck();
        if (urgent_out == 1)
		{
        	if (left_dis <= forward_threshold || right_dis <= forward_threshold)
        	{
        		CANTxData.priority = CONTROL_PRIOR_NORMAL;
				CANTxData.speed = CAN_SPEED_NORMAL;
				CANTxData.direction1 = CONTROL_DIR_BACKWARD;
				CANTxData.direction2 = direction2;
        	}
        	else
        	{
        		urgent_out = 0;
        	}
        	temp = 0;
		}
        else if (left_dis < turn_threshold && right_dis < turn_threshold)
        {
        	if (left_dis < right_dis)
        	{
        		direction2 = CONTROL_DIR_LEFT;
        	}
        	else
        	{
        		direction2 = CONTROL_DIR_RIGHT;
        	}
        	if (cur_speed > CAN_SPEED_MIN)
        	{
				CANTxData.priority = CONTROL_PRIOR_URGENT;
				CANTxData.speed = CAN_SPEED_NORMAL;
				CANTxData.direction1 = CONTROL_DIR_BACKWARD;
        	}
        	else
        	{
        		urgent_out = 1;
        	}
        	temp = 1;
        }
        else if (left_dis >= forward_threshold && right_dis >= forward_threshold) // go Straight
        {
          CANTxData.priority = CONTROL_PRIOR_NORMAL;
          CANTxData.speed = CAN_SPEED_NORMAL;
          CANTxData.direction1 = CONTROL_DIR_FORWARD;
		  CANTxData.direction2 = CONTROL_DIR_STRAIGHT;
      	temp = 2;
        }
        else if (((left_dis - right_dis) > diff_LeftRight) && (right_dis < 900))
        {
          CANTxData.priority = CONTROL_PRIOR_NORMAL;
          CANTxData.speed = CAN_SPEED_NORMAL;
          CANTxData.direction1 = CONTROL_DIR_FORWARD;
		  CANTxData.direction2 = CONTROL_DIR_LEFT;
      	temp = 3;
        }
        else if (((right_dis - left_dis) > diff_LeftRight) && (left_dis < 900))
        {
          CANTxData.priority = CONTROL_PRIOR_NORMAL;
          CANTxData.speed = CAN_SPEED_NORMAL;
          CANTxData.direction1 = CONTROL_DIR_FORWARD;
          CANTxData.direction2 = CONTROL_DIR_RIGHT;
      	temp = 4;
        }
      }
    }
    if (timeCountTest > 200)
    {
      		  printf("\n\nl %d",left_dis);
      		  printf("\nr %d",right_dis);
      		  printf("\nkey %d",temp);
      //		  printf("\nd %d",CANTxData.direction);
    }
    CANSensorTransmit(&hcan, &CANTxData);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief CAN Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_FilterTypeDef canfilterconfig;

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 12; // which filter bank to use from the assigned ones
  canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
  canfilterconfig.FilterIdHigh = CAN_PROJECT_ACTUATOR_STDID << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = CAN_PROJECT_ACTUATOR_STDID << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilterconfig.SlaveStartFilterBank = 13; // how many filters to assign to the CAN1 (master can)

  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
  /* USER CODE END CAN_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 35999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 199;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LedOnBoard_Pin | XSHUT_MCU1_0_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LEDB_Pin | LEDG_Pin | LEDR_Pin | GPIO_PIN_3 | XSHUT_MCU1_1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MPU_BOOT_Pin | MPU_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LedOnBoard_Pin XSHUT_MCU1_0_Pin */
  GPIO_InitStruct.Pin = LedOnBoard_Pin | XSHUT_MCU1_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO1_MCU1_0_Pin */
  GPIO_InitStruct.Pin = GPIO1_MCU1_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO1_MCU1_0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN2_Pin BTN1_Pin */
  GPIO_InitStruct.Pin = BTN2_Pin | BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LEDB_Pin LEDG_Pin LEDR_Pin MPU_BOOT_Pin
                           MPU_RST_Pin PB3 XSHUT_MCU1_1_Pin */
  GPIO_InitStruct.Pin = LEDB_Pin | LEDG_Pin | LEDR_Pin | MPU_BOOT_Pin | MPU_RST_Pin | GPIO_PIN_3 | XSHUT_MCU1_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : MPU_INT_Pin GPIO1_MCU1_1_Pin */
  GPIO_InitStruct.Pin = MPU_INT_Pin | GPIO1_MCU1_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void init2Sensor(VL53L0X_t *lox1, VL53L0X_t *lox2, I2C_HandleTypeDef *i2cx)
{

  // Reset sensor
  lox1->Address = ADDRESS_DEFAULT;
  lox2->Address = ADDRESS_DEFAULT;
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
  HAL_Delay(20);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
  HAL_Delay(20);

  // init first sensor
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
  HAL_Delay(20);
  while (!initVL53L0X(1, lox1, i2cx))
  {
    HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
    printf("Failed 1!\n");
  }
  setAddress(0x50, lox1);

  while (!initVL53L0X(1, lox1, i2cx))
  {
    HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
    printf("Failed 2!\n");
  }

  // init second sensor
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
  HAL_Delay(20);

  while (!initVL53L0X(1, lox2, i2cx))
  {
    HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
    printf("Failed 3!\n");
  }
  setAddress(0x42, lox2);

  while (!initVL53L0X(1, lox2, i2cx))
  {
    HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
    printf("Failed 4!\n");
  }

  // turn on 2 sensor
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
  HAL_Delay(20);
  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 1); // off led
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LEDG_GPIO_Port, LEDG_Pin);
    HAL_Delay(200);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
