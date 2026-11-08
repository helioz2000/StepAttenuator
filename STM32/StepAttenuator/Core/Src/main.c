/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/*
 * SERVICE MODE
 * ============
 * This operation mode allows testing of each individual attenuator pad
 *
 * Wiring to J5 (MI2955 header):
 * +5V pin 5/7/9
 * GND pin 2
 * 6 buttons connected between GND and
 * pin 1 - 40dB_A_IN   Button 1
 * pin 3 - 40dB_A_OUT  Button 2
 * pin 4 - 20dB_IN     Button 3
 * pin 6 - 20dB_OUT    Button 4
 * pin 8 - 40dB_B_IN   Button 5
 * pin 10 -40dB_B_OUT  Button 6
 *
 * Wiring to STM32 MCU for text output to a 3.3V to USB Converter
 * pin 19 - USART1 TX
 * pin 21 - USART1 RX
 * !! Note: if using a 5V TTL converter you will need V-Divider on pin 21 drop the voltage to 3.3V
 * !! Divider with 1K8/3K3 resistors will work
 *
 * Service Mode Operation
 * ----------------------
 * 1) Hold in B1 and B2 and one of the other buttons (B3=U2, B4=U3, etc) to enable
 *    service mode for one attenuator pad.
 * 2) Apply power to the board
 * 3) Wait for text to appear on the serial interface
 * 4) Release all buttons
 * Service mode is now enabled with the following functions
 * B1 - Increment attenuation value by 0.5dB
 * B2 - Decrement attenuation value by 0.5dB
 * B3 - Toggle bypass relay
 * To end service mode power cycle the board
 *
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PE4312_TIME 1	// [ms] delay between signal changes on the pads
#define SERVICE_SIGNAL_DEBOUNCE 200 // [ms] debounce time for Service Mode signals
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Volatile declaration for access in ISR and main program
volatile bool Att40A_state = false;
volatile bool Att40B_state = false;
volatile bool Att20_state = false;
volatile bool AttStateChanged = false;
volatile bool ServiceMode = false;				// Service mode for manual control
volatile bool ServiceModeInc = false;			// Service mode value increment
volatile bool ServiceModeDec = false;			// Service mode value decrement
volatile bool ServiceModeBypass = false;		// Service mode bypass enable
volatile bool ServiceModeBypassChanged = false;
bool ServicePE1 = false;
bool ServicePE2 = false;
bool ServicePE3 = false;
bool ServicePE4 = false;
int ServiceValue = 0;
volatile uint32_t ServiceLast_exti_time = 0;
/*
 * This matrix represents the attenuation settings for each of the 4 pad
 * in the attenuation circuit.
 * Power dissipation %age per pad is shown in line comments.
 * The maximum power dissipation per pad is +24dBm
 * At 100dB total attenuation the max input power to the
 * attenuator is +26.74dBm (472mW)
 *
 * NOTE: each attenuator pad has a typical insertion loss of 1.3dB (<1gHz) which
 * is taken into account in the below configuration. The attenuation
 * figures in brackets in the line comment represent attenuation
 * including the pad insertion losses
*/
const int ATTENUATOR_MATRIX[6][4] = {
    {  0,   0,   0,   0},  // 0dB: bypass via relay (total insertion loss of pads is 5.2dB)
    {  0,   5,  15, 130},  // 20.2dB  25.9%, 25.1%, 23.3%, 24.7% (1.3dB,1.8dB,2.8dB,14.3dB)
    {  0,  10,  30, 310},  // 40.2dB: 25.9%, 3.5%, 27.4%, 16.2% (1.3dB,2.3dB,4.3dB,32.3dB)
    {  5,  15, 220, 310},  // 60.2dB: 33.9%, 31.4%, 34.5%, 0.2% (1.8dB,2.8dB,23.3dB,32.3dB
    { 15, 105, 310, 310},  // 80.2dB: 47.5%, 49.7%, 2.7%, 0% (2.8dB,11.5dB,32.3dB,32.3dB)
    { 20, 310, 310, 310}   // 100.2dB: 53.2%, 46.7%, 0%, 0% (3.3dB,32.3dB,32.3dB,32.3dB)
/*	figures without pad insertion loss
    { 10,  15,  30, 145},  // 20dB  20.6%, 23.2%, 28%, 27.2%
    { 15,  25,  50, 310},  // 40dB: 29.2%, 31%, 27.2%, 12.6%
    { 15,  30, 245, 310},  // 60dB: 29.2%, 35.3%, 35.3%, 0.2%
    { 30, 150, 310, 310},  // 80dB: 49.9%, 48.5%, 1.6%, 0%
    { 70, 310, 310, 310}   // 100dB: 80%, 20%, 0%, 0%
*/
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Redirects standard output (printf) to the USART1 peripheral.
  * @param  file: Not used in this implementation.
  * @param  ptr: Pointer to the string buffer to transmit.
  * @param  len: Length of the string data to transmit.
  * @retval Number of characters transmitted.
  */
int _write(int file, char *ptr, int len)
{
  // External reference to the UART handle generated by CubeMX
  extern UART_HandleTypeDef huart1;

  // Transmit data over USART1 with a safe 100ms timeout window
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);

  return len;
}

