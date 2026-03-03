extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include "motor.h"
#include "fsm.h"


Gpio EnablePhaseU(GPIOC ,GPIO_PIN_8); 
Gpio EnablePhaseV(GPIOB ,GPIO_PIN_6);
Gpio EnablePhaseW(GPIOC ,GPIO_PIN_7);

Timer timer1(&htim1, (uint32_t)TIM_CHANNEL_1);
Timer timer2(&htim1, (uint32_t)TIM_CHANNEL_2);
Timer timer3(&htim1, (uint32_t)TIM_CHANNEL_3);

ADC adc1(&hadc1, (uint32_t)ADC_CHANNEL_1);
ADC adc2(&hadc1, (uint32_t)ADC_CHANNEL_2);
ADC adc3(&hadc1, (uint32_t)ADC_CHANNEL_3);
ADC adc4(&hadc1, (uint32_t)ADC_CHANNEL_4);
ADC adc5(&hadc1, (uint32_t)ADC_CHANNEL_5);
ADC adc6(&hadc1, (uint32_t)ADC_CHANNEL_6);
ADC adc7(&hadc1, (uint32_t)ADC_CHANNEL_7);
ADC adc8(&hadc1, (uint32_t)ADC_CHANNEL_8);

Sensor sens1(&adc1, 1, 0);
Sensor sens2(&adc2, 1, 0);
Sensor sens3(&adc3, 1, 0);
Sensor sens4(&adc4, 1, 0);
Sensor sens5(&adc5, 1, 0);
Sensor sens6(&adc6, 1, 0);
Sensor sens7(&adc7, 1, 0);

std::array<IPeripheral*, 3> motortimers
{
    &timer1, &timer2, &timer3
};

std::array<IPeripheral*, 3> motorgpio
{
    &EnablePhaseU, &EnablePhaseV, &EnablePhaseW
};

std::array<Sensor*,7> motorsensors
{
    &sens1, &sens2, &sens3, &sens4, &sens5, &sens6, &sens7
};

Sensor :: Sensor(IPeripheral *periph, float sensorgain, float sensoroffset)
{
    if(periph!=nullptr && sensorgain!=0)
    {
        this->periph = periph;
        this->sensorgain = sensorgain;
        this->sensoroffset = sensoroffset;
    }

}

void Sensor ::init(void)
{
    if(this->periph==nullptr) 
    {
        return;
    }
    this->periph->init();
}
void Sensor :: read(void)
{

    if(this->periph==nullptr) 
    {
        return;
    }
    this->periph->read();
    this->sensroraw = this->periph->rawbuffer;
    this->sensorphy = this->sensorgain * this->sensroraw + this->sensoroffset;
}
void Sensor ::write(void)
{
    
}
void Sensor ::uninit(void)
{
    this->periph->uninit();
    this->periph = nullptr;
}

/*Ihm16m1 here*/


IHM16M1 :: IHM16M1()
{
    timer_periphs = motortimers;
    gpio_periphs = motorgpio;
    generic_sensors = motorsensors;


}

void IHM16M1 :: set_pwm_duty_cycle(uint8_t phase, float duty)
{
    if(phase > timer_periphs.size() || timer_periphs[phase]==nullptr)
    {
        return;
    }
    if(duty>1.0f) { duty = 1.0f;}
    else if(duty<0.0f) {duty = 0.0f;}

    this->timer_periphs[phase]->rawbuffer = (uint32_t)(duty * PWM_PERIOD);
    this->timer_periphs[phase]->write();
    this->timer_periphs[phase]->init();
}
void IHM16M1 :: enable_pwm_phase(uint8_t phase) 
{
    if(phase>gpio_periphs.size() || gpio_periphs[phase]==nullptr)
    {
        return;
    }

    this->gpio_periphs[phase]->rawbuffer = 0x00000001;
    this->gpio_periphs[phase]->write();

}
void IHM16M1 :: disable_pwm_phase(uint8_t phase) 
{
    this->gpio_periphs[phase]->rawbuffer = 0x00000000;
    this->gpio_periphs[phase]->write();
}
		
void IHM16M1 :: get_pwm_duty_cycle(uint8_t phase, float *duty)			
{

}						
void IHM16M1 :: get_temperature(float *temperature)						
{

}
void IHM16M1 :: get_backemf(uint8_t phase, float *bemf)
{

}			
void IHM16M1 :: get_fdbkcurrent(uint8_t phase, float *fdbkcurrent)
{

}	
void IHM16M1 :: init()
{
    for(auto &timer : timer_periphs)
    {
        if(timer != nullptr)
        {
            timer->rawbuffer = 0;
            timer->write();
        }

    }
    for(auto &enablepins : motorgpio)
    {
        if(enablepins != nullptr)
        {
            enablepins->rawbuffer = 0;
            enablepins->write();
        }
    }

    for(auto &sensors : generic_sensors)
    {
        if(sensors!=nullptr)
        {
            sensors->uninit();
        }
    }
}		
void IHM16M1 :: get_vbus(float *vbus)
{
    
}



IHM16M1 :: ~IHM16M1()
{

}

