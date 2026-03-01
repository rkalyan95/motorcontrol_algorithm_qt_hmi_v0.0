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
    #include "adc.h"
    #include <cstring>
    #include <cstdio>
}

// 2. Include your C++ logic
#include "fsm.h"
#include "motor.h"


void SystemClock_Config(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint16_t my_reading[8];

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
Gpio EnablePhaseU(GPIOC ,GPIO_PIN_8); 
Gpio EnablePhaseV(GPIOB ,GPIO_PIN_6);
Gpio EnablePhaseW(GPIOA ,GPIO_PIN_11);

Timer timer1(&htim1, (uint32_t)TIM_CHANNEL_1);
Timer timer2(&htim1, (uint32_t)TIM_CHANNEL_2);
Timer timer3(&htim1, (uint32_t)TIM_CHANNEL_3);

ADC    adc1(&hadc1, (uint32_t)ADC_CHANNEL_1);
Sensor mysensor(&adc1, 1, 0);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
   
  /* USER CODE BEGIN 1 */
   bool ledstate  = 0;
   float dutycycle = 0.15;
   IPeripheral *myled = &DefaultLed;
   IPeripheral *mytimer1 = &timer1;
   IPeripheral *mytimer2 = &timer2;
   IPeripheral *mytimer3 = &timer3;
   IPeripheral *myadc1 = &adc1;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN Init */
  // 1. Start the 3 PWM channels


// Update Channel 1 (PA8) to 50% duty cycle
// If ARR is 39999, set CCR to 20000
//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 100);

// Update Channel 2 (PA9)
//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 100);

// Update Channel 3 (PA10)
//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 100);

//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // PA8
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // PA9
//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); // PA10
//timer1.setdutycycle(&dutycycle);
//timer2.setdutycycle(&dutycycle);
//timer3.setdutycycle(&dutycycle);
 SystemClock_Config();
mytimer1->rawbuffer = 100;
mytimer2->rawbuffer = 120;
mytimer3->rawbuffer = 190;
mytimer1->write();
mytimer2->write();
mytimer3->write();
mytimer1->init();
mytimer2->init();
mytimer3->init();

__HAL_TIM_MOE_ENABLE(&htim1);   //will be handled by motor class
//timer1.startpwm();
//timer2.startpwm();
//timer3.startpwm();
// 2. Enable the Main Output (Crucial for Advanced Timers)

// 3. Keep the IHM16M1 board powered (Nucleo-P SMPS pin)
//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); 

// 4. For your test, turn on Phase U Enable
//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
  /* USER CODE END Init */
   
  /* Configure the system clock */
 

  /* USER CODE BEGIN SysInit */
   
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  
 
  /* USER CODE BEGIN 2 */
   myled->init();  
  
   myadc1->init();
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1) {
    myled->rawbuffer = 1;
    myled->write();
    mysensor.init();
    mysensor.read();
    mysensor.uninit();
    HAL_Delay(mysensor.sensroraw);
    myled->rawbuffer = 0;
    myled->write();
    mysensor.init();
    mysensor.read();
    mysensor.uninit();
    HAL_Delay(mysensor.sensroraw);
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
