/**
 * @file stm_gpio.cpp
 * @brief GPIO peripheral implementation for motor control enable pins.
 * @details Provides GPIO initialization, read, write, and cleanup operations.
 */

extern "C" {
    #include "gpio.h"
    #include <tim.h>
}
#include <Peripheral.h>

template <typename StateType>
/**
 * @brief Initializes the GPIO peripheral.
 */
void Gpio<StateType> :: init(void)
{
}

template <typename StateType>
/**
 * @brief Uninitializes the GPIO peripheral.
 */
void Gpio<StateType>  :: uninit(void)
{
    /* As of now nothing */
}

template <typename StateType>
/**
 * @brief Writes the configured output state to the GPIO pin.
 */
void Gpio<StateType>  :: write(void)
{
    if(this->periphraddress != nullptr)
    {
        HAL_GPIO_WritePin(this->periphraddress, this->pin, (this->rawbuffer > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    
}
template <typename StateType>
/**
 * @brief Reads the current state of the GPIO pin.
 */
void Gpio<StateType>  :: read(void)
{
    if(this->periphraddress != nullptr)
    {
        this->rawbuffer = HAL_GPIO_ReadPin(this->periphraddress, this->pin);
    }
}

template <typename StateType>
/**
 * @brief Destructor for the GPIO peripheral wrapper.
 */
Gpio<StateType>  :: ~Gpio()
{
     this->periphraddress = nullptr;
}
template <typename StateType>
Gpio<StateType>  :: Gpio(GPIO_TypeDef *port,uint16_t portpin)
{
    if(port!=nullptr)
    {
        this->periphraddress = port;
        this->pin = portpin;
    }

} 



template class Gpio<GPIO_PinState>;