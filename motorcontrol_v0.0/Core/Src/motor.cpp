extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include "motor.h"



Gpio EnablePhaseU(GPIOC ,GPIO_PIN_8); 
Gpio EnablePhaseV(GPIOB ,GPIO_PIN_6);
Gpio EnablePhaseW(GPIOC ,GPIO_PIN_7);

Timer PwmPhaseU(&htim1, (uint32_t)TIM_CHANNEL_1);
Timer PwmPhaseV(&htim1, (uint32_t)TIM_CHANNEL_2);
Timer PwmPhaseW(&htim1, (uint32_t)TIM_CHANNEL_3);

ADC VBUS(&hadc1, (uint32_t)ADC_CHANNEL_1);
ADC NTC(&hadc1, (uint32_t)ADC_CHANNEL_2);
ADC CURRFDBK_2(&hadc1, (uint32_t)ADC_CHANNEL_3);
ADC CURRFDBK_3(&hadc1, (uint32_t)ADC_CHANNEL_4);
ADC BEMF_PHA_V(&hadc1, (uint32_t)ADC_CHANNEL_5);
ADC BEMF_PHA_U(&hadc1, (uint32_t)ADC_CHANNEL_6);
ADC BEMF_PHA_W(&hadc1, (uint32_t)ADC_CHANNEL_7);
ADC CURRFDBK_1(&hadc1, (uint32_t)ADC_CHANNEL_8);

Sensor sense_vbus(&VBUS, 1, 0);
Sensor sense_ntc(&NTC, 1, 0);

Sensor sense_curr_fdbk1(&CURRFDBK_1, 1, 0);
Sensor sense_curr_fdbk2(&CURRFDBK_2, 1, 0);
Sensor sense_curr_fdbk3(&CURRFDBK_3, 1, 0);

Sensor sense_bemf_v(&BEMF_PHA_V, 1, 0);
Sensor sense_bemf_u(&BEMF_PHA_U, 1, 0);
Sensor sense_bemf_w(&BEMF_PHA_W, 1, 0);


std::array<IPeripheral*, 3> motortimers
{
    &PwmPhaseU, &PwmPhaseV, &PwmPhaseW
};

std::array<IPeripheral*, 3> motorgpio
{
    &EnablePhaseU, &EnablePhaseV, &EnablePhaseW
};

