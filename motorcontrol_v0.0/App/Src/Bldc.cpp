
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
    if(motordriver!=nullptr)
    {
        hbridge = motordriver;
    }

    
    
}
/* Vreq = Ireq * R + L * dI/dt + Vbemf*/

/**
 * @brief Sets the target motor speed.
 * @details Future Implementation: Target RPM -> Torque -> Current -> PWM.
 * @param rpm Target speed in Revolutions Per Minute.
 */
void BLDC::set_motor_speed(float target_rpm) 
{
    if(hbridge!=nullptr)
    {
        //assuming rpm is torque as of now 
        static float calculated_dc = 0.0f;

        float angular_speed = target_rpm*0.1047f;   //20 * .1047 = 2.094 
        float torque = this->viscous_friction * angular_speed + this->static_load_tq; // 2.094*0.00001 + 0.015 = 0.0150;
    
        if(hbridge!=nullptr)
         {
                this->torque_rqstd = torque;   //0.0150
                this->desired_I = this->torque_rqstd / this->kt_const;  //0.0150/.0367 = 0.40A

                //hbridge->get_backemf(this->floating_phase,&(this->back_emf));

                this->back_emf = angular_speed * this->ke_const;  ///2.094 * 0.0367 = 0.07V

                this->applied_voltage = this->desired_I * this->resistance + this->back_emf; //0.40A*16 = 6.4V + 0.07 = 6.47V

               // hbridge->get_vbus(&(this->voltage_reference));

                this->voltage_reference = 12.0f; //todo : fix it , 
                if(this->voltage_reference == 0.0f)
                {
                    this->voltage_reference = 12.0f;
                }

                calculated_dc = this->applied_voltage / this->voltage_reference;  //6.47/12 = 0.55

                //calculated_dc += 0.15f;
                if(calculated_dc>0.95f)
                {
                    calculated_dc = 0.95f;
                }
                /*DO the timer calculations here and move the commutation logic to the ISR of the timer
                
                   you see the thing is , we need to dynamically change the period at which timer fires 
                   for 1500 rpm value , the timer should commutate at 1ms , but if we take say 500rpm 
                   then the calculations would be something like the below 
                   500 * 42 = 21000 steps per minute
                   which means 350 steps per second 
                   which means approximately 3ms 
                */
                start_motor_commutation(commutation_stage,calculated_dc);
                commutation_stage++;
                if(commutation_stage==6)
                {
                    commutation_stage = 0;
                }
                          
          }
    }
}

/**
 * @brief Retrieves the current estimated or measured motor speed.
 * @param rpm Pointer to store the result in RPM.
 */
void BLDC::get_motor_speed(float *rpm) 
{
    if (rpm == nullptr) return;
    if(hbridge!=nullptr) 
    {
        // TODO: Calculate RPM from BEMF zero-crossing or Hall sensors
        *rpm = 0.0f;
    }

}

/**
 * @brief Retrieves the current rotor position.
 * @param ang Pointer to store the result in degrees (0-360) or radians.
 */
void BLDC::get_motor_posn(float *ang) 
{
    if (ang == nullptr) return;
    if(hbridge!=nullptr)
    {
        // TODO: Implement position estimation/tracking
        *ang = 0.0f;      
    }

}

/**
 * @brief Retrieves the motor temperature.
 * @param temp Pointer to store the result in Celsius.
 */
void BLDC::get_motor_tempe(float *temp) 
{
    if (temp == nullptr) return;
    if(hbridge!=nullptr)
    {
        // TODO: Read from temperature sensor via hbridge
        *temp = 0.0f;
    }

}

/**
 * @brief Retrieves the Back-Electromotive Force (BEMF) of a specific phase.
 * @param phase Phase index (0, 1, or 2).
 * @param emf Pointer to store the result in Volts.
 */
void BLDC::get_phase_emf(uint8_t phase, float *emf) 
{
    if (emf == nullptr) return;
    if(hbridge!=nullptr)
    {
         // TODO: Read ADC value from BEMF divider for the specific phase
         *emf = 0.0f;
    }

}

/**
 * @brief Retrieves the instantaneous phase current.
 * @param phase Phase index (0, 1, or 2).
 * @param curr Pointer to store the result in Amperes.
 */
void BLDC::get_phase_current(uint8_t phase, float *curr) 
{
    if (curr == nullptr) return;
    if(hbridge!=nullptr)
    {
         // TODO: Read shunt resistor/hall effect current sensor
        *curr = 0.0f;      
    }

}

void BLDC::start_motor_commutation(uint8_t current_stage , float duty)
{
    if(hbridge!=nullptr)
    {
        
        this->commutation_stage = current_stage;
        
        //implement 6 switch here A-B , whatever you wrote in notebook with 20% dutycycle
        switch(current_stage)
        {
            case 0://U-V
                //W floating
                hbridge->disable_pwm_phase(2);          //W phase floating
                hbridge->set_pwm_duty_cycle(2,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(1);     //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,0.0);   //INV set to PWM 0%

                hbridge->enable_pwm_phase(0);           //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,duty);   //INU set to PWM

                this->floating_phase = 2;
                
                break;
            case 1:  //U-W
                //V floating
                hbridge->set_pwm_duty_cycle(1, 0.0f); // Set to neutral/safe
                hbridge->disable_pwm_phase(1);          //V phase floating  
                

                
                hbridge->enable_pwm_phase(2);     //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,0.0);   //INW set to PWM 0%  
                hbridge->enable_pwm_phase(0);           //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,duty);   //INVU set to PWM

                this->floating_phase = 1;
                
                break;
            case 2:  //V-W
                //U floating
                hbridge->disable_pwm_phase(0);          //U phase floating
                hbridge->set_pwm_duty_cycle(0,0.0f);   //INVU set to PWM


                hbridge->enable_pwm_phase(2);     //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,0.0);   //INW set to PWM 0%
                hbridge->enable_pwm_phase(1);           //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,duty);   //INV set to PWM

                this->floating_phase = 0;
                
                break;
            case 3: //V-U

                //W floating
                hbridge->disable_pwm_phase(2);          //W phase floating  
                hbridge->set_pwm_duty_cycle(2,0.0f);   //INVU set to PWM


                hbridge->enable_pwm_phase(0);     //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,0.0);   //INU set to PWM 0%

                hbridge->enable_pwm_phase(1);           //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,duty);   //INV set to PWM
                
                this->floating_phase = 2;
                
                break;
            case 4://W-U
                //V floating
                hbridge->disable_pwm_phase(1);          //V phase floating 
                hbridge->set_pwm_duty_cycle(1,0.0f);   //INVU set to PWM

                
                hbridge->enable_pwm_phase(0);     //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,0.0);   //INU set to PWM 0%
                hbridge->enable_pwm_phase(2);           //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,duty);   //INW set to PWM
                
                this->floating_phase = 1;
                
                break;
            case 5: //W-V
                //U floating
                hbridge->disable_pwm_phase(0);          //U phase floating
                hbridge->set_pwm_duty_cycle(0,0.0f);   //INVU set to PWM

                
                hbridge->enable_pwm_phase(1);     //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,0.0);   //INV set to PWM 0
                hbridge->enable_pwm_phase(2);           //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,duty);   //INW set to PWM
                
                this->floating_phase = 0;
                //while(1);
                break;      
            
     }

 }


}


void BLDC::shutdown_all(void)
{
    hbridge->disable_pwm_phase(0);  // u bot on
    hbridge->disable_pwm_phase(1);  // u bot on
    hbridge->disable_pwm_phase(2);  // u bot on
    hbridge->set_pwm_duty_cycle(0,0.0f);
    hbridge->set_pwm_duty_cycle(1,0.0f);
    hbridge->set_pwm_duty_cycle(2,0.0f);
}

void BLDC::align_motor(void)
{
    this->start_motor_commutation(0,0.15f);
    HAL_Delay(500);
}