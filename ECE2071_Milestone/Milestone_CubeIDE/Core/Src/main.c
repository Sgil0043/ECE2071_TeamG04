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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
int checkSum(uint8_t msg[]) {

    int checksum = 0; // initialising the checksum to equal 0

    for (int i = 0; msg[i] != '\0'; i++) { // iterating through each letter in the string we are testing
        checksum = checksum ^ msg[i]; // updating the checksum after each character so that we can claculate it for the whole string
    }
    return checksum;
}
//hello this is a test change
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char ID[] = "G04_0"; // ID = G04_0, G04_1, G04_3, G04_4 :)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint8_t daisy_chain_msg[256]; // daisy_chain_msg => the message which is to be passed through the daisy-chain of STMs
      uint8_t byte; // byte => the byte which is being transmitted / received
      int i; // i => counter for the number of bytes written into the message thus far

      // INPUT FROM PC -> BEGINS LOOP
      if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) { // Every UART has a 'register' with a flag called RXNA. (Credit to Jay Lynch (EdStem)).
    	  	  	  	  	  	  	  	  	  	  	  	  	  // The hardware itself gets this flag to 1 (truthy) the second a byte is received
    	  	  	  	  	  	  	  	  	  	  	  	  	  // Loops continues (while loop) til the flag says that either UART1 or UART2 has received an input.

    	  HAL_UART_Receive(&huart2, &byte, 1, HAL_MAX_DELAY); // Retrieves the first byte from UART2

          memset(daisy_chain_msg, 0, sizeof(daisy_chain_msg)); // Resets the message to be empty
          i = 0; // Sets the counter / size of the message to 1
          daisy_chain_msg[i] = byte;
          i++;

          while (byte != '\n' && i < 255) { // Appends all characters in the msg
              if (HAL_UART_Receive(&huart2, &byte, 1, 1000) != HAL_OK) {
            	  break; // Tries to receive another byte from UART2 - if it does not after 1000 ms, it bails from this.
            	  	  	 // Stops the code from hanging, trying to receive a message.
              }
              daisy_chain_msg[i] = byte; // Append one character to the message
              i++; // Increment length of the message
          }

          daisy_chain_msg[i-1] = '\0';
          strcat((char*)daisy_chain_msg, "_");
		  strcat((char*)daisy_chain_msg, ID);  // change number per node

		  int csInt1 = checkSum(daisy_chain_msg);
		  char csHex1[3];
		  sprintf(csHex1, "%02X", csInt1);
		  strcat((char*)daisy_chain_msg, csHex1);

		  strcat((char*)daisy_chain_msg, "\n"); // Adds a new line to denote the end of the msg
		  i = strlen((char*)daisy_chain_msg); // The new length of the string

		  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turns LED3 on
		  uint32_t hold = HAL_GetTick(); // Gets the starting hold time
		  while (HAL_GetTick() - hold < 250); // Hold/wait token for 250ms, HAL_Delay does not work here as it would mess with timing!

		  HAL_UART_Transmit(&huart1, daisy_chain_msg, i, HAL_MAX_DELAY); // Transmits the message to the next STM
		  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Turns LED3 off

          memset(daisy_chain_msg, 0, sizeof(daisy_chain_msg)); // Sets the message to empty
          i = 0; // Sets the message length to 0

          uint32_t t = HAL_GetTick(); // Gets the current time (ms)

          while (!__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) { // Waits whilst RXNE is Falsy (i.e. has nothing)
              if (HAL_GetTick() - t > 2000) { // Waits for 2 seconds. If there is no loop back, then it moves on.
            	  break;
              }
          }

          if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) { // Once a byte has been received, RXNE will be truthy
        	  HAL_UART_Receive(&huart1, &byte, 1, HAL_MAX_DELAY); // Receive first byte

        	  daisy_chain_msg[i] = byte; // Append first character
        	  i++; // Increment msg length

              while (byte != '\n' && i < 255) { // Appends all characters in the msg
                  if (HAL_UART_Receive(&huart1, &byte, 1, 1000) != HAL_OK) { //
                	  break;  // Tries to receive another byte from UART2 - if it does not after 1000 ms, it bails from this.
                	  	  	  // Stops the code from hanging, trying to receive a message.
                  }

                  daisy_chain_msg[i] = byte; // Assign character
                  i++; // Increment msg length
              }

              char csHexhp[3] = { daisy_chain_msg[i-3], daisy_chain_msg[i-2], '\0' };
              int checkSumPrev = (int)strtol(csHexhp, NULL, 16);
              daisy_chain_msg[i-3] = '\0';
              daisy_chain_msg[i-2] = 0;
              daisy_chain_msg[i-1] = 0;

              int checkSumCurr = checkSum(daisy_chain_msg);

              if (checkSumCurr == checkSumPrev) {
            	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turns LED3 on
				  uint32_t hold = HAL_GetTick(); // Gets the starting hold time
				  while (HAL_GetTick() - hold < 250); // Hold/wait token for 250ms, HAL_Delay does not work here as it would mess with timing!

				  strcat((char *)daisy_chain_msg, "\n");
				  int msg_len = strlen((char *)daisy_chain_msg);

				  HAL_UART_Transmit(&huart2, daisy_chain_msg, msg_len, HAL_MAX_DELAY); // Transmit the message back to the PC
				  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Turns LED3 off
              }

              else {
            	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turns LED3 on
            	  while(1);
              }

          }
      }

      // THROUGHOUT LOOP -> PASSES MESSAGE THROUGHOUT
      else if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) { // Every UART has a 'register' with a flag called RXNA.
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	   // The hardware itself gets this flag to 1 (truthy) the second a byte is received
	  	  	  	  	  	                                       // Loops continues (while loop) til the flag says that either UART1 or UART2 has received an input.

    	  HAL_UART_Receive(&huart1, &byte, 1, HAL_MAX_DELAY); // Receives first byte

          memset(daisy_chain_msg, 0, sizeof(daisy_chain_msg)); // Sets the message array to empty
          i = 0; // Sets the message length to empty
          daisy_chain_msg[i] = byte; // Assigns the first character
          i++; // Increments the message length

          while (byte != '\n' && i < 255) { // Appends all characters in the msg
              if (HAL_UART_Receive(&huart1, &byte, 1, 1000) != HAL_OK) {
            	  break; // Tries to receive another byte from UART1 - if it does not after 1000 ms, it bails from this.
    	  	  	  	  	 // Stops the code from hanging, trying to receive a message.
              }
              daisy_chain_msg[i] = byte; // Assigns the next character in the message
              i++; // Increments the message length
          }

          char csHexnp[3] = { daisy_chain_msg[i-3], daisy_chain_msg[i-2], '\0' };
          int checkSumPrev = (int)strtol(csHexnp, NULL, 16);
          daisy_chain_msg[i-3] = '\0';
          daisy_chain_msg[i-2] = 0;
          daisy_chain_msg[i-1] = 0;

          int checkSumCurr = checkSum(daisy_chain_msg);

          if (checkSumCurr != checkSumPrev) {
        	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turns LED3 on}
        	  while(1);
          }

          daisy_chain_msg[i-1] = '\0';
          strcat((char*)daisy_chain_msg, "_");
		  strcat((char*)daisy_chain_msg, ID);  // Changes number per node

		  int csIntn = checkSum(daisy_chain_msg);
		  char csHexn[3];
		  sprintf(csHexn, "%02X", csIntn);
		  strcat((char*)daisy_chain_msg, csHexn);

		  strcat((char*)daisy_chain_msg, "\n"); // Adds a new line to denote the end of the msg
		  i = strlen((char*)daisy_chain_msg); // The new length of the string


		  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turns LED3 On
		  uint32_t hold = HAL_GetTick(); // Gets the starting hold time
		  while (HAL_GetTick() - hold < 250); // Hold token for 250ms

		  HAL_UART_Transmit(&huart1, daisy_chain_msg, i, HAL_MAX_DELAY); // Transmits message to next daisy-chained STM
		  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Turns LED3 Off
      }
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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

#ifdef  USE_FULL_ASSERT
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
