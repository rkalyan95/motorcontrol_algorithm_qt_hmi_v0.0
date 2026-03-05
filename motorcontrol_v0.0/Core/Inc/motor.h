#ifndef __MOTOR_H
#define __MOTOR_H

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

#include <Peripheral.h>

#include <array>

class Sensor
{
    private:
        IPeripheral *periph;
        float sensorgain;
        float sensoroffset;

    public:
        uint32_t sensroraw;
        float sensorphy;
        Sensor(IPeripheral *periph, float sensorgain, float sensoroffset);
        void read(void);
        void init(void);
        void write(void);
        void uninit(void);

};



class IMotorDriver {
protected:
	// --- State Variables ---
	std::array<float, 3>	pwm_phase_duty;		// Duty cycles for U, V, W
	std::array<bool, 3>		phase_bot_enable;	// Bottom switch/Enable states
	std::array<float, 3>	phase_bemf;			// Back-EMF readings
	float					motor_driver_temperature;
	float					motor_bus_voltage;
	std::array<float, 3>	current_fdbk;		// Phase currents
	bool					sensorless_control;	// Flag for control mode

	// --- Aggregated Peripherals ---
	// Using std::array to store multiple timer/gpio channels for U, V, W
	std::array<IPeripheral*, 3>	timer_periphs;	// Pointers to Phase U, V, W Timers
	std::array<IPeripheral*, 3>	gpio_periphs;	// Pointers to Phase U, V, W EN Pins
	std::array<Sensor*, 8>		generic_sensors;

public:
	virtual ~IMotorDriver() = default;

	// --- Pure Virtual Methods (The Contract) ---
	virtual void   set_pwm_duty_cycle(uint8_t phase, float duty)			= 0;
    virtual void   enable_pwm_phase(uint8_t phase)    = 0;
    virtual void   disable_pwm_phase(uint8_t phase)   = 0;
	virtual void   get_pwm_duty_cycle(uint8_t phase, float *duty)			= 0;
	
	virtual void	get_vbus(float *vbus)									= 0;
	virtual void	get_temperature(float *temperature)						= 0;
	
	virtual void	get_backemf(uint8_t phase, float *bemf)					= 0;
	virtual void	get_fdbkcurrent(uint8_t phase, float *fdbkcurrent)		= 0;

	virtual void	init()													= 0;
 
};

class IHM16M1 : public IMotorDriver {
public:
	// --- Constants ---
	// Hardcoded for 10kHz @ 4MHz Clock (400 ticks total)
	static constexpr uint32_t PWM_PERIOD = 199; 

	// --- Constructor ---
	IHM16M1();
	virtual ~IHM16M1() override;

	// --- Overridden Interface Methods ---
	void	set_pwm_duty_cycle(uint8_t phase, float duty)			override;
	void	get_pwm_duty_cycle(uint8_t phase, float *duty)			override;
	
	void	get_vbus(float *vbus)									override;
	void	get_temperature(float *temperature)						override;
	
	void	get_backemf(uint8_t phase, float *bemf)					override;
	void	get_fdbkcurrent(uint8_t phase, float *fdbkcurrent)		override;
    void    enable_pwm_phase(uint8_t phase)    override;
    void    disable_pwm_phase(uint8_t phase)   override;
	void	init()													override;
    

private:
	// Helper for internal scaling
	float	raw_to_float(uint32_t raw, float scale);
};


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

        virtual void get_motor_speed(float *rpm) override;
        virtual void get_motor_posn(float *ang) override;
        virtual void get_motor_tempe(float *temp) override;
        virtual void get_phase_emf(uint8_t phase , float *emf) override;
        virtual void get_phase_current(uint8_t phase , float *curr) override; 
        virtual void start_motor_commutation(uint8_t current_stage , float duty) override;
};

#endif

#endif /* __MOTOR_H */