extern "C" {
    #include "gpio.h"
    
    #include "tim.h"
}

#include "fsm.h"

template <typename StateType>
void Gpio<StateType> :: init(void)
{
    
}
template <typename StateType>
void Gpio<StateType>  :: setstate(StateType nextstate)
{
    this->pinstate = nextstate;
}
template <typename StateType>
StateType Gpio<StateType>  :: getstate(void)
{
    read();
    return this->pinstate;
}
template <typename StateType>
void Gpio<StateType>  :: uninit(void)
{
    /*As of now nothing*/
}
template <typename StateType>
void Gpio<StateType>  :: write(void)
{
    HAL_GPIO_WritePin(this->periphraddress, this->pin, this->pinstate);
}
template <typename StateType>
void Gpio<StateType>  :: read(void)
{
    pinstate = HAL_GPIO_ReadPin(this->periphraddress, this->pin);
}
template <typename StateType>
Gpio<StateType>  :: ~Gpio()
{
     this->periphraddress = NULL;
     this->pin = 0xFFFF;
}
template <typename StateType>
Gpio<StateType>  :: Gpio(GPIO_TypeDef *port,uint16_t portpin)
{
    this->periphraddress = port;
    this->pin = portpin;
} 
template <typename StateType>
void Gpio<StateType> ::setpin(void)
{
     this->pinstate = GPIO_PIN_SET;
     write();
}
template <typename StateType>
void Gpio<StateType> ::resetpin(void)
{
     this->pinstate = GPIO_PIN_RESET;
     write();
}


/*Timer implementation below as of now*/

template <typename channel_t, typename counter_reg_t>
Timer<channel_t,counter_reg_t> :: Timer(TIM_HandleTypeDef *timer, channel_t channelnumber)
{
    this->periphraddress = timer;
    this->channelnumber = channelnumber;
    this->dc = 0;
    this->counterregistervalue = 0;

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: setdutycycle(float *dutycycle)
{
    this->dc = *dutycycle;
    this->counterregistervalue = (uint32_t)(this->dc * (this->periphraddress->Init.Period));
    write();
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: getdutycycle(float *dutycycle)
{
    this->read();

    *dutycycle = (float)this->counterregistervalue / (float)this->periphraddress->Init.Period;
}

template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: init(void)
{
     this->dc = 0;
     this->counterregistervalue = 0;
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: uninit(void)
{

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: read(void)
{
   this->counterregistervalue = __HAL_TIM_GET_COMPARE(this->periphraddress, this->channelnumber);
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: write(void)
{
    __HAL_TIM_SET_COMPARE(this->periphraddress, this->channelnumber, this->counterregistervalue);
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: startpwm(void)
{
     HAL_TIM_PWM_Start(this->periphraddress, this->channelnumber); 
    /* To avoid compiler warning*/
     [[maybe_unused]] static bool moe = [this] ()->bool {
        __HAL_TIM_MOE_ENABLE(this->periphraddress);
        return true;
     }();
}

template class Timer<uint32_t , uint32_t>;
template class Gpio<GPIO_PinState>;