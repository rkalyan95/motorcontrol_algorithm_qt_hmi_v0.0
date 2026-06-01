/**
 * @file motor.c
 * @brief BLDC motor control implementation, ADC/DMA sensor sampling, and commutation helpers.
 *
 * This file provides the motor startup sequence, commutation stage handling, BEMF processing,
 * timer callback handling, and helper routines for GPIO/ADC/TIM access.
 */

#include "motor.h"
#include "gpio.h"
#include "adc.h"
#include "tim.h"
#include <stdint.h>
#include <stdbool.h>
/*
EnablePhaseU (GPIOC ,GPIO_PIN_8); 
EnablePhaseV (GPIOB ,GPIO_PIN_6);
EnablePhaseW (GPIOC ,GPIO_PIN_7);
Gpio EnabePower(GPIOC ,GPIO_PIN_4);
PwmPhaseU(&htim1, (uint32_t)TIM_CHANNEL_1);
PwmPhaseV(&htim1, (uint32_t)TIM_CHANNEL_2);
PwmPhaseW(&htim1, (uint32_t)TIM_CHANNEL_3);

VBUS(&hadc1, (uint32_t)ADC_CHANNEL_1);   //PC0
NTC(&hadc1, (uint32_t)ADC_CHANNEL_2);   //PC1

CURRFDBK_1(&hadc1, (uint32_t)ADC_CHANNEL_8); //PA3
CURRFDBK_2(&hadc1, (uint32_t)ADC_CHANNEL_3); //PC2
CURRFDBK_3(&hadc1, (uint32_t)ADC_CHANNEL_4); //PC3

BEMF_PHA_V(&hadc1, (uint32_t)ADC_CHANNEL_15);//PB0
BEMF_PHA_U(&hadc1, (uint32_t)ADC_CHANNEL_6); //PA1
BEMF_PHA_W(&hadc1, (uint32_t)ADC_CHANNEL_16);//PB1
*/

uint16_t dmalocalbuffer[8];
static volatile uint8_t dmadone = 0;

uint8_t floating_phase = 0;
uint8_t commutation_stage = 0;
#define PWM_PERIOD 19

uint32_t commutation_ticks = 142851; 
uint32_t acceleration = 2500;
#define MIN_COMMUTATION_TICKS 3000
#define SYNC_COMMUTATION_TICKS 9000

volatile bool zerocrossing_detected = false;
volatile uint8_t motor_mode = 0;
uint16_t vbus ;
uint16_t ntc;
 uint16_t curr1 ;
 uint16_t curr2 ;
uint16_t curr3 ;
uint16_t bemf_u ;
uint16_t bemf_w ;
uint16_t bemf_v ;
volatile float bemf_phy = 0.0f;
volatile uint8_t sync_counter = 0;
/**
 * @brief Set a GPIO output pin to the active state.
 * @param pin GPIO pin identifier.
 * @param port GPIO port base address.
 */
static void setgpio(uint16_t pin, GPIO_TypeDef *port)
{
    if(port==NULL)
    {
        return;
        
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

}
/**
 * @brief Clear a GPIO output pin.
 * @param pin GPIO pin identifier.
 * @param port GPIO port base address.
 */
static void cleargpio(uint16_t pin, GPIO_TypeDef *port)
{
    if(port==NULL)
    {
        return;
        
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}
/**
 * @brief Read a GPIO input pin state.
 * @param pin GPIO pin identifier.
 * @param port GPIO port base address.
 * @param[out] value Returned pin state (0 or 1).
 */
static void readgpio(uint16_t pin, GPIO_TypeDef *port,uint8_t *value)
{
    if(port==NULL)
    {
        return;  
    }

    *value = HAL_GPIO_ReadPin(port, pin);
}

/**
 * @brief Initialize the ADC driver and start DMA into the local buffer.
 * @param hadc Pointer to the ADC handle.
 */
static void adc_init(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    HAL_ADC_Start_DMA(hadc, (uint32_t *)dmalocalbuffer, 8);
}

/**
 * @brief Stop ADC DMA and deinitialize ADC sampling.
 * @param hadc Pointer to the ADC handle.
 */
static void adc_uninit(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    HAL_ADC_Stop_DMA(hadc);
}

/**
 * @brief Read the latest ADC sample for a configured channel from the DMA buffer.
 * @param channel_num ADC channel identifier.
 * @return ADC sample value, or 0 if the channel is unknown.
 */
static uint16_t adc_read(uint8_t channel_num)
{
    uint16_t value = 0;
    uint8_t index = 0;
    switch (channel_num)
     {
     case ADC_CHANNEL_1:
         index = 0;
         break; // VBUS
     case ADC_CHANNEL_2:
         index = 1;
         break; // NTC
     case ADC_CHANNEL_8:
         index = 2;
         break; // CURR1
     case ADC_CHANNEL_3:
         index = 3;
         break; // CURR2
     case ADC_CHANNEL_4:
         index = 4;
         break; // CURR3
     case ADC_CHANNEL_6:
         index = 5;
         break; // BEMF_U
     case ADC_CHANNEL_16:
         index = 7;
         break; // BEMF_W
     case ADC_CHANNEL_15:
         index = 6;
         break; // BEMF_V
     default:
        return;
     }
    
    value = dmalocalbuffer[index];

    return value;
}

/**
 * @brief Start PWM output on the specified timer channel.
 * @param htim Pointer to the timer handle.
 * @param Channel Timer channel identifier.
 */
static void tim_init(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    if(htim==NULL)
    {
        return;
    }
    HAL_TIM_PWM_Start(htim, Channel);
}

/**
 * @brief Stop PWM output on the specified timer channel.
 * @param htim Pointer to the timer handle.
 * @param Channel Timer channel identifier.
 */
static void tim_uninit(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    if(htim==NULL)
    {
        return;
    }
    HAL_TIM_PWM_Stop(htim, Channel);
}

/**
 * @brief Set the compare value for a timer PWM channel.
 * @param htim Pointer to the timer handle.
 * @param Channel Timer channel identifier.
 * @param value Compare value to write.
 */
static void tim_write(TIM_HandleTypeDef *htim, uint32_t Channel,uint32_t value)
{
    if(htim==NULL)
    {
        return;
    }    
    __HAL_TIM_SET_COMPARE(htim, Channel, value);
}

/**
 * @brief Read the current PWM compare value from a timer channel.
 * @param htim Pointer to the timer handle.
 * @param Channel Timer channel identifier.
 * @param[out] value Current compare register value.
 */
static void tim_read(TIM_HandleTypeDef *htim, uint32_t Channel,uint32_t *value)
{
    if(htim==NULL)
    {
        return;
    }
    *value = __HAL_TIM_GET_COMPARE(htim, Channel);
}


/**
 * @brief Convert a phase duty cycle to a timer compare value and update the PWM channel.
 * @param phase Motor phase index: 0=U, 1=V, 2=W.
 * @param dutycycle Duty cycle in the range [0.0, 1.0].
 */
static void pwm_write(uint8_t phase, float dutycycle)
{
    if(dutycycle>1.0f) 
    { 
        dutycycle = 1.0f;
    }
    else if(dutycycle<0.0f) 
    {
        dutycycle = 0.0f;
    }
    switch(phase)
    {
        case 0:
            tim_write(&htim1, (uint32_t)TIM_CHANNEL_1, (uint32_t)(dutycycle*19)); // Phase U
            break;
        case 1:
            tim_write(&htim1, (uint32_t)TIM_CHANNEL_2, (uint32_t)(dutycycle*19)); // Phase V
            break;
        case 2:
            tim_write(&htim1, (uint32_t)TIM_CHANNEL_3, (uint32_t)(dutycycle*19)); // Phase W
            break;
        default:
            return;
    }
    
}





/**
 * @brief Enable a motor phase by switching its enable GPIO and applying PWM.
 * @param phase Motor phase index: 0=U, 1=V, 2=W.
 * @param dutycycle PWM duty cycle to apply on the enabled phase.
 */
static void enable_pwm_phase(uint8_t phase,float dutycycle)
{
    switch(phase)
    {
        case 0:
            setgpio(GPIO_PIN_8, GPIOC); // Disable ENU
            pwm_write(phase,dutycycle);
            break;
        case 1:
            setgpio(GPIO_PIN_6, GPIOB); // Disable ENV
            pwm_write(phase,dutycycle);
            break;
        case 2:
            setgpio(GPIO_PIN_7, GPIOC); // Disable ENW
            pwm_write(phase,dutycycle);
            break;
        default:
            return;
    }
}

/**
 * @brief Disable a motor phase by clearing its enable GPIO and forcing zero PWM.
 * @param phase Motor phase index: 0=U, 1=V, 2=W.
 */
static void disable_pwm_phase(uint8_t phase)
{
    switch(phase)
    {
        case 0:
            cleargpio(GPIO_PIN_8, GPIOC); // Disable ENU
            pwm_write(phase,0.0f);
            break;
        case 1:
            cleargpio(GPIO_PIN_6, GPIOB); // Disable ENV
            pwm_write(phase,0.0f);
            break;
        case 2:
            cleargpio(GPIO_PIN_7, GPIOC); // Disable ENW
            pwm_write(phase,0.0f);
            break;
        default:
            return;
    }
}

/**
 * @brief Process the floating phase BEMF measurement and detect zero crossings.
 *
 * Converts the selected raw ADC BEMF channel into a physical voltage value, then sets
 * the zero crossing flag when the floating phase crosses the estimated neutral voltage.
 */
void process_bemf(void)
{
    
    float neutral_voltage = 6.0f;
    bool rising_edge = (commutation_stage%2==0);
    uint32_t current_ticks = 0;
    if(motor_mode==0)
    {
        return;
    }
    switch(floating_phase)
    {
        case 0:
            
            bemf_phy = (float)bemf_u * 5.545f * 0.0008058f;
            if(rising_edge && bemf_phy>neutral_voltage)
            {
                    zerocrossing_detected = true;
                    current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            else if(!rising_edge && bemf_phy<neutral_voltage)
            {
                    zerocrossing_detected = true;
                    current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            break;
        case 1:
            bemf_phy = (float)bemf_v * 5.545f * 0.0008058f;
            if(rising_edge && bemf_phy>neutral_voltage)
            {
                    zerocrossing_detected = true;
                     current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            else if(!rising_edge && bemf_phy<neutral_voltage)
            {
                    zerocrossing_detected = true;
                     current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            break;
        case 2:
            bemf_phy = (float)bemf_w * 5.545f * 0.0008058f;
            if(rising_edge && bemf_phy>neutral_voltage)
            {
                    zerocrossing_detected = true;
                     current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            else if(!rising_edge && bemf_phy<neutral_voltage)
            {
                    zerocrossing_detected = true;
                     current_ticks = __HAL_TIM_GET_AUTORELOAD(&htim2);
            }
            break;
        default:
            return;
    }


    if(zerocrossing_detected && motor_mode==2)
    {
        zerocrossing_detected = false;
        
        uint32_t new_ticks = current_ticks / 2;
        if(new_ticks<MIN_COMMUTATION_TICKS)
        {
            new_ticks = MIN_COMMUTATION_TICKS;
        }
         __HAL_TIM_SET_AUTORELOAD(&htim2, new_ticks - 1);
    }
}
/**
 * @brief Disable all motor phases and place the driver in a safe shutdown state.
 */
void motor_shutdown(void)
{
    disable_pwm_phase(0);
    disable_pwm_phase(1);
    disable_pwm_phase(2);
}

/**
 * @brief Set the BLDC commutation stage and drive two active phases.
 * @param stage Commutation stage index [0..5].
 * @param dutycycle PWM duty cycle for the active phases.
 */
void motor_commutate(uint8_t stage , float dutycycle)
{
        switch(stage)
        {
            case 0://U-V
                //W floating
                disable_pwm_phase(2);          //W phase floatingto PWM
                enable_pwm_phase(1,0.0f);     //ENV set to 1
                enable_pwm_phase(0,dutycycle);           //ENU set to 1
                floating_phase = 2;
                break;
            case 1:  //U-W
                //V floating
                disable_pwm_phase(1);          //V phase floating  
                enable_pwm_phase(2,0.0f);     //ENW set to 1
                enable_pwm_phase(0,dutycycle);           //ENU set to 1
                floating_phase = 1;
                break;
            case 2:  //V-W
                //U floating
                disable_pwm_phase(0);          //U phase floating
                enable_pwm_phase(2,0.0f);     //ENW set to 1
                enable_pwm_phase(1,dutycycle);           //ENV set to 1
                floating_phase = 0;
                break;
            case 3: //V-U
                //W floating
                disable_pwm_phase(2);          //W phase floating  
                enable_pwm_phase(0,0.0f);     //ENU set to 1
                enable_pwm_phase(1,dutycycle);           //ENV set to 1
                floating_phase = 2;
                break;
            case 4://W-U
                //V floating
                disable_pwm_phase(1);          //V phase floating 
                enable_pwm_phase(0,0.0);     //ENU set to 1
                enable_pwm_phase(2,dutycycle);           //ENW set to 1
                floating_phase = 1;
                break;
            case 5: //W-V
                //U floating
                disable_pwm_phase(0);          //U phase floating
                enable_pwm_phase(1,0.0f);     //ENV set to 1
                enable_pwm_phase(2,dutycycle);           //ENW set to 1
                floating_phase = 0;
                break;      
        }    
}


/**
 * @brief ADC conversion complete DMA callback.
 *
 * Signals the main loop that a fresh batch of ADC samples is available.
 * @param hadc Pointer to the ADC handle.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    dmadone = 1;

}

/**
 * @brief ADC half-transfer DMA callback.
 *
 * Currently unused, but preserved for future mid-buffer processing.
 * @param hadc Pointer to the ADC handle.
 */
void  HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{

}


/**
 * @brief Initialize application peripherals required by motor control.
 *
 * This stub is present for project wiring and can be extended to initialize
 * GPIOs, ADCs, timers, and any additional board peripherals.
 */
void peripherals_init(void)
{

}
/**
 * @brief Hold the motor in a fixed commutation state to align the rotor.
 */
void align_motor(void)
{
    motor_commutate(commutation_stage,0.75f);
    HAL_Delay(1000);
    commutation_stage++;
}


/**
 * @brief Power up the motor driver, start ADC capture and timer interrupts, and begin motor startup.
 */
void motor_start(void)
{
    setgpio(GPIO_PIN_4, GPIOC); // Enable Power
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_1);
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_2);
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_3);
    adc_init(&hadc1);
    align_motor();
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim1);
    __HAL_TIM_MOE_ENABLE(&htim1);
    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);

    while(1)
    {
        if(dmadone)
        {
            dmadone = 0;
            // Process ADC values here
            vbus = adc_read(ADC_CHANNEL_1);
            ntc = adc_read(ADC_CHANNEL_2);
            curr1 = adc_read(ADC_CHANNEL_8);
            curr2 = adc_read(ADC_CHANNEL_3);
            curr3 = adc_read(ADC_CHANNEL_4);
            bemf_u = adc_read(ADC_CHANNEL_6);
            bemf_w = adc_read(ADC_CHANNEL_16);
            bemf_v = adc_read(ADC_CHANNEL_15);
             process_bemf();

            // Implement control algorithm using the ADC values
        }
    }
}




/**
 * @brief Timer callback for periodic commutation and mode progression.
 * @param htim Pointer to the timer handle.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    if(htim==NULL)
    {
        return;
    }
    if(htim->Instance == TIM2)
    {
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        motor_commutate(commutation_stage,0.75f);
        commutation_stage++;
        if(commutation_stage>5)
        {
            commutation_stage = 0;
        }
        if(motor_mode == 0) // If in startup mode, accelerate
        {
            if(commutation_ticks>MIN_COMMUTATION_TICKS)
            {
               commutation_ticks -= acceleration; 
            }

            __HAL_TIM_SET_AUTORELOAD(&htim2, commutation_ticks-1);

            if(commutation_ticks<SYNC_COMMUTATION_TICKS)
            {
                motor_mode = 1; // Switch to sync mode
                sync_counter = 0;
            }
            
        }
        else if(motor_mode == 1) // If in running mode, maintain speed
        {
            if(zerocrossing_detected)
            {
                zerocrossing_detected = 0;
                sync_counter++;
            }

            if(sync_counter>=5)
            {
                sync_counter = 0;
                motor_mode = 2; // Switch to closed-loop control mode
            }
        }
        
    }
}

/**
 * @brief Timer output compare delay callback.
 *
 * Reserved for future BEMF-based synchronization logic and sensor sampling.
 * @param htim Pointer to the timer handle.
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
//to be utilised for bemf interrupt based processing later
}