/**
 * @brief  Print service mode selected attenuator
 */
void print_service_mode_PE(void)
{
	if (ServicePE1) {
		printf("ATT1(U2)");
	} else if (ServicePE2){
		printf("ATT2(U3)");
	} else if (ServicePE3){
		printf("ATT3(U4)");
	} else if (ServicePE4){
		printf("ATT4(U5)");
	}
}

/**
 * @brief  Handle button interrupt in service mode
 */
void HAL_GPIO_EXTI_ServiceMode(uint16_t GPIO_Pin){
  uint32_t current_time = HAL_GetTick();
  // debounce service mode signals by ignoring signals for debounce time
  if ((current_time - ServiceLast_exti_time) < SERVICE_SIGNAL_DEBOUNCE) return;

  if (GPIO_Pin == ATT40A_IN_Pin)
  {
	ServiceModeInc = true;
  } else if (GPIO_Pin == ATT40A_OUT_Pin)
  {
	ServiceModeDec = true;
  } else if (GPIO_Pin == ATT20_IN_Pin)
  {
	ServiceModeBypass = !ServiceModeBypass;
	ServiceModeBypassChanged = true;
  }
  ServiceLast_exti_time = HAL_GetTick();
}

/**
 * @brief  Callback when attenuator setting inputs change
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  if (!ServiceMode) {
	if (GPIO_Pin == ATT40A_IN_Pin)
	{
		if (Att40A_state != true) AttStateChanged = true;
	    Att40A_state = true;
	} else if (GPIO_Pin == ATT40A_OUT_Pin)
	{
	    if (Att40A_state != false) AttStateChanged = true;
	    Att40A_state = false;
	} else if (GPIO_Pin == ATT20_IN_Pin)
	{
		if (Att20_state != true) AttStateChanged = true;
	    Att20_state = true;
	} else if (GPIO_Pin == ATT20_OUT_Pin)
	{
		if (Att20_state != false) AttStateChanged = true;
		Att20_state = false;
	} else if (GPIO_Pin == ATT40B_IN_Pin)
	{
		if (Att40B_state != true) AttStateChanged = true;
		Att40B_state = true;
	} else if (GPIO_Pin == ATT40B_OUT_Pin)
	{
		if (Att40B_state != false) AttStateChanged = true;
		Att40B_state = false;
	}
  } else {	// Service mode active
	  HAL_GPIO_EXTI_ServiceMode(GPIO_Pin);
  }
}

/**
 * @brief	Put MCU into stop mode until wake up by External Interrupt
 * @note	Upon wake up the MCU executes the ISR before code resumption
 */
void MCU_stop()
{
	/* Clear any pending flags and prepare for sleep */
	// It is good practice to wait for any signals to be fully released
	// so you don't instantly re-wake up from a lingering touch.
	HAL_Delay(500);

	/* ENTER DEEP SLEEP (STOP MODE) */
	/* This command drops current consumption to micro-amps.
       Execution pauses here until PB0-PB5 triggers an interrupt. */
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	/* When execution resumes after MCU stop, the interrupt service routine (ISR)
	 * will be executed before resuming execution at this point */

	/* IMPORTANT: RESTORE SYSTEM CLOCKS IMMEDIATELY AFTER WAKING UP */
	/* When waking from STOP mode, the STM32G0 always defaults back to
	   its internal HSI clock (16 MHz) and disables your high-speed PLL.
	   You must call this function to safely rebuild your clock tree back to
	   your configured speed (e.g., 64 MHz). */
	SystemClock_Config();

    /* Handle any post-wakeup operations */
	// Code here executes immediately after the clock is restored

}

