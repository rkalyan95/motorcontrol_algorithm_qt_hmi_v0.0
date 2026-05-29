/**
  ******************************************************************************
  * @file    motor.h
  * @brief   Header file for Core motor initialization and system setup.
  ******************************************************************************
  */
#ifndef __MOTOR_H
#define __MOTOR_H


#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Initialize MCU peripherals and start the motor control subsystem.
  * @retval None
  */
void peripherals_init(void);

#ifdef __cplusplus
}
#endif

#endif