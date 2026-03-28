#ifndef __IMOTORDRIVER_H
#define __IMOTORDRIVER_H

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
#include <Sensor.h>

#include <array>

class IMotorDriver {
protected:
	// --- State Variables ---


	// --- Aggregated Peripherals ---
	// Using std::array to store multiple timer/gpio channels for U, V, W
	std::array<IPeripheral*, 3>	timer_periphs;	// Pointers to Phase U, V, W Timers
	std::array<IPeripheral*, 3>	gpio_periphs;	// Pointers to Phase U, V, W EN Pins
	std::array<Sensor*, 8>		generic_sensors;

public:
	std::array<float, 3>	pwm_phase_duty;		// Duty cycles for U, V, W
	std::array<bool, 3>		phase_bot_enable;	// Bottom switch/Enable states
	std::array<float, 3>	phase_bemf;			// Back-EMF readings
	float					motor_driver_temperature;
	float					motor_bus_voltage;
	std::array<float, 3>	current_fdbk;		// Phase currents
	bool					sensorless_control;	// Flag for control mode
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

#endif

#endif /* __IMOTORDRIVER_H */