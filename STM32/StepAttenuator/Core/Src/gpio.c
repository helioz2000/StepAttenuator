/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LE1_Pin|LE2_Pin|LE3_Pin|LE4_Pin
                          |C8_Pin|C4_Pin|C2_Pin|C1_Pin
                          |C0_5_Pin|RLY_ON_Pin|RLY_OFF_Pin|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : LE1_Pin LE2_Pin LE3_Pin LE4_Pin
                           C8_Pin C4_Pin C2_Pin C1_Pin
                           C0_5_Pin RLY_ON_Pin RLY_OFF_Pin PA15 */
  GPIO_InitStruct.Pin = LE1_Pin|LE2_Pin|LE3_Pin|LE4_Pin
                          |C8_Pin|C4_Pin|C2_Pin|C1_Pin
                          |C0_5_Pin|RLY_ON_Pin|RLY_OFF_Pin|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ATT40A_IN_Pin ATT40A_OUT_Pin ATT20_IN_Pin ATT20_OUT_Pin
                           ATT40B_IN_Pin ATT40B_OUT_Pin */
  GPIO_InitStruct.Pin = ATT40A_IN_Pin|ATT40A_OUT_Pin|ATT20_IN_Pin|ATT20_OUT_Pin
                          |ATT40B_IN_Pin|ATT40B_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
