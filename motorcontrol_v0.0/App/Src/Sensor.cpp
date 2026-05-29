/**
 * @file Sensor.cpp
 * @brief Sensor abstraction for reading analog peripheral values.
 * @details Converts raw ADC readings into calibrated physical sensor units.
 */

extern "C" {
    #include "gpio.h"
    #include "adc.h"
    #include "tim.h"
}

#include <Sensor.h>

/**
 * @brief Constructs a Sensor instance.
 * @param periph Underlying peripheral used for sensor readings.
 * @param sensorgain Gain factor applied to raw ADC values.
 * @param sensoroffset Offset added after gain scaling.
 */
Sensor :: Sensor(IPeripheral *periph, float sensorgain, float sensoroffset)
{
    if(periph!=nullptr && sensorgain!=0)
    {
        this->periph = periph;
        this->sensorgain = sensorgain;
        this->sensoroffset = sensoroffset;
    }
}

/**
 * @brief Initializes the sensor peripheral.
 */
void Sensor ::init(void)
{
    if(this->periph==nullptr)
    {
        return;
    }
    this->periph->init();
}

/**
 * @brief Reads the sensor value and converts raw counts to physical units.
 */
void Sensor :: read(void)
{
    if(this->periph==nullptr)
    {
        return;
    }
    this->periph->read();
    this->sensroraw = this->periph->rawbuffer;
    this->sensorphy = ((float)((this->sensroraw)*(float)(this->sensorgain)) + (float)(this->sensoroffset));
    this->sensorphy = (this->perbitphy*this->sensorphy);
}

/**
 * @brief Writes the sensor output to the underlying peripheral.
 * @details Placeholder for future bidirectional sensor implementations.
 */
void Sensor ::write(void)
{
}

/**
 * @brief Uninitializes the sensor peripheral.
 */
void Sensor ::uninit(void)
{
    this->periph->uninit();
}