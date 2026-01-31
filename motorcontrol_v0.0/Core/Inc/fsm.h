#ifndef __FSM_H
#define __FSM_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

/* * C++ ONLY: The C compiler will skip everything inside this block 
 */
#ifdef __cplusplus

class Gpio 
{
    private:
        GPIO_TypeDef *hwport;
        const uint16_t pin;
        
    public:
        Gpio(GPIO_TypeDef &port,uint16_t portpin);
        void Toggle(void);
};

#endif /* __cplusplus */

#endif /* __FSM_H */