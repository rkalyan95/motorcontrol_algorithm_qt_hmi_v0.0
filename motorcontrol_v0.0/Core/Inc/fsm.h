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

class Peripheral
{
    public:
        virtual void Init(void)=0;
        virtual void Uninit(void)=0;
        
};




class Gpio : public Peripheral
{ 
    private:
        GPIO_TypeDef *hwport;
        const uint16_t pin;
        
    public:
        Gpio();
        Gpio(GPIO_TypeDef &port,uint16_t portpin);
        void Init(void)  override ;
        void Uninit(void)  override ;
        void Set (void);
        void Reset(void);
        void Toggle(void);
        ~Gpio();
};

#endif /* __cplusplus */

#endif /* __FSM_H */