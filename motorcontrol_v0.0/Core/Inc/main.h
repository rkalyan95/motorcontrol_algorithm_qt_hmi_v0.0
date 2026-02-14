/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define pwm_pin_pa0_Pin GPIO_PIN_0
#define pwm_pin_pa0_GPIO_Port GPIOA
#define pwm_pin_pa1_Pin GPIO_PIN_1
#define pwm_pin_pa1_GPIO_Port GPIOA
#define pwm_pin_pa2_Pin GPIO_PIN_2
#define pwm_pin_pa2_GPIO_Port GPIOA
#define led_builtin_nucleo_Pin GPIO_PIN_13
#define led_builtin_nucleo_GPIO_Port GPIOB
#define extern_button_ml_Pin GPIO_PIN_8
#define extern_button_ml_GPIO_Port GPIOC
#define usart1_tx_pa9_Pin GPIO_PIN_9
#define usart1_tx_pa9_GPIO_Port GPIOA
#define usart1_tx_pa10_Pin GPIO_PIN_10
#define usart1_tx_pa10_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
