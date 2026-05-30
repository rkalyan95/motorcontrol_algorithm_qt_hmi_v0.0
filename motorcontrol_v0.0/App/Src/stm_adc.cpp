/**
 * @file stm_adc.cpp
 * @brief ADC peripheral implementation using DMA for motor control.
 * @details Manages ADC initialization, DMA transfer handling, and raw data conversion.
 */

extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "dma.h"
    #include "tim.h"
}

#include <Peripheral.h>
static uint16_t dmalocalbuffer[8];
static uint8_t dmadone = 0;
uint8_t adcdmainterrupt;
static bool dma_is_running = false;
extern "C"
{
/**
 * @brief ADC conversion complete callback.
 * @param hadc Pointer to the ADC handle.
 */
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
    {
        if(hadc->Instance == ADC1)
        {
          adcdmainterrupt = 1;
          dmadone = 1;
                    extern void motor_adc_samples_ready_from_isr(void);
                    motor_adc_samples_ready_from_isr();
        }
    }

/**
 * @brief ADC conversion half-complete callback.
 * @param hadc Pointer to the ADC handle.
 */
    void  HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
    {
        if(hadc->Instance == ADC1)
        {
        }
    }
}

template <typename channelnum_t, typename rawcount_t>
/**
 * @brief Initializes the ADC peripheral and starts DMA transfers.
 */
void ADC<channelnum_t,rawcount_t> :: init(void)
{
    if(this->periphraddress != nullptr && !dma_is_running)
    {
       HAL_ADC_Start_DMA(this->periphraddress, (uint32_t *)dmalocalbuffer, 8);
       dmadone = 0;
       dma_is_running = true;
    }
}

template <typename channelnum_t, typename rawcount_t>
/**
 * @brief Stops the ADC DMA transfer and uninitializes the ADC peripheral.
 */
void ADC<channelnum_t,rawcount_t> :: uninit(void)
{
    if(this->periphraddress != nullptr)
    {
       HAL_ADC_Stop_DMA(this->periphraddress);
    }
}

template <typename channelnum_t, typename rawcount_t>
/**
 * @brief Reads the latest ADC value from the DMA buffer.
 */
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
            case ADC_CHANNEL_16: index = 7; break; // BEMF_W
            case ADC_CHANNEL_15: index = 6; break; // BEMF_V
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
/**
 * @brief Writes ADC state or configuration to the peripheral.
 * @details Placeholder for ADC implementations that require write operations.
 */
void ADC<channelnum_t,rawcount_t> :: write(void)
{
}

template <typename channelnum_t, typename rawcount_t>
/**
 * @brief Constructs an ADC wrapper instance.
 * @param adc HAL ADC handle pointer.
 * @param channelnum ADC channel number identifier.
 */
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

