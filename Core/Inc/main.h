/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define MQ3_ADC_Pin GPIO_PIN_1
#define MQ3_ADC_GPIO_Port GPIOA
#define HX1_DT_Pin GPIO_PIN_0
#define HX1_DT_GPIO_Port GPIOB
#define HX1_SCK_Pin GPIO_PIN_1
#define HX1_SCK_GPIO_Port GPIOB
#define HX2_DT_Pin GPIO_PIN_2
#define HX2_DT_GPIO_Port GPIOB
#define HX2_SCK_Pin GPIO_PIN_10
#define HX2_SCK_GPIO_Port GPIOB
#define HX3_DT_Pin GPIO_PIN_12
#define HX3_DT_GPIO_Port GPIOB
#define HX3_SCK_Pin GPIO_PIN_13
#define HX3_SCK_GPIO_Port GPIOB
#define HX4_DT_Pin GPIO_PIN_14
#define HX4_DT_GPIO_Port GPIOB
#define HX4_SCK_Pin GPIO_PIN_15
#define HX4_SCK_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
