extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include <IMotordriver.h>
#include <Sensor.h>

Gpio EnablePhaseU(GPIOC ,GPIO_PIN_8); 
Gpio EnablePhaseV(GPIOB ,GPIO_PIN_6);
Gpio EnablePhaseW(GPIOC ,GPIO_PIN_7);

Timer PwmPhaseU(&htim1, (uint32_t)TIM_CHANNEL_1);
Timer PwmPhaseV(&htim1, (uint32_t)TIM_CHANNEL_2);
Timer PwmPhaseW(&htim1, (uint32_t)TIM_CHANNEL_3);

//configure timer 2 channel for using current reference setting
std::array<IPeripheral*, 3> motortimers
{
    &PwmPhaseU, &PwmPhaseV, &PwmPhaseW
};

std::array<IPeripheral*, 3> motorgpio
{
    &EnablePhaseU, &EnablePhaseV, &EnablePhaseW
};



ADC VBUS(&hadc1, (uint32_t)ADC_CHANNEL_1);
ADC NTC(&hadc1, (uint32_t)ADC_CHANNEL_2);
ADC CURRFDBK_2(&hadc1, (uint32_t)ADC_CHANNEL_3);
ADC CURRFDBK_3(&hadc1, (uint32_t)ADC_CHANNEL_4);
//ADC BEMF_PHA_V(&hadc1, (uint32_t)ADC_CHANNEL_5);
ADC BEMF_PHA_U(&hadc1, (uint32_t)ADC_CHANNEL_6);
//ADC BEMF_PHA_W(&hadc1, (uint32_t)ADC_CHANNEL_7);
ADC CURRFDBK_1(&hadc1, (uint32_t)ADC_CHANNEL_8);

Sensor sense_vbus(&VBUS, 16.0f, 0);
Sensor sense_ntc(&NTC, 1.0f, 0.0f);

Sensor sense_curr_fdbk1(&CURRFDBK_1, 1.0f, 0.0f);
Sensor sense_curr_fdbk2(&CURRFDBK_2, 1.0f, 0.0f);
Sensor sense_curr_fdbk3(&CURRFDBK_3, 1.0f, 0.0f);

//Sensor sense_bemf_v(&BEMF_PHA_V, 11.0f, 0.0f);
Sensor sense_bemf_u(&BEMF_PHA_U, 11.0f, 0.0f);
//Sensor sense_bemf_w(&BEMF_PHA_W, 11.0f, 0.0f);

std::array<Sensor*,8> motorsensors   //lvalues can be used here?
{
    &sense_vbus, &sense_ntc, &sense_curr_fdbk1, &sense_curr_fdbk2, &sense_curr_fdbk3, &sense_bemf_u
};

enum class sensor_id : uint8_t
{
     VBUS_SENSOR_ID = 0,
     NTC_SENSOR_ID = 1,
     CURRFDBK_1_SENSOR_ID = 2,
     CURRFDBK_2_SENSOR_ID = 3,
     CURRFDBK_3_SENSOR_ID = 4,
     BEMF_V_SENSOR_ID = 5,
     BEMF_U_SENSOR_ID = 6,
     BEMF_W_SENSOR_ID = 7
};


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
    if(gpio_periphs[phase]==nullptr)
    {
        return;
    }
    this->gpio_periphs[phase]->rawbuffer = 0x00000000;
    this->gpio_periphs[phase]->write();
}
		
void IHM16M1 :: get_pwm_duty_cycle(uint8_t phase, float *duty)			
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
            timer->init();
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
    uint8_t index = static_cast<uint8_t>(sensor_id::VBUS_SENSOR_ID);

    if(generic_sensors[index]==nullptr)
    {
        return;
    }
    this->generic_sensors[index]->init();
    this->generic_sensors[index]->read();
    this->generic_sensors[index]->uninit();
   *vbus = this->generic_sensors[index]->sensorphy;
    
}

void IHM16M1 :: get_temperature(float *temperature)						
{
    uint8_t index = static_cast<uint8_t>(sensor_id::NTC_SENSOR_ID);

    if(generic_sensors[index]==nullptr)
    {
        return;
    }   
    generic_sensors[index]->init(); 
    generic_sensors[index]->read();
    generic_sensors[index]->uninit(); 
    *temperature = generic_sensors[index]->sensorphy;
}
void IHM16M1 :: get_backemf(uint8_t phase, float *bemf)
{
     static constexpr std::array<sensor_id,3> sensoridmap
     {
        sensor_id::BEMF_U_SENSOR_ID,
        sensor_id::BEMF_V_SENSOR_ID,
        sensor_id::BEMF_W_SENSOR_ID
     };

     if(phase>2)
     {
        return;
     }

     uint8_t index = static_cast<uint8_t>(sensoridmap[phase]);

     if(generic_sensors[index]==nullptr)
     {
        return;
     }
     generic_sensors[index]->init(); 
     generic_sensors[index]->read();
     generic_sensors[index]->uninit(); 
     *bemf = generic_sensors[index]->sensorphy;


}			
void IHM16M1 :: get_fdbkcurrent(uint8_t phase, float *fdbkcurrent)
{
     static constexpr std::array<sensor_id,3> feedbackcurrmap
     {
        sensor_id::CURRFDBK_1_SENSOR_ID,
        sensor_id::CURRFDBK_2_SENSOR_ID,
        sensor_id::CURRFDBK_3_SENSOR_ID
     };

     if(phase>2)
     {
        return;
     }

     uint8_t index = static_cast<uint8_t>(feedbackcurrmap[phase]);

     if(generic_sensors[index]==nullptr)
     {
        return;
     }
     generic_sensors[index]->init();
     generic_sensors[index]->read();
     generic_sensors[index]->uninit();
     *fdbkcurrent = generic_sensors[index]->sensorphy;
}	


IHM16M1 :: ~IHM16M1()
{

}
