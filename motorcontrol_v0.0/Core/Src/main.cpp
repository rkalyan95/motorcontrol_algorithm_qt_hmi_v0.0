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
    #include "dma.h"
    #include <cstring>
    #include <cstdio>
}

// 2. Include your C++ logic
#include <IMotordriver.h>
#include <IMotor.h>
#include <Peripheral.h>

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
Gpio EnabePower(GPIOC ,GPIO_PIN_4);
IHM16M1 hbridge;
   IMotorDriver *mymotordriver = &hbridge;
   BLDC mybldc(mymotordriver);
   IMotor *mymotor = &mybldc;
   IPeripheral *myled = &DefaultLed;
   IPeripheral *powerstage = &EnabePower;
  uint8_t commutation_stage_main = 6;
   float speedrpm = 6.0f;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
   
  /* USER CODE BEGIN 1 */
  uint8_t com_count = 0;
  
  /* USER CODE END 1 */

  /* USER CODE END Init */
   
   HAL_Init();
   
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  
  SystemClock_Config();
  /* USER CODE BEGIN WHILE */

  HAL_Delay(2000);
  __HAL_TIM_MOE_ENABLE(&htim1);
   powerstage->rawbuffer = 1;
   powerstage->write();
   mymotor->shutdown_all();
   mymotor->align_motor();
   //HAL_Delay(2000);
   uint32_t counter = 39999;
/*

   
   __HAL_TIM_SET_AUTORELOAD(&htim2, 19999);
while(counter!=1999)
{
    
    mymotor->start_motor_commutation(commutation_stage_main,0.30);

    commutation_stage_main++;
    if(commutation_stage_main==6)
    {
      commutation_stage_main = 0;
      com_count++;
      __HAL_TIM_SET_AUTORELOAD(&htim2, counter--);
    }
    HAL_Delay(10);
  }
  */
   __HAL_TIM_SET_AUTORELOAD(&htim2, counter);
   HAL_TIM_Base_Start_IT(&htim2);
   mymotor->set_motor_speed(speedrpm); // Calculating voltage for 100 RPM
      myled->rawbuffer = 0;
    myled->write();
    HAL_Delay(2000);
while(1) 
{
  
   if(adcdmainterrupt==1)
   {
     adcdmainterrupt = 0;
    myled->rawbuffer = 1;
    myled->write();
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

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
if (htim->Instance == TIM2)
{
  
  mymotor->start_motor_commutation(commutation_stage_main,0.50f);
  //mymotor->set_motor_speed(speedrpm); // Calculating voltage for 100 RPM
  commutation_stage_main++;
  if(commutation_stage_main==6)
  {
      commutation_stage_main = 0;
  }


}
}

