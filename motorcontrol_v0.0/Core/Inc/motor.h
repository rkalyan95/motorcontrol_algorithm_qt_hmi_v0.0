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

#include "fsm.h"

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

#endif

#endif /* __MOTOR_H */