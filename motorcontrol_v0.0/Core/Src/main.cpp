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


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern "C" {
    #include "main.h"
    #include "gpio.h"
    #include "tim.h"
    #include "usart.h"
    #include <cstring>
    #include <cstdio>
}

// 2. Include your C++ logic
#include "fsm.h"



void SystemClock_Config(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


Gpio DefaultLed(led_builtin_nucleo_GPIO_Port ,GPIO_PIN_13);
Gpio tinyMlButton(extern_button_ml_GPIO_Port,extern_button_ml_Pin);
Timer PWM_PA0(&htim2, TIM_CHANNEL_1);
Timer PWM_PA1(&htim2, TIM_CHANNEL_1);
Timer PWM_PA2(&htim2, TIM_CHANNEL_1);
float weights[50] = {
-0.0000f, -0.0000f, -0.0000f, -0.0000f, -0.0000f, -0.0000f, -0.0000f, 
-0.0000f, -0.0000f, -0.0000f, -0.0827f, -0.1974f, -0.4032f, -0.3515f, 
-0.1192f, -0.0935f, -0.0230f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 
0.0000f, 0.0386f, 0.1376f, 0.2118f, 0.4065f, 0.6143f, 0.6143f, 0.7335f, 
0.7335f, 0.7335f, 0.7335f, 0.7335f, 0.5664f, 0.4957f, 0.1853f, 0.0000f, 
0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 
0.0000f, 0.0000f, 0.0000f, 0.0000f
};
float bias = -2.4701f;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
   
  /* USER CODE BEGIN 1 */
GPIO_PinState mlbuttonstate = GPIO_PIN_SET;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
   MX_USART1_UART_Init();
  /* USER CODE END Init */
   
  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
   
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  
 
  /* USER CODE BEGIN 2 */
  float live_input[20];
  float sum = 0.0f;
  float final_result = 0.0f;
  DefaultLed.Init();  
  tinyMlButton.Init();
  PWM_PA0.Init();
  PWM_PA1.Init();
  PWM_PA2.Init();

  PWM_PA0.PwmSetDutyCycle(0);
  PWM_PA1.PwmSetDutyCycle(0);
  PWM_PA2.PwmSetDutyCycle(0);

  PWM_PA0.RunTimer();
  PWM_PA1.RunTimer();
  PWM_PA1.RunTimer();
  HAL_UART_Transmit(&huart1, (uint8_t *)"welcome", strlen("welcome"), 500);
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1) {
    // Wait for initial press

    if (tinyMlButton.Get() == GPIO_PIN_RESET) 
    {
    float live_input[50];
    float sum = 0.0f;

    // 1. Capture 500ms of data
    for (int i = 0; i < 50; i++) {
        live_input[i] = (tinyMlButton.Get() == GPIO_PIN_RESET) ? 1.0f : 0.0f;
        HAL_Delay(10);
    }

    // 2. Compute Dot Product (Inference)
    for (int i = 0; i < 50; i++) {
        sum += (live_input[i] * weights[i]);
    }

    float final_score = sum + bias;

    // 3. AI Decision
    if (final_score > 0.0f) {
        // Positive score = Double Click (Class 1)
        HAL_UART_Transmit(&huart1, (uint8_t*)"AI: DOUBLE CLICK\r\n", 18, 100);
        DefaultLed.Toggle(); 
    } else {
        // Negative score = Single Click (Class 0)
        HAL_UART_Transmit(&huart1, (uint8_t*)"AI: SINGLE CLICK\r\n", 18, 100);
        DefaultLed.Toggle();
    }

    // Wait for release
    while (tinyMlButton.Get() == GPIO_PIN_RESET);
  }
  }

    /*if (tinyMlButton.Get() == GPIO_PIN_RESET) {
        float live_input[50];
        float sum = 0.0f;
        // Capture 50 samples (10ms apart)

        
        for (int i = 0; i < 50; i++) {
            int state = (tinyMlButton.Get() == GPIO_PIN_RESET) ? 1 : 0;
            
            // Print to Serial: 1,0,1... (no spaces, just commas)
            char buf[4];
            sprintf(buf, "%d%s", state, (i == 49) ? "" : ",");
            HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 10);
            
            HAL_Delay(10);
        }
        
        // Newline at the end of the pattern
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 10);
        
        // Wait for release before allowing next capture
        while (tinyMlButton.Get() == GPIO_PIN_RESET);

        */
    

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
