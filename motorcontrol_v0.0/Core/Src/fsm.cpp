extern "C" {
    #include "gpio.h"
    //#include "usart.h"
}

#include "fsm.h"

Gpio :: Gpio(GPIO_TypeDef &port,uint16_t portpin) : hwport(&port),pin(portpin)
{
    MX_GPIO_Init();
} 

void Gpio :: Toggle(void)
{
      HAL_GPIO_TogglePin(hwport, pin);
     
}