/**
  * @brief  Set attenuator bypass relay
  * @param attValue: attenuator setpoint in dB
  */
void attenuator_bypass_relay(int attValue){
	//Switch relay into ON/OFF state
	if (attValue == 0) {
		HAL_GPIO_WritePin(RLY_ON_GPIO_Port, RLY_ON_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(RLY_OFF_GPIO_Port, RLY_OFF_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(RLY_ON_GPIO_Port, RLY_ON_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(RLY_OFF_GPIO_Port, RLY_OFF_Pin, GPIO_PIN_SET);
	}
	// Time for relay to resume new position
	HAL_Delay(200);
	// The relay will stay in position without power
	HAL_GPIO_WritePin(RLY_ON_GPIO_Port, RLY_ON_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RLY_OFF_GPIO_Port, RLY_OFF_Pin, GPIO_PIN_RESET);
}

/**
  * @brief  Set attenuator value on parallel control lines
  * @param attValue: pad set point in dB x 10
  * @retval True on success
  */
// Pad attenuation range is 0-31.5dB in 0.5dB steps
bool attenuator_set_value(int attValue)
{
	// range check
	if ((attValue > 315) || (attValue < 0)) { return false; }
	int att = attValue;
	if (att >= 160) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C16_Pin, GPIO_PIN_SET);
		att -= 160;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C16_Pin, GPIO_PIN_RESET);
	}
	if (att >= 80) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C8_Pin, GPIO_PIN_SET);
		att -= 80;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C8_Pin, GPIO_PIN_RESET);
	}
	if (att >= 40) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C4_Pin, GPIO_PIN_SET);
		att -= 40;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C4_Pin, GPIO_PIN_RESET);
	}
	if (att >= 20) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C2_Pin, GPIO_PIN_SET);
		att -= 20;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C2_Pin, GPIO_PIN_RESET);
	}
	if (att >= 10) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C1_Pin, GPIO_PIN_SET);
		att -= 10;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C1_Pin, GPIO_PIN_RESET);
	}
	if (att >= 5) {
		HAL_GPIO_WritePin(C16_GPIO_Port, C05_Pin, GPIO_PIN_SET);
		att -= 5;
	} else {
		HAL_GPIO_WritePin(C16_GPIO_Port, C05_Pin, GPIO_PIN_RESET);
	}

    return true;
}

/**
  * @brief  Set attenuator pads to correct value
  * @param attValue: attenuator total attenuation in dB
  */
void attenuator_set_pads(int attValue) {
	// Possible values: 0, 20, 40, 60, 80, 100
	// Convert to array index 0,1,2,3,4,5
	int attIndex = attValue/20;
	// All latch signals off
	HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_RESET);
	HAL_Delay(PE4312_TIME);

	// Attenuator Pad 1
	attenuator_set_value(ATTENUATOR_MATRIX[attIndex][0]);
	HAL_Delay(PE4312_TIME);
	// latch value into pad
	HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_SET);
	HAL_Delay(PE4312_TIME);
	HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_RESET);

	// Attenuator Pad 2
	attenuator_set_value(ATTENUATOR_MATRIX[attIndex][1]);
	HAL_Delay(PE4312_TIME);
	// latch value into pad
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_SET);
	HAL_Delay(PE4312_TIME);
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);

	// Attenuator Pad 3
	attenuator_set_value(ATTENUATOR_MATRIX[attIndex][2]);
	HAL_Delay(PE4312_TIME);
	// latch value into pad
	HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_SET);
	HAL_Delay(PE4312_TIME);
	HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_RESET);

	// Attenuator Pad 4
	attenuator_set_value(ATTENUATOR_MATRIX[attIndex][3]);
	HAL_Delay(PE4312_TIME);
	// latch value into pad
	HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_SET);
	HAL_Delay(PE4312_TIME);
	HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_RESET);
}

/**
  * @brief  Set attenuator pad service value on service pad
  */
