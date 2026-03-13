extern "C" {
    #include "gpio.h"
    
    #include "tim.h"
}

#include <Peripheral.h>

/*Timer implementation below as of now*/

template <typename channel_t, typename counter_reg_t>
Timer<channel_t,counter_reg_t> :: Timer(TIM_HandleTypeDef *timer, channel_t channelnumber)
{
    if(timer!=nullptr)
    {
        this->periphraddress = timer;
        this->channelnumber = channelnumber;
        this->rawbuffer = 0;
    }

}


template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: init(void)
{
    if(this->periphraddress!=nullptr)
    {
        HAL_TIM_PWM_Start(this->periphraddress, this->channelnumber);
    }

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: uninit(void)
{

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: read(void)
{
    if(this->periphraddress!=nullptr)
    {
        this->rawbuffer = __HAL_TIM_GET_COMPARE(this->periphraddress, this->channelnumber);
    }
   
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: write(void)
{
    if(this->periphraddress!=nullptr)
    {
         __HAL_TIM_SET_COMPARE(this->periphraddress, this->channelnumber, this->rawbuffer);
    }
   
}






template class Timer<uint32_t , uint32_t>;