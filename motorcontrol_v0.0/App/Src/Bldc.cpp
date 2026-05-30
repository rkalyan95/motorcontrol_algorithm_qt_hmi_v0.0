/**
 * @file Bldc.cpp
 * @brief BLDC motor control implementation.
 * @details Contains BLDC motor control helper functions and commutation routines.
 */

extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include "stm32l4xx_hal_rcc.h"

#include <IMotordriver.h>
#include <IMotor.h>
#include <Sensor.h>


/**
 * @brief Constructs a BLDC controller instance.
 * @param motordriver Pointer to the motor driver interface.
 * @details Stores the provided H-bridge driver pointer for later control operations.
 */
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
 * @details Calculates a target torque, current, and PWM duty cycle based on the requested RPM.
 *          This function currently uses a simplified model with viscous friction and back-EMF.
 * @param target_rpm Target speed in Revolutions Per Minute.
 */
void BLDC::set_motor_speed(float target_rpm) 
{
    float backemflocal;
    if(hbridge!=nullptr)
    {
        if(target_rpm > IMotor::max_rpm) target_rpm = IMotor::max_rpm;
        float angular_speed = target_rpm*0.1047f;   //20 * .1047 = 2.094 
        float torque = this->viscous_friction * angular_speed + this->static_load_tq; // 2.094*0.00001 + 0.015 = 0.0150;
    
        if(hbridge!=nullptr)
         {
               this->torque_rqstd = torque;   //0.0150
                this->desired_I = this->torque_rqstd / this->kt_const;  //0.0150/.0367 = 0.40A
                backemflocal = angular_speed * this->ke_const;  ///2.094 * 0.0367 = 0.07V

                this->applied_voltage = this->desired_I * this->resistance + backemflocal; //0.40A*16 = 6.4V + 0.07 = 6.47V

                if(this->voltage_reference == 0.0f)
                {
                    this->voltage_reference = 12.0f;
                }

                this->calculated_duty_cycle = this->applied_voltage / this->voltage_reference;  //6.47/12 = 0.55
                if(this->calculated_duty_cycle>0.95f)
                {
                    this->calculated_duty_cycle = 0.95f;
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
        uint32_t arr = (uint32_t)this->commutation_timer_ticks;
        if(arr == 0) { *rpm = 0.0f; return; }
        uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
        uint32_t prescaler = htim2.Init.Prescaler + 1u;
        uint32_t fin = pclk1 / prescaler; /* timer input clock */
        /* Rpm = Fin * 10 / (Arr * PolePair) -- derived from Fcommut = Rpm*PolePair*0.1 */
        float rpm_calc = ((float)fin * 10.0f) / ((float)arr * (float)IMotor::polepair);
        *rpm = rpm_calc;
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

/**
 * @brief Starts commutation for the specified motor stage.
 * @param current_stage The commutation stage index (0-5).
 * @param duty PWM duty cycle to apply for the active phase.
 * @details Configures the H-bridge outputs for the selected stage and stores the floating phase.
 */
void BLDC::start_motor_commutation(uint8_t current_stage , float duty)
{
    if(hbridge!=nullptr)
    {
        this->commutation_stage = current_stage;
        /* Smooth duty to avoid abrupt steps that jerk the motor */
        static float last_comm_duty = 0.0f;
        const float duty_alpha = 0.1f; /* apply 10% of the new command each commutation (smoother) */
        float smoothed_duty = last_comm_duty + (duty - last_comm_duty) * duty_alpha;
        last_comm_duty = smoothed_duty;
        
        switch(current_stage)
        {
            case 0://U-V
                //W floating
                hbridge->disable_pwm_phase(2);          //W phase floating
                hbridge->set_pwm_duty_cycle(2,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(1);     //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,0.0);   //INV set to PWM 0%

                hbridge->enable_pwm_phase(0);           //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,smoothed_duty);   //INU set to PWM

                this->floating_phase = 2;
                break;
            case 1:  //U-W
                //V floating
                hbridge->set_pwm_duty_cycle(1, 0.0f); // Set to neutral/safe
                hbridge->disable_pwm_phase(1);          //V phase floating  
                
                hbridge->enable_pwm_phase(2);     //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,0.0);   //INW set to PWM 0%  
                hbridge->enable_pwm_phase(0);           //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,smoothed_duty);   //INVU set to PWM

                this->floating_phase = 1;
                break;
            case 2:  //V-W
                //U floating
                hbridge->disable_pwm_phase(0);          //U phase floating
                hbridge->set_pwm_duty_cycle(0,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(2);     //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,0.0);   //INW set to PWM 0%
                hbridge->enable_pwm_phase(1);           //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,smoothed_duty);   //INV set to PWM

                this->floating_phase = 0;
                break;
            case 3: //V-U
                //W floating
                hbridge->disable_pwm_phase(2);          //W phase floating  
                hbridge->set_pwm_duty_cycle(2,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(0);     //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,0.0);   //INU set to PWM 0%

                hbridge->enable_pwm_phase(1);           //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,smoothed_duty);   //INV set to PWM
                
                this->floating_phase = 2;
                break;
            case 4://W-U
                //V floating
                hbridge->disable_pwm_phase(1);          //V phase floating 
                hbridge->set_pwm_duty_cycle(1,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(0);     //ENU set to 1
                hbridge->set_pwm_duty_cycle(0,0.0);   //INU set to PWM 0%
                hbridge->enable_pwm_phase(2);           //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,smoothed_duty);   //INW set to PWM

                this->floating_phase = 1;
                break;
            case 5: //W-V
                //U floating
                hbridge->disable_pwm_phase(0);          //U phase floating
                hbridge->set_pwm_duty_cycle(0,0.0f);   //INVU set to PWM

                hbridge->enable_pwm_phase(1);     //ENV set to 1
                hbridge->set_pwm_duty_cycle(1,0.0);   //INV set to PWM 0
                hbridge->enable_pwm_phase(2);           //ENW set to 1
                hbridge->set_pwm_duty_cycle(2,smoothed_duty);   //INW set to PWM

                this->floating_phase = 0;
                break;      
        }
    }
}

/**
 * @brief Disables all PWM outputs and shuts down the motor driver.
 * @details Leaves the H-bridge in a safe disabled state for each phase.
 */
void BLDC::shutdown_all(void)
{
    hbridge->disable_pwm_phase(0);  // u switch off
    hbridge->disable_pwm_phase(1);  // v switch off
    hbridge->disable_pwm_phase(2);  // w switch off
}

/**
 * @brief Aligns the motor rotor by applying a fixed commutation pattern.
 * @details Uses a high duty cycle on the first commutation stage and holds it for 500 ms.
 */
void BLDC::align_motor(void)
{
    this->start_motor_commutation(0,0.90f);
    HAL_Delay(500);
    this->commutation_stage++;
}

/**
 * @brief Reads all motor-related sensors.
 * @details Reads bus voltage, temperature, back-EMF, and phase current feedback through the H-bridge driver.
 */
void BLDC :: read_all_sensors(void)
{
    if(hbridge!=nullptr)
    {
        this->voltage_reference  = 0.0f;
        this->back_emf = 0.0f;
        hbridge->get_vbus(&this->voltage_reference);
        hbridge->get_temperature(&this->temperature);
        hbridge->get_backemf(this->floating_phase, &this->back_emf);
        hbridge->get_fdbkcurrent(0, &hbridge->current_fdbk[0]);
        hbridge->get_fdbkcurrent(1, &hbridge->current_fdbk[1]);
        hbridge->get_fdbkcurrent(2, &hbridge->current_fdbk[2]);
    }
}

/**
 * @brief Performs open-loop motor start-up.
 * @param targetrpm Desired motor speed in RPM for the open-loop ramp.
 * @details Ramps the internal speed target while adapting the commutation timer.
 */
void BLDC::start_motor_openloop(float targetrpm)
{
    uint32_t frequencyArrValue = 0;
    
    if(hbridge!=nullptr)
    {

        if(targetrpm>IMotor::max_rpm) targetrpm = IMotor::max_rpm;
        if(targetrpm>this->rampedrpm)
        {
            this->rampedrpm+=10.0f;
        }
        else if(targetrpm<this->rampedrpm)
        {
            this->rampedrpm-=10.0f;
        }


        if(this->motorsynch<60)
        {

            //read here backemf and update sync flag if greater than 3.5v

            if(this->rampedrpm>1.0f)
            {
                if(this->back_emf>2.5f)
                {
                    this->motorsynch++;
                }
                frequencyArrValue = 5714285/this->rampedrpm;
                __HAL_TIM_SET_AUTORELOAD(&htim2, frequencyArrValue);
            }
            else
            {
                 __HAL_TIM_SET_AUTORELOAD(&htim2, 65535);
            }
        }
        
    }
}

/**
 * @brief Updates motor speed using PID control.
 * @param target_rpm Desired speed in RPM.
 * @details Placeholder for future PID speed regulation implementation.
 */
void BLDC::run_speed_pid(float target_rpm) {
    /* Simple PI controller running at commutation rate. Uses class `speed_integral` for I state. */
    float current_rpm = 0.0f;
    get_motor_speed(&current_rpm);

    const float Kp = 0.005f; /* proportional gain */
    const float Ki = 0.0001f; /* integral gain (small because call rate is high) */
    float error = target_rpm - current_rpm;

    /* integrate with simple accumulation; clamp integrator to avoid windup */
    this->speed_integral += error;
    if(this->speed_integral > 10000.0f) this->speed_integral = 10000.0f;
    if(this->speed_integral < -10000.0f) this->speed_integral = -10000.0f;

    float p_term = Kp * error;
    float i_term = Ki * this->speed_integral;

    float control = p_term + i_term;

    /* apply control as a delta to duty to raise/lower torque */
    this->calculated_duty_cycle += control;
    if(this->calculated_duty_cycle > 0.95f) this->calculated_duty_cycle = 0.95f;
    if(this->calculated_duty_cycle < 0.05f) this->calculated_duty_cycle = 0.05f;

    (void)current_rpm;
}