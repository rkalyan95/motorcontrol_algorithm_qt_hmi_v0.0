extern "C" {
    #include "gpio.h"
    
    #include "tim.h"
}

#include "motor.h"


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
    this->periph = nullptr;
}
