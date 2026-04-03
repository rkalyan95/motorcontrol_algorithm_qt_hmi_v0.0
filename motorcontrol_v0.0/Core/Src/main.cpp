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
float rampeduprpm = 300.0f;
float prevspeedrpm = 0.0f;
float vbusvoltage = 0.0;

uint32_t prevarr = 0;
volatile uint8_t z_detected = 0;
void run_openloop_control(float targetrpm);
uint32_t closed_loop_control(float target_speed);
/* USER CODE END 0 */
  uint16_t countervalue = 0;
  float input_clk = 80000000.0f;
  uint32_t prescaler;
  float  fin;
  uint32_t currentarr;   
  float fcommut;   
  volatile bool openloop = true;
float Kp = 0.005f;  // Proportional gain (adjust based on motor response)
float Ki = 0.002f; // Integral gain
float integral_error = 0.0f;

float dutycyclenew = 0.35f;
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
  //HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  HAL_Delay(1000);

  HAL_TIM_Base_Start_IT(&htim2);

  __HAL_TIM_MOE_ENABLE(&htim1);
  // Start the motor channels normally
// Start Channel 4 specifically to trigger the Interrupt
HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);
  float targetrpm = 300.0f;
  
  run_openloop_control(targetrpm);

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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

inline void updatezerodetection(void)
{
    if(mymotor->commutation_stage%2==0)
        {
          /*Rising Phase*/
          if(mymotor->back_emf > mymotor->voltage_reference/2.0f)
          {
            z_detected = 1;
            mymotor->motorsynch++;
          }
        }
        else
        {
          /*Falling Phase*/
         if(mymotor->back_emf < mymotor->voltage_reference/2.0f)
          {
            z_detected = 1;
            mymotor->motorsynch++;
          }
        }

}


inline static void speedtoarr(float targetrpm)
{
  prescaler = htim2.Init.Prescaler;
  fin = (float)(input_clk/(float)prescaler);
  fcommut = (float)((currentspeedrpm*(float)mymotor->polepair)/10.0f);

  currentarr = (uint32_t)(fin/(fcommut));
}


void run_openloop_control(float targetrpm)
{
  currentspeedrpm = targetrpm;
  do{
    if(currentspeedrpm>rampeduprpm)
    {
      currentspeedrpm = currentspeedrpm*0.75f;
    }
    else
    {
      
    }
    speedtoarr(currentspeedrpm);
    __HAL_TIM_SetAutoreload(&htim2,currentarr);
   prevarr = currentarr;
   HAL_Delay(50);
 }while(mymotor->motorsynch<6000);


   
   myled->rawbuffer = ~myled->rawbuffer & 0x01;
   myled->write();
   openloop = false;
   
}

void run_pid_loop(float targetrpm, float currentrpm)
{
    float error = targetrpm-currentrpm;
    
    integral_error+=error;

    if(integral_error > 100.0f) integral_error = 100.0f;
    if(integral_error < -100.0f) integral_error = -100.0f;  

    float correction = (Kp * error) + (Ki * integral_error);
    mymotor->set_motor_speed(correction);
    //mymotor->calculated_duty_cycle += correction;

    if(mymotor->calculated_duty_cycle > 0.95f) mymotor->calculated_duty_cycle = 0.95f;
    if(mymotor->calculated_duty_cycle < 0.35f) mymotor->calculated_duty_cycle = 0.37f;

    dutycyclenew = mymotor->calculated_duty_cycle;
}

uint32_t closed_loop_control(float target_speed)
{

  float actual_rpm = 0.0f;
   uint32_t blankingpercent=40;
   uint32_t blankingth = 0;
   volatile uint32_t currentcount = 0;

    /* 
       check for openloop bool flag 
       check for z_detected true
       once z_detected true 
       read the current timer 2 which is the commutation timer count value
       then use a blanking window calculation for 20% of the count 
       if the count is greater than the blanking window amount 
       set nextcommutation to true 
       set the arr value to 2xcount read above
       and handle the nextcommutation step in the timer 2 handler which is the commutation handler

    
      */

      if(!openloop)
      {
        if(z_detected)
        {

            currentcount = __HAL_TIM_GET_COUNTER(&htim2);
            actual_rpm = (10000000.0f * 10.0f) / (float)((currentcount * mymotor->polepair));
            currentarr = currentcount*2;

            blankingth = (blankingpercent*prevarr)/100;
            if(currentcount>blankingth)
            {
              if(currentcount<100)
             {
                currentcount = 100;
              }
             // //if (currentcount >= (currentarr - 5)) 
             // {
                __HAL_TIM_SET_AUTORELOAD(&htim2, currentcount);
             // }
             // else 
            //  {
              //    __HAL_TIM_SET_AUTORELOAD(&htim2, currentarr);
             /// }
             //   prevarr = currentarr;
              //  z_detected = false;
                
            }

            
            
            run_pid_loop(target_speed, actual_rpm);

        }
        else
        {
          return 0;
        }
      }
      else
      {
        return 0;
      }

      return currentarr;
}

extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t counter = 0;

    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
    {
          mymotor->read_all_sensors();
          updatezerodetection();
          
          if (!openloop && z_detected == 1 )
                  {

                      closed_loop_control(1700.0f); 

                        

                  }
    }
}


extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* use this timer to update the commutation stage
     let us begin to implement a simple open loop control 

   In mechanical , frequency is RPM/60 
   in electroca; , frquencys is Fmech * PolePair
   in commutation , frequency is Felct*6

   in timer terms , frequency is Fcommut = Fin/Arr
   here Fin = FinTimerClk / Prescaler = 80000000/100 = 800000
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
  fcommut max  = 2000*7*0.1 = 1400
  Arr = Fin/Fcommut 

  1. Fin = input_clk/prescaler+1 = 4*10^6/99+1
  2. Fcommut = Rpm * PolePar * 0.1
  3. Arr = Fin/Fcommut

  Fcommut = 1*7*0.1 = 1.4 = 1.4

  so 
  Arr =  Fin/Fcommut = 40000/0.7 = 1428
  
  base arr = 57142 , the count which makes this motor run at 1rpm
  if we divide the base arr by 2 , we will get the count which makes this motor run at 2rpm
  for one complete rotation , the function would be called 42 times for 7 pole 
  6*polepair = 42

*/

  if(htim==&htim2)
{
  //__HAL_TIM_SET_COUNTER(&htim2, 0);
    if (openloop)
    {
       mymotor->start_motor_commutation(mymotor->commutation_stage,0.35f);
    }
    else
    {
       mymotor->start_motor_commutation(mymotor->commutation_stage,dutycyclenew);
    }
    

    mymotor->commutation_stage++;
    mymotor->commutation_stage = (mymotor->commutation_stage % 6); 
    z_detected = 0;
    __HAL_TIM_CLEAR_IT(htim,TIM_IT_UPDATE);
  }
    
}

