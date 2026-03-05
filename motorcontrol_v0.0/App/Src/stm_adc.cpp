extern "C" {
    #include "gpio.h"
    
    #include "tim.h"
}

#include <Peripheral.h>

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

