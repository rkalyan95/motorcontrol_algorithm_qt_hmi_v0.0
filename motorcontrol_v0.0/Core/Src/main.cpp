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
volatile uint8_t z_detected = 0;
/* USER CODE END 0 */


     
     


void peripherals_init(void)
{

     HAL_Init();
     SystemClock_Config();
     MX_GPIO_Init();
     MX_TIM1_Init();
     MX_TIM2_Init();
     MX_DMA_Init();
     MX_ADC1_Init();
     
   powerstage->rawbuffer = 0;
   myled->rawbuffer = 0;
   powerstage->write();
   myled->write();
   __HAL_TIM_MOE_DISABLE(&htim1);
   mymotor->shutdown_all();
   mymotor->commutation_stage = 0;
  powerstage->rawbuffer = 0x01U;
  powerstage->write();
   mymotor->align_motor();
   mymotor->motorsynch = 0; 
   z_detected = 0;
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
  peripherals_init();
  HAL_Delay(10000);
  HAL_TIM_Base_Start_IT(&htim2);
while(1) 
{
     powerstage->read();
     if(powerstage->rawbuffer == 0)
    {
        mymotor->shutdown_all();
     }
     else
     {

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

extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
   /* Use this timer to sample the data tim1 channel 4*/
}


extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* use this timer to update the commutation stage
     let us begin to implement a simple open loop control 

   In mechanical , frequency is RPM/60 
   in electroca; , frquencys is Fmech * PolePair
   in commutation , frequency is Felct*6

   in timer terms , frequency is Fcommut = Fin/Arr
   here Fin = FinTimerClk / Prescaler = 4000000/100 = 40000
   so 
   Arr = Fin / Fcommut  = 40000/2000 = 20
   so 
   Arr can be 2000 initially to achieve 200Hz of commutation for open loop 
   to increase speed we will increase the commutation frequency , 
   which will further decrease the Arr 

   the below information is useless as of now 

   Here 


   Fcommut = Felectrical*6 = Fmech*PolePair*6 =  (Rpm/60)*(PolePair)*(6)
                                               (Rpm * PolePair*0.1)

  Fcommut = Rpm*PolePair*0.1
  Fcommut = 1*7*0.1 = 1.4 = 1.4

  so 
  Arr =  Fin/Fcommut = 40000/0.7 = 1428
  


*/

  if(htim==&htim2)
  {
    myled->rawbuffer = ~myled->rawbuffer & 0x01;
    myled->write();
    
    mymotor->start_motor_commutation(mymotor->commutation_stage,0.75f);
    mymotor->commutation_stage++;
    mymotor->commutation_stage = (mymotor->commutation_stage % 6); 
    __HAL_TIM_CLEAR_IT(htim,TIM_IT_UPDATE);
  }
}