void attenuator_set_service(void) {
	// All latch signals off
	HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_RESET);
	HAL_Delay(PE4312_TIME);

	attenuator_set_value(ServiceValue);
	if (ServicePE1) {
		HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_SET);
		HAL_Delay(PE4312_TIME);
		HAL_GPIO_WritePin(LE1_GPIO_Port, LE1_Pin, GPIO_PIN_RESET);
	} else if (ServicePE2)
	{
		HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_SET);
		HAL_Delay(PE4312_TIME);
		HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);
	} else if (ServicePE3)
	{
		HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_SET);
		HAL_Delay(PE4312_TIME);
		HAL_GPIO_WritePin(LE3_GPIO_Port, LE3_Pin, GPIO_PIN_RESET);
	} else if (ServicePE4)
	{
		HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_SET);
		HAL_Delay(PE4312_TIME);
		HAL_GPIO_WritePin(LE4_GPIO_Port, LE4_Pin, GPIO_PIN_RESET);
	}
	HAL_Delay(PE4312_TIME);
}

/**
  * @brief  Set attenuation in accordance with attenuator state
  */
void attenuator_set(void)
{
	int attValue = 0;
	// Calculate total attenuation
	if (Att40A_state) attValue += 40;
	if (Att40B_state) attValue += 40;
	if (Att20_state) attValue += 20;
	// Set bypass relay
	attenuator_bypass_relay(attValue);
	// set all attenuator pads
	attenuator_set_pads(attValue);
}

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  // Check for service mode (ATT40A_IN and ATT40A_OUT activated [low])
  // Other inputs to select attenuator pad to test
  if (HAL_GPIO_ReadPin(ATT40A_IN_GPIO_Port, ATT40A_IN_Pin) == GPIO_PIN_RESET)
  {
    if (HAL_GPIO_ReadPin(ATT40A_OUT_GPIO_Port, ATT40A_OUT_Pin) == GPIO_PIN_RESET)
    {
      ServiceMode = true;
      printf("Service Mode selected - release buttons\r\n");
	  if (HAL_GPIO_ReadPin(ATT20_IN_GPIO_Port, ATT20_IN_Pin) == GPIO_PIN_RESET)
	  {
      ServicePE1 = true;
	  } else if (HAL_GPIO_ReadPin(ATT20_OUT_GPIO_Port, ATT20_OUT_Pin) == GPIO_PIN_RESET)
	  {
	    ServicePE2 = true;
	  } else if (HAL_GPIO_ReadPin(ATT40B_IN_GPIO_Port, ATT40B_IN_Pin) == GPIO_PIN_RESET)
	  {
	    ServicePE3 = true;
	  } else if (HAL_GPIO_ReadPin(ATT40B_OUT_GPIO_Port, ATT40B_OUT_Pin) == GPIO_PIN_RESET)
	  {
	    ServicePE4 = true;
	  } else {
	  	ServicePE1 = true;	// in case 3rd button is not pressed
	  }
	  // Set bypass relay off
	  attenuator_bypass_relay(100);
	  // set all attenuator pads to 0dB
	  attenuator_set_pads(0);
    }
    // Wait for service mode trigger to be released
    while (1)
    {
      if (HAL_GPIO_ReadPin(ATT40A_IN_GPIO_Port, ATT40A_IN_Pin) == GPIO_PIN_SET) {
    	  if (HAL_GPIO_ReadPin(ATT40A_OUT_GPIO_Port, ATT40A_OUT_Pin) == GPIO_PIN_SET) {
    		break;
    	  }
      }
    }
    print_service_mode_PE();
    printf(" Service\r\n");
    HAL_Delay(200); 	// Let inputs settle.
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (!ServiceMode)
	// Normal operation mode
	{
		// change attenuator setting
		if (AttStateChanged)
		{
		  attenuator_set();
		  AttStateChanged = false;
		}
		MCU_stop();
	} else
	// Service Mode
	{
	  // Bypass relay
	  if (ServiceModeBypassChanged) {
	    ServiceModeBypassChanged = false;
	    attenuator_bypass_relay(!ServiceModeBypass);
	    printf("Bypass relay: %d\r\n", ServiceModeBypass);
	  }
	  if (ServiceModeInc) {
	  	if (ServiceValue <= 310) ServiceValue += 5;
	  	else ServiceValue = 0;
	  	ServiceModeInc = false;
	  	print_service_mode_PE();
	  	printf(" %d dB x 10", ServiceValue);
	  	attenuator_set_service();
	  }
	  if (ServiceModeDec) {
	  	if (ServiceValue >= 5) ServiceValue -= 5;
	  	else ServiceValue = 315;
	  	ServiceModeDec = false;
	  	print_service_mode_PE();
	  	printf(" %d dB x 10", ServiceValue);
	  	attenuator_set_service();
	  }
		HAL_Delay(10);
	}
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
