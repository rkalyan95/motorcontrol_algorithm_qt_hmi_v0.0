extern "C" {
    #include "gpio.h"
    
    #include "tim.h"
}

#include "motor.h"

/*
Sensor :: Sensor(Peripheral *periph, float sensorgain, float sensoroffset)
{
    this->periph = periph;
    this->sensorgain = sensorgain;
    this->sensoroffset = sensoroffset;
}

void Sensor ::init(void)
{
    
}
void Sensor :: read(void)
{
    this->sensroraw = this->periph->getrawvalues();
}
void Sensor ::write(void)
{
    
}
void Sensor ::uninit(void)
{
    
}
float Sensor ::getphy(void)
{
    
    this->sensorphy = this->sensorgain * this->sensroraw + this->sensoroffset;
    return this->sensorphy;
}
uint32_t Sensor :: getraw(void)
{
    read();
    return this->sensroraw;
}
*/