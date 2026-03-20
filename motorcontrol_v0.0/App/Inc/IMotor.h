#ifndef __IMOTOR_H
#define __IMOTOR_H

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

#include <IMotordriver.h>

#include <array>

class IMotor 
{
    public:
        IMotorDriver *hbridge;
        static constexpr bool wdg_connection = 0;
        static constexpr float resistance = 2.5f;
        static constexpr float inductance = 0.00005f;
        static constexpr uint8_t polepair = 7;
        static constexpr float maxvoltage_rated = 12.0f;
        static constexpr float maxcurrent_rated = 15.0f;
        static constexpr float kv_rating = 110.0f;
        static constexpr float ke_const = 0.086f;
        static constexpr float kt_const = 0.086f;
        static constexpr float rotor_inertia = 0.00001f;
        static constexpr float slot_count = 12.0f;
        float current_rpm;
        float target_rpm;
        float desired_I;
        float deltaI;
        float torque_rqstd;
        float back_emf;
        float applied_voltage;
        float voltage_reference;
        static constexpr float viscous_friction = 0.00001f;
        static constexpr float static_load_tq = 0.06f;
        
        uint8_t commutation_stage;
        uint8_t floating_phase;
    public:
        virtual void shutdown_all(void) = 0;
        virtual void align_motor(void) = 0;
        virtual void set_motor_speed(float rpm) = 0;
        virtual void get_motor_speed(float *rpm) = 0;
        virtual void get_motor_posn(float *ang) = 0;
        virtual void get_motor_tempe(float *temp) = 0;
        virtual void get_phase_emf(uint8_t phase , float *emf) = 0;
        virtual void get_phase_current(uint8_t phase , float *curr)  =0; 
        virtual void start_motor_commutation(uint8_t current_stage , float duty) = 0;
        

};

class BLDC : public IMotor
{
    public:
        BLDC(IMotorDriver *motordriver);
        virtual void set_motor_speed(float rpm) override;
        virtual void shutdown_all(void) override;
        void align_motor(void) override;
        virtual void get_motor_speed(float *rpm) override;
        virtual void get_motor_posn(float *ang) override;
        virtual void get_motor_tempe(float *temp) override;
        virtual void get_phase_emf(uint8_t phase , float *emf) override;
        virtual void get_phase_current(uint8_t phase , float *curr) override; 
        virtual void start_motor_commutation(uint8_t current_stage , float duty) override;
};



#endif

#endif /* __IMOTOR_H */