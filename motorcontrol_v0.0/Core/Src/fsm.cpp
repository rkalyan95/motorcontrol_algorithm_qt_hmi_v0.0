extern "C" {
    #include "gpio.h"
    //#include "usart.h"
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


Gpio :: Gpio(GPIO_TypeDef &port,uint16_t portpin) : hwport(&port),pin(portpin)
{
    /*null is being checked by constructor , but pin can be validated by a validator function*/
} 

void Gpio :: Toggle(void)
{
      HAL_GPIO_TogglePin(hwport, pin);
     
}
