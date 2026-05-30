/**
  ******************************************************************************
  * @file    motor.cpp
  * @brief   Core motor initialization and control glue logic.
  ******************************************************************************
  */
extern "C" {
    #include "main.h"
    #include "gpio.h"
    #include "tim.h"
    #include "adc.h"
    #include "dma.h"
    #include <cstring>
    #include <cstdio>
    #include "motor.h"

void SystemClock_Config(void);
}

#ifdef __cplusplus
#include "..\..\App\Inc\IMotordriver.h"
#include "..\..\App\Inc\IMotor.h"
#include "..\..\App\Inc\Peripheral.h"

#include <cmath>

Gpio DefaultLed(GPIOB ,GPIO_PIN_13);
Gpio EnabePower(GPIOC ,GPIO_PIN_4);
IHM16M1 hbridge;
IMotorDriver *mymotordriver = &hbridge;
BLDC mybldc(mymotordriver);
IMotor *mymotor = &mybldc;
IPeripheral *myled = &DefaultLed;
IPeripheral *powerstage = &EnabePower;

volatile uint8_t control_mode=0;
volatile bool zero_crossing_detected = false;
volatile uint8_t syncounter = 0;
volatile uint8_t adc_samples_ready_for_main = 0;
void calculate_virtual_neutral(void);
void motor_process_adc_samples(void);

/* Internal filtering state (not part of public API) */
static float filtered_vbus = 0.0f;
static float filtered_bemf = 0.0f;
static const float vbus_alpha = 0.15f; /* slow LPF for DC link */
static const float bemf_alpha = 0.15f; /* faster LPF for bemf */
static float bemf_ma_buf[5] = {0};
static uint8_t bemf_ma_idx = 0;
static const uint8_t bemf_ma_len = 5;
static float bemf_ma_sum = 0.0f;

/* USER CODE END 0 */

/**
  * @brief  Initialize all required peripherals and start the motor control loop.
  * @retval None
  */
void peripherals_init(void)
{

     HAL_Init();
     SystemClock_Config();
     MX_DMA_Init();
      MX_ADC1_Init();
     MX_GPIO_Init();
     MX_TIM1_Init();
     MX_TIM2_Init();
     
    
     
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
  mymotor->calculated_duty_cycle = 0.55;
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim1);
  __HAL_TIM_MOE_ENABLE(&htim1);
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);

  while(1)
  {
    motor_process_adc_samples();
    HAL_Delay(1);
  }
}

/**
  * @brief  Configure the system clock for the STM32L4 device.
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

/**
  * @brief  Update rotor zero-crossing detection state.
  * @retval None
  */
inline void updatezerodetection(void)
{

}

/**
  * @brief  Convert a target RPM value into a timer ARR value.
  * @param  targetrpm Desired motor speed in RPM.
  * @retval None
  */
inline static void speedtoarr(float targetrpm)
{

}

/**
  * @brief  Execute open-loop motor control for startup.
  * @param  targetrpm Desired speed in RPM.
  * @retval None
  */
void run_openloop_control(float targetrpm)
{

   
}

/**
  * @brief  Run PID speed control to track the requested RPM.
  * @param  targetrpm Desired speed in RPM.
  * @param  currentrpm Measured current speed in RPM.
  * @retval None
  */
void run_pid_loop(float targetrpm, float currentrpm)
{

}

/**
  * @brief  Closed-loop speed control helper function.
  * @param  target_speed Desired speed setpoint.
  * @retval uint32_t Control output or timing value.
  */
uint32_t closed_loop_control(float target_speed)
{
    return 0;
}

void calculate_virtual_neutral(void)
{
    /* Detection is performed on DMA-complete ISR to guarantee fresh samples.
       keep this no-op to avoid duplicate processing. */
    if (control_mode != 1 && control_mode != 2) {
        return;
    }
    return;
}

/* ISR-safe entry called when new ADC samples are available. */
extern "C" void motor_adc_samples_ready_from_isr(void)
{
  /* Keep ISR lightweight: set flag for main loop to process fresh samples */
  adc_samples_ready_for_main = 1;
  /* Short toggle for timing probe */
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
}

/**
 * @brief Process ADC samples in main context (non-ISR).
 * @note Contains filtering, MA and zero-cross detection. Called from main loop.
 */
