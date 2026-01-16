extern "C" {
    #include "gpio.h"
    #include "usart.h"
}

#include "fsm.h"

void blink::init(void) {
            MX_GPIO_Init();
}


void blink::run(void)
{
      HAL_GPIO_TogglePin(led_builtin_nucleo_GPIO_Port, led_builtin_nucleo_Pin);
      HAL_Delay(500);

}