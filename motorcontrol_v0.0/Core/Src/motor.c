#include "motor.h"
#include "gpio.h"
#include "adc.h"
#include "tim.h"
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

static uint16_t dmalocalbuffer[8];
static uint8_t dmadone = 0;

uint8_t floating_phase = 0;
uint8_t commutation_stage = 0;
#define PWM_PERIOD 19

uint32_t commutation_ticks = 142851; 
uint32_t acceleration = 2500;
#define MIN_COMMUTATION_TICKS 3000


static void setgpio(uint16_t pin, GPIO_TypeDef *port)
{
    if(port==NULL)
    {
        return;
        
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

}
static void cleargpio(uint16_t pin, GPIO_TypeDef *port)
{
    if(port==NULL)
    {
        return;
        
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}
static void readgpio(uint16_t pin, GPIO_TypeDef *port,uint8_t *value)
{
    if(port==NULL)
    {
        return;  
    }

    *value = HAL_GPIO_ReadPin(port, pin);
}

static void adc_init(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    HAL_ADC_Start_DMA(hadc, (uint32_t *)dmalocalbuffer, 8);
}

static void adc_uninit(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    HAL_ADC_Stop_DMA(hadc);
}

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

static void tim_init(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    if(htim==NULL)
    {
        return;
    }
    HAL_TIM_PWM_Start(htim, Channel);
}

static void tim_uninit(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    if(htim==NULL)
    {
        return;
    }
    HAL_TIM_PWM_Stop(htim, Channel);
}

static void tim_write(TIM_HandleTypeDef *htim, uint32_t Channel,uint32_t value)
{
    if(htim==NULL)
    {
        return;
    }    
    __HAL_TIM_SET_COMPARE(htim, Channel, value);
}

static void tim_read(TIM_HandleTypeDef *htim, uint32_t Channel,uint32_t *value)
{
    if(htim==NULL)
    {
        return;
    }
    *value = __HAL_TIM_GET_COMPARE(htim, Channel);
}


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

void motor_shutdown(void)
{
    disable_pwm_phase(0);
    disable_pwm_phase(1);
    disable_pwm_phase(2);
}

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


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc==NULL)
    {
        return;
    }
    dmadone = 1;

}

void  HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{

}


void peripherals_init(void)
{

}
void align_motor(void)
{
    motor_commutate(commutation_stage,0.75f);
    HAL_Delay(1000);
    commutation_stage++;
}


void motor_start(void)
{
    setgpio(GPIO_PIN_4, GPIOC); // Enable Power
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_1);
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_2);
    tim_init(&htim1, (uint32_t)TIM_CHANNEL_3);
    align_motor();
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim1);
    __HAL_TIM_MOE_ENABLE(&htim1);
    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_4);

    while(1)
    {

    }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    if(htim==NULL)
    {
        return;
    }
    if(htim->Instance == TIM2)
    {
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        motor_commutate(commutation_stage,0.5f);
        commutation_stage++;
        if(commutation_stage>5)
        {
            commutation_stage = 0;
        }

        commutation_ticks = commutation_ticks - acceleration;
        if(commutation_ticks<MIN_COMMUTATION_TICKS)
        {
            commutation_ticks = MIN_COMMUTATION_TICKS;
        }
        __HAL_TIM_SET_AUTORELOAD(&htim2, commutation_ticks);
    }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{

}