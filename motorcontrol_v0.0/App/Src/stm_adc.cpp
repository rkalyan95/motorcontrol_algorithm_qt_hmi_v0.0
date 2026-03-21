extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "dma.h"
    #include "tim.h"
}

#include <Peripheral.h>
static uint16_t dmalocalbuffer[4];
static uint8_t dmadone = 0;
uint8_t adcdmainterrupt;
static bool dma_is_running = false; // Add this global/static
extern "C"
{ 
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
    {
    /* Prevent unused argument(s) compilation warning */
    if(hadc->Instance == ADC1)
    {
      adcdmainterrupt = 1;
      dmadone = 1;
    }

    }
void  HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(hadc->Instance == ADC1)
    {
    }
}
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: init(void)
{
    if(this->periphraddress != nullptr && !dma_is_running)
    {
       // HAL_ADC_Start(this->periphraddress);
       HAL_ADC_Start_DMA(this->periphraddress, (uint32_t *)dmalocalbuffer, 4);
       dmadone = 0;
       dma_is_running = true;
    }
     
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: uninit(void)
{
    if(this->periphraddress != nullptr)
    {
       // HAL_ADC_Stop(this->periphraddress);
       HAL_ADC_Stop_DMA(this->periphraddress);
       //dmadone = 0;
    }
    
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: read(void)
{
    uint8_t index = 0;
    if(this->periphraddress != nullptr)
    {
        dmadone = 0;
        adcdmainterrupt = 0;
        switch(this->channelnum) {
            case ADC_CHANNEL_1: index = 0; break; // VBUS
            case ADC_CHANNEL_2: index = 1; break; // NTC
            case ADC_CHANNEL_8: index = 2; break; // CURR1
            case ADC_CHANNEL_3: index = 3; break; // CURR2
            case ADC_CHANNEL_4: index = 4; break; // CURR3
            case ADC_CHANNEL_6: index = 5; break; // BEMF_U
            default: return;
        }
        this->rawbuffer = dmalocalbuffer[index];
        //if (HAL_ADC_PollForConversion(this->periphraddress, 10) == HAL_OK) 
           // {
           //     this->rawbuffer = HAL_ADC_GetValue(this->periphraddress);
           // }
        //else
           // {
            //    this->rawbuffer = 0;
           // }
    }
}

template <typename channelnum_t, typename rawcount_t>
void ADC<channelnum_t,rawcount_t> :: write(void)
{
   
}

template <typename channelnum_t, typename rawcount_t>
ADC<channelnum_t,rawcount_t>::ADC(ADC_HandleTypeDef *adc,channelnum_t channelnum)
{
    if(adc!=nullptr)
    {
        this->periphraddress = adc;
        this->channelnum = channelnum;
        this->rawbuffer = 0;
    }

}



template class ADC<uint32_t , uint32_t>;

