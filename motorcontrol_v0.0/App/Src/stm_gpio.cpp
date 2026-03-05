extern "C" {
    #include "gpio.h"
    
    #include <tim.h>
}
#include <Peripheral.h>

template <typename StateType>
void Gpio<StateType> :: init(void)
{
    
}

template <typename StateType>
void Gpio<StateType>  :: uninit(void)
{
    /*As of now nothing*/
}
template <typename StateType>
void Gpio<StateType>  :: write(void)
{
    
    HAL_GPIO_WritePin(this->periphraddress, this->pin, (this->rawbuffer > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
template <typename StateType>
void Gpio<StateType>  :: read(void)
{
     this->rawbuffer = HAL_GPIO_ReadPin(this->periphraddress, this->pin);
     
}

template <typename StateType>
Gpio<StateType>  :: ~Gpio()
{
     this->periphraddress = nullptr;
     
}
template <typename StateType>
Gpio<StateType>  :: Gpio(GPIO_TypeDef *port,uint16_t portpin)
{
    this->periphraddress = port;
    this->pin = portpin;
} 



template class Gpio<GPIO_PinState>;