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
/*
class Sensor
{
    private:
        Peripheral *periph;
        float sensorgain;
        float sensoroffset;
        uint32_t sensroraw;
        float sensorphy;
    public:
        Sensor(Peripheral *periph, float sensorgain, float sensoroffset);
        void read(void);
        void init(void);
        void write(void);
        void uninit(void);
        float getphy(void);
        uint32_t getraw(void);

};
*/
#endif

#endif /* __MOTOR_H */