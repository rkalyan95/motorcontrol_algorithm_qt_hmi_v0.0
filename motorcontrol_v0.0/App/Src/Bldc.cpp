
extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include <IMotordriver.h>
#include <IMotor.h>
#include <Sensor.h>



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