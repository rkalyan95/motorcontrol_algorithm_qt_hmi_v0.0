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


Gpio DefaultLed(GPIOB ,GPIO_PIN_13);
Gpio EnabePower(GPIOC ,GPIO_PIN_4);
IHM16M1 hbridge;
IMotorDriver *mymotordriver = &hbridge;
BLDC mybldc(mymotordriver);
IMotor *mymotor = &mybldc;
IPeripheral *myled = &DefaultLed;
IPeripheral *powerstage = &EnabePower;
uint8_t commutation_stage_main = 6;
float currentspeedrpm = 0.0f;
float prevspeedrpm = 0.0f;
float vbusvoltage = 0.0;
uint8_t z_detected = 0;
/* USER CODE END 0 */


     
     


void openloopcontrolmotor(void)
{

     HAL_Init();
     MX_GPIO_Init();
     MX_TIM1_Init();
     MX_TIM2_Init();
     MX_DMA_Init();
     MX_ADC1_Init();
     SystemClock_Config();
   powerstage->rawbuffer = 1;
   powerstage->write();
     __HAL_TIM_MOE_ENABLE(&htim1);
     mymotor->shutdown_all();

}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
   
  /* USER CODE BEGIN 1 */
  uint16_t com_count = 0;
  
  /* USER CODE END 1 */

  /* USER CODE END Init */
   

  /* USER CODE BEGIN WHILE */
  openloopcontrolmotor();
  HAL_Delay(2000);
  

   mymotor->commutation_stage = 0;
   mymotor->align_motor();
   
   /*while(com_count <= 10)
   {
    mymotor->start_motor_openloop(50.0f);
    mymotor->start_motor_commutation(mymotor->commutation_stage,0.75f);
    
    mymotor->commutation_stage++;
    if(mymotor->commutation_stage==6)
    {
      mymotor->commutation_stage = 0;
      com_count++;
    }
     HAL_Delay(5);
   }
   */

__HAL_TIM_SET_AUTORELOAD(&htim2, 39999);
HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE); 
__HAL_TIM_SET_COUNTER(&htim2, 0);

mymotor->motorsynch = 0; // This must block open-loop ARR writes
z_detected = 0;
mymotor->commutation_stage = 0; 
__HAL_TIM_SET_COUNTER(&htim2, 0);
HAL_TIM_Base_Start_IT(&htim2);
HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);
   
while(1) 
{
  
     powerstage->read();
     if(powerstage->rawbuffer == 0)
    {
        mymotor->shutdown_all();
     }
     else
     {
        currentspeedrpm+=40.0f;
     }
     HAL_Delay(5000);
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

extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    float ref_volt = mymotor->voltage_reference/2.0f;
    uint32_t nextArr = 0;
    bool triggred = 0;

    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
    {    
      
        mymotor->start_motor_openloop(500.0f);
        if(z_detected==0)
        {
          if(mymotor->commutation_stage % 2 == 0)  
          {
              if(mymotor->back_emf < ref_volt)
              {   
                triggred = 1;
                
              }
          }
          else
          {
            if((mymotor->back_emf > ref_volt))
            {
                  triggred = 1;
            }

          }

          if(triggred==1 && mymotor->motorsynch >= 2)
          {
            if(__HAL_TIM_GET_COUNTER(&htim2) > 10000) 
            {
              z_detected = 1;
              nextArr = __HAL_TIM_GET_COUNTER(&htim2)<<1;
              
                if(nextArr < 1999)
                {
                  nextArr = 1999;
                  
                }
                __HAL_TIM_SET_AUTORELOAD(&htim2,nextArr); 
            }
        }
        }

    }
}


extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
if (htim->Instance == TIM2)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  if(z_detected==1)
  {
    z_detected = 0;
  }
  mymotor->start_motor_commutation(mymotor->commutation_stage,mymotor->calculated_duty_cycle);
  mymotor->commutation_stage++;
  if(mymotor->commutation_stage==6)
  {
      mymotor->commutation_stage = 0;
  }
}
}