void motor_process_adc_samples(void)
{
  if(!adc_samples_ready_for_main) return;
  adc_samples_ready_for_main = 0;

  mymotor->read_all_sensors();

  /* update LPF for vbus and bemf */
  float vb = mymotor->voltage_reference;
  float bemf = mymotor->back_emf;

  filtered_vbus = (vbus_alpha * vb) + ((1.0f - vbus_alpha) * filtered_vbus);
  filtered_bemf = (bemf_alpha * bemf) + ((1.0f - bemf_alpha) * filtered_bemf);

  /* moving average window for additional smoothing */
  bemf_ma_sum -= bemf_ma_buf[bemf_ma_idx];
  bemf_ma_buf[bemf_ma_idx] = filtered_bemf;
  bemf_ma_sum += bemf_ma_buf[bemf_ma_idx];
  bemf_ma_idx = (bemf_ma_idx + 1) % bemf_ma_len;
  float bemf_ma = bemf_ma_sum / (float)bemf_ma_len;

  float neutral = filtered_vbus * 0.5f;
  const float hysteresis = 0.02f * (filtered_vbus > 0 ? filtered_vbus / 12.0f : 1.0f); /* scale with Vbus */
  const float min_amplitude = 0.01f * filtered_vbus; /* require some amplitude */

  static float prev_bemf_ma = 0.0f;

  bool look_for_rising = (mymotor->commutation_stage == 0 || mymotor->commutation_stage == 2 || mymotor->commutation_stage == 4);

  /* amplitude guard */
  if (fabsf(bemf_ma - neutral) < min_amplitude) {
    prev_bemf_ma = bemf_ma;
    return; /* ignore tiny signals */
  }

  if(!mymotor->zcp_found_this_step)
  {
    if(look_for_rising)
    {
      if(prev_bemf_ma < (neutral - hysteresis) && bemf_ma >= (neutral + hysteresis))
      {
        zero_crossing_detected = true;
        mymotor->zcp_found_this_step = true;
        uint32_t t_30 = __HAL_TIM_GET_COUNTER(&htim2);
        mymotor->commutation_timer_ticks = t_30 * 2;
        if(mymotor->commutation_timer_ticks < 1500) { mymotor->commutation_timer_ticks = 1500; t_30 = 750; }
        uint32_t next_commutation_target = t_30 * 2;
        __HAL_TIM_SET_AUTORELOAD(&htim2, next_commutation_target - 1);
      }
    }
    else
    {
      if(prev_bemf_ma > (neutral + hysteresis) && bemf_ma <= (neutral - hysteresis))
      {
        zero_crossing_detected = true;
        mymotor->zcp_found_this_step = true;
        uint32_t t_30 = __HAL_TIM_GET_COUNTER(&htim2);
        mymotor->commutation_timer_ticks = t_30 * 2;
        if(mymotor->commutation_timer_ticks < 1500) { mymotor->commutation_timer_ticks = 1500; t_30 = 750; }
        uint32_t next_commutation_target = t_30 * 2;
        __HAL_TIM_SET_AUTORELOAD(&htim2, next_commutation_target - 1);
      }
    }
  }

  prev_bemf_ma = bemf_ma;
}
/**
  * @brief  HAL callback executed when TIM output compare delay elapses.
  * @param  htim Pointer to TIM handle
  * @retval None
  */
extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim==&htim1 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_4)
  {
    /* Keep this callback minimal. ADC conversion completion triggers processing in main loop. */
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
  }
}


/**
  * @brief  HAL callback executed on TIM period elapsed event.
  * @param  htim Pointer to TIM handle
  * @retval None
  */
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
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    mymotor->start_motor_commutation(mymotor->commutation_stage,mymotor->calculated_duty_cycle);
    mymotor->zcp_found_this_step = false;
    if(control_mode == 0)
    {
    if(mymotor->commutation_timer_ticks>1000)
    {
        mymotor->commutation_timer_ticks = mymotor->commutation_timer_ticks-mymotor->acceleration_rate;
        if(mymotor->commutation_timer_ticks<3000)
        {
            control_mode = 1; 
            syncounter = 0;
        }
    }
    }
    else if(control_mode == 1)
    {
        if(zero_crossing_detected)
        {
        syncounter++;
        /* toggle debug LED on each successful zero-cross detection to observe on scope */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
        /* consume the event so a new detection is required next period */
        zero_crossing_detected = false;
        if(syncounter>5)
        {
          control_mode = 2; // Transition to closed-loop control after 5 successful syncs
        }
        }
        
    }
    else if(control_mode == 2)
    {
      /* Closed-loop: run PID on each TIM2 period to update duty */
      mymotor->run_speed_pid(mymotor->target_rpm);
    }

  /* cycle commutation stage 0..5 (six states) */
  if(mymotor->commutation_stage < 5)
  {
      mymotor->commutation_stage++;
  }
  else
  {
      mymotor->commutation_stage = 0;
      __HAL_TIM_SET_AUTORELOAD(&htim2,mymotor->commutation_timer_ticks);
  }
  __HAL_TIM_CLEAR_IT(htim,TIM_IT_UPDATE);
  

  }
    
}




#endif /* __cplusplus */








