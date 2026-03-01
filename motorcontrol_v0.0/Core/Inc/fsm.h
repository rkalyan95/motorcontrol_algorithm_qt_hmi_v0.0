#ifndef __FSM_H
#define __FSM_H

extern "C" {
    #include "stdint.h"
    #include "gpio.h"
}

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

/* * C++ ONLY: The C compiler will skip everything inside this block 
 */
#ifdef __cplusplus


/*Use templates maybe*/

class IPeripheral
{

    public:
        uint32_t rawbuffer;
        virtual void init(void)=0;
        virtual void uninit(void)=0;
        virtual void read(void)=0;
        virtual void write(void)=0;
};

template <typename T>
class Peripheral : public IPeripheral
{
    protected:
        T *periphraddress;
};


template <typename Pinstate = GPIO_PinState>
class Gpio : public Peripheral<GPIO_TypeDef>
{ 
    private:
        uint16_t pin;
        Pinstate pinstate;

    public:
        Gpio(GPIO_TypeDef *port,uint16_t portpin);
        void init(void)  override ;
        void uninit(void)  override ;
        void read (void) override;
        void write(void) override;
        ~Gpio();
};

template <typename channel_t = uint32_t , typename counter_reg_t = uint32_t>
class Timer : public Peripheral<TIM_HandleTypeDef>
{
    private:
        channel_t channelnumber;
    public:
        Timer(TIM_HandleTypeDef *timer, channel_t channelnumber);
        void init(void) override;
        void uninit(void) override;
        void read(void) override;
        void write(void) override;
};

template <typename channelnum_t=uint32_t , typename rawcount_t = uint16_t>
class ADC : public Peripheral<ADC_HandleTypeDef>
{
    private:
        channelnum_t channelnum;
    public:
        ADC(ADC_HandleTypeDef *adc,channelnum_t channelnum);
        void init(void) override;
        void uninit(void) override;
        void read(void) override;
        void write(void) override;

};

#endif /* __cplusplus */

#endif /* __FSM_H */