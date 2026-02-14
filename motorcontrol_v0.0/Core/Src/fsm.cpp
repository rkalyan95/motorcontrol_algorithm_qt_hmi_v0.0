extern "C" {
    #include "gpio.h"
    #include "usart.h"
    #include "tim.h"
}

#include "fsm.h"

void Gpio :: Init(void)
{
    MX_GPIO_Init();
}

void Gpio :: Uninit(void)
{
    /*As of now nothing*/
}

void Gpio :: Set(void)
{
    HAL_GPIO_WritePin(hwport, pin, GPIO_PIN_SET);
}

GPIO_PinState Gpio :: Get(void)
{
    return HAL_GPIO_ReadPin(hwport, pin);
}
void Gpio :: Reset(void)
{
    HAL_GPIO_WritePin(hwport, pin, GPIO_PIN_RESET);
}

Gpio :: Gpio() :hwport(GPIOB),pin(GPIO_PIN_13)
{
}

Gpio :: ~Gpio()
{
     /*destructor does nothing as of now*/
}


Gpio :: Gpio(GPIO_TypeDef *port,uint16_t portpin) : hwport(port),pin(portpin)
{
    
} 

void Gpio :: Toggle(void)
{
      HAL_GPIO_TogglePin(hwport, pin);
     
}

Timer :: Timer()
{

}

Timer :: Timer(TIM_HandleTypeDef *htim,uint8_t channelnum) : hardwareinstance(htim),channel_num(channelnum)
{
   
}

Timer :: ~Timer()
{
    /*destructor does nothing as of now*/
}

void Timer :: Init(void)
{
    MX_TIM2_Init();
    __HAL_TIM_SET_PRESCALER(hardwareinstance, 399);
    __HAL_TIM_SET_AUTORELOAD(hardwareinstance, 9999);
    HAL_TIM_GenerateEvent(hardwareinstance, TIM_EVENTSOURCE_UPDATE);
}

void Timer :: Uninit(void)
{
    /*As of now nothing*/
}

void Timer :: PwmSetDutyCycle(float dutycycle)
{

    __HAL_TIM_SET_COMPARE(hardwareinstance, channel_num, 5000);
    
    
}

void Timer :: RunTimer(void)
{
    
    HAL_TIM_PWM_Start(hardwareinstance, channel_num);
}