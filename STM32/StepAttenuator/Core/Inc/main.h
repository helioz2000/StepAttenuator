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
#include "stm32g0xx_hal.h"

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
#define LE1_Pin GPIO_PIN_0
#define LE1_GPIO_Port GPIOA
#define LE2_Pin GPIO_PIN_1
#define LE2_GPIO_Port GPIOA
#define LE3_Pin GPIO_PIN_2
#define LE3_GPIO_Port GPIOA
#define LE4_Pin GPIO_PIN_3
#define LE4_GPIO_Port GPIOA
#define C8_Pin GPIO_PIN_4
#define C8_GPIO_Port GPIOA
#define C4_Pin GPIO_PIN_5
#define C4_GPIO_Port GPIOA
#define C2_Pin GPIO_PIN_6
#define C2_GPIO_Port GPIOA
#define C1_Pin GPIO_PIN_7
#define C1_GPIO_Port GPIOA
#define ATT40A_IN_Pin GPIO_PIN_0
#define ATT40A_IN_GPIO_Port GPIOB
#define ATT40A_OUT_Pin GPIO_PIN_1
#define ATT40A_OUT_GPIO_Port GPIOB
#define ATT20_IN_Pin GPIO_PIN_2
#define ATT20_IN_GPIO_Port GPIOB
#define C0_5_Pin GPIO_PIN_8
#define C0_5_GPIO_Port GPIOA
#define RLY_ON_Pin GPIO_PIN_11
#define RLY_ON_GPIO_Port GPIOA
#define RLY_OFF_Pin GPIO_PIN_12
#define RLY_OFF_GPIO_Port GPIOA
#define ATT20_OUT_Pin GPIO_PIN_3
#define ATT20_OUT_GPIO_Port GPIOB
#define ATT40B_IN_Pin GPIO_PIN_4
#define ATT40B_IN_GPIO_Port GPIOB
#define ATT40B_OUT_Pin GPIO_PIN_5
#define ATT40B_OUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