std::array<Sensor*,8> motorsensors
{
    &sense_vbus, &sense_ntc, &sense_curr_fdbk1, &sense_curr_fdbk2, &sense_curr_fdbk3, &sense_bemf_v, &sense_bemf_u , &sense_bemf_w
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


BLDC :: BLDC(IMotorDriver *motordriver)
{
    hbridge = motordriver;
    
}
/* Vreq = Ireq * R + L * dI/dt + Vbemf*/

/**
 * @brief Sets the target motor speed.
 * @details Future Implementation: Target RPM -> Torque -> Current -> PWM.
 * @param rpm Target speed in Revolutions Per Minute.
 */
void BLDC::set_motor_speed(float target_rpm) 
{
    //assuming rpm is torque as of now 
    float calculated_dc = 0.0f;

    float angular_speed = target_rpm*0.1047197f;
    float torque = this->viscous_friction * angular_speed + this->static_load_tq;
    
    if(hbridge!=nullptr)
    {
        this->torque_rqstd = torque;
        this->desired_I = this->torque_rqstd / this->kt_const;
        hbridge->get_backemf(this->floating_phase,&(this->back_emf));
        this->back_emf = angular_speed * 0.10f;
        this->applied_voltage = this->desired_I * this->resistance + this->back_emf;
        hbridge->get_vbus(&(this->voltage_reference));
        this->voltage_reference = 12.0f;
        calculated_dc = this->applied_voltage / this->voltage_reference;
        //calculated_dc = 0.15f;
       if(calculated_dc>0.95f)
       {
        calculated_dc = 0.95f;
       }
        start_motor_commutation(this->commutation_stage,calculated_dc);
       
    }

}

/**
 * @brief Retrieves the current estimated or measured motor speed.
 * @param rpm Pointer to store the result in RPM.
 */
void BLDC::get_motor_speed(float *rpm) 
{
    if (rpm == nullptr) return;

    // TODO: Calculate RPM from BEMF zero-crossing or Hall sensors
    *rpm = 0.0f;
}

/**
 * @brief Retrieves the current rotor position.
 * @param ang Pointer to store the result in degrees (0-360) or radians.
 */
void BLDC::get_motor_posn(float *ang) 
{
    if (ang == nullptr) return;

    // TODO: Implement position estimation/tracking
    *ang = 0.0f;
}

/**
 * @brief Retrieves the motor temperature.
 * @param temp Pointer to store the result in Celsius.
 */
void BLDC::get_motor_tempe(float *temp) 
{
    if (temp == nullptr) return;

    // TODO: Read from temperature sensor via hbridge
    *temp = 0.0f;
}

/**
 * @brief Retrieves the Back-Electromotive Force (BEMF) of a specific phase.
 * @param phase Phase index (0, 1, or 2).
 * @param emf Pointer to store the result in Volts.
 */
void BLDC::get_phase_emf(uint8_t phase, float *emf) 
{
    if (emf == nullptr) return;

    // TODO: Read ADC value from BEMF divider for the specific phase
    *emf = 0.0f;
}

/**
 * @brief Retrieves the instantaneous phase current.
 * @param phase Phase index (0, 1, or 2).
 * @param curr Pointer to store the result in Amperes.
 */
void BLDC::get_phase_current(uint8_t phase, float *curr) 
{
    if (curr == nullptr) return;

    // TODO: Read shunt resistor/hall effect current sensor
    *curr = 0.0f;
}

void BLDC::start_motor_commutation(uint8_t current_stage , float duty)
{
    this->commutation_stage = current_stage;
     //implement 6 switch here A-B , whatever you wrote in notebook with 20% dutycycle
     switch(this->commutation_stage)
     {
         case 0:
             hbridge->set_pwm_duty_cycle(0,duty);   //U-V  //U top ON
             hbridge->disable_pwm_phase(0);           //U Bot off

             hbridge->set_pwm_duty_cycle(1,0.0f);   //V Top Off
             hbridge->enable_pwm_phase(1);     //V Bot ON

             hbridge->set_pwm_duty_cycle(2,0.0f);   //W Top Off
             hbridge->disable_pwm_phase(2);          //W Bot Off


             this->commutation_stage = 1;
             this->floating_phase = 2;
             break;
         case 1:
             hbridge->set_pwm_duty_cycle(0,duty);  //U-W //U top On
             hbridge->disable_pwm_phase(0);           //U Bot off

             hbridge->set_pwm_duty_cycle(1,0.0f);  //V top off
             hbridge->disable_pwm_phase(1);         // v bot off

             hbridge->set_pwm_duty_cycle(2,0.0f);  //W top off
             hbridge->enable_pwm_phase(2);        //W bot on
             this->commutation_stage = 2;
             this->floating_phase = 1;
            break;
         case 2:
             hbridge->set_pwm_duty_cycle(1,duty);  //V-W  //V top on
             hbridge->disable_pwm_phase(1);    // v bot off

             hbridge->set_pwm_duty_cycle(0,0.0f);  //V-W  //U top off
             hbridge->disable_pwm_phase(0);      // U bot off

             hbridge->set_pwm_duty_cycle(2,0.0f);  //V-W  //W top off
             hbridge->enable_pwm_phase(2);   //W bot on

            this->commutation_stage = 3;
             this->floating_phase = 0;
             break;
         case 3:
             hbridge->set_pwm_duty_cycle(1,duty);   //V-U
             hbridge->disable_pwm_phase(1);    // v bot off

             hbridge->set_pwm_duty_cycle(0,0.0f);   //V-U u  top off
             hbridge->enable_pwm_phase(0);    // u bot on

             hbridge->set_pwm_duty_cycle(2,0.0f);   //V-U W  top off
             hbridge->disable_pwm_phase(2);   // W bot off
              this->commutation_stage = 4;
              this->floating_phase = 2;
             break;
         case 4:
             hbridge->set_pwm_duty_cycle(2,duty);  //W-U   // w top on
             hbridge->disable_pwm_phase(2);    //w bot off
             
             hbridge->set_pwm_duty_cycle(1,0.0f);  //W-U   // v top off
             hbridge->disable_pwm_phase(1);   // v bot off

             hbridge->set_pwm_duty_cycle(0,0.0f);  //W-U   // u top off
             hbridge->enable_pwm_phase(0);
              this->commutation_stage = 5;
              this->floating_phase = 1;
             break;
         case 5:
             hbridge->set_pwm_duty_cycle(2,duty);  //W-V  W top on
             hbridge->disable_pwm_phase(2);  //w bot off

             hbridge->set_pwm_duty_cycle(0,0.0f);  //W-V  U top off
             hbridge->disable_pwm_phase(0);  // u bot off

             hbridge->set_pwm_duty_cycle(1,0.0f);  //W-V  V top off
             hbridge->enable_pwm_phase(1);  //v bot on
             this->commutation_stage = 0;
             this->floating_phase = 0;
             break;      
            
     }



}