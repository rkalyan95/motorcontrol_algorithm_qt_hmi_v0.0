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



/*Timer implementation below as of now*/

template <typename channel_t, typename counter_reg_t>
Timer<channel_t,counter_reg_t> :: Timer(TIM_HandleTypeDef *timer, channel_t channelnumber)
{
    this->periphraddress = timer;
    this->channelnumber = channelnumber;
    this->rawbuffer = 0;

}


template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: init(void)
{
    HAL_TIM_PWM_Start(this->periphraddress, this->channelnumber);

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: uninit(void)
{

}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: read(void)
{
   this->rawbuffer = __HAL_TIM_GET_COMPARE(this->periphraddress, this->channelnumber);
}
template <typename channel_t, typename counter_reg_t>
void Timer<channel_t,counter_reg_t> :: write(void)
{
    __HAL_TIM_SET_COMPARE(this->periphraddress, this->channelnumber, this->rawbuffer);
}



/* ADC Class impleementation here below */


template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: init(void)
{
     HAL_ADC_Start(this->periphraddress);
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: uninit(void)
{
    HAL_ADC_Stop(this->periphraddress);
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: read(void)
{
    
    if (HAL_ADC_PollForConversion(this->periphraddress, 10) == HAL_OK) 
        {
           this->rawbuffer = HAL_ADC_GetValue(this->periphraddress);
        }
        else
        {
          this->rawbuffer = 0;
        }
    
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: write(void)
{

}

template <typename channelnum_t, typename rawcount_t>
ADC<channelnum_t,rawcount_t>::ADC(ADC_HandleTypeDef *adc,channelnum_t channelnum)
{
    this->periphraddress = adc;
    this->channelnum = channelnum;
    this->rawbuffer = 0;
}


template class ADC<uint32_t , uint16_t>;
template class Timer<uint32_t , uint32_t>;
template class Gpio<GPIO_PinState>;