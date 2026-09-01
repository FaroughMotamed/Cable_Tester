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
#include "stm32g4xx_hal.h"

#include "stm32g4xx_nucleo.h"

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
#define RETEST_SW_N_Pin GPIO_PIN_13
#define RETEST_SW_N_GPIO_Port GPIOC
#define RCC_OSC_IN_Pin GPIO_PIN_0
#define RCC_OSC_IN_GPIO_Port GPIOF
#define RCC_OSC_OUT_Pin GPIO_PIN_1
#define RCC_OSC_OUT_GPIO_Port GPIOF
#define JOY_UP_N_Pin GPIO_PIN_0
#define JOY_UP_N_GPIO_Port GPIOC
#define SELECTOR_ADC_Pin GPIO_PIN_1
#define SELECTOR_ADC_GPIO_Port GPIOC
#define MUX_FB_CS_N_Pin GPIO_PIN_2
#define MUX_FB_CS_N_GPIO_Port GPIOC
#define MUX_SA_CS_N_Pin GPIO_PIN_3
#define MUX_SA_CS_N_GPIO_Port GPIOC
#define LCD_TE_Pin GPIO_PIN_0
#define LCD_TE_GPIO_Port GPIOA
#define LCD_RESET_N_Pin GPIO_PIN_1
#define LCD_RESET_N_GPIO_Port GPIOA
#define LED_FAIL_Pin GPIO_PIN_2
#define LED_FAIL_GPIO_Port GPIOA
#define LED_STATUS_Pin GPIO_PIN_3
#define LED_STATUS_GPIO_Port GPIOA
#define LED_PASS_Pin GPIO_PIN_4
#define LED_PASS_GPIO_Port GPIOA
#define MUX_A0_Pin GPIO_PIN_4
#define MUX_A0_GPIO_Port GPIOC
#define MUX_A1_Pin GPIO_PIN_5
#define MUX_A1_GPIO_Port GPIOC
#define JOY_RIGHT_N_Pin GPIO_PIN_0
#define JOY_RIGHT_N_GPIO_Port GPIOB
#define MUX_FA_CS_N_Pin GPIO_PIN_1
#define MUX_FA_CS_N_GPIO_Port GPIOB
#define MUX_WR_N_Pin GPIO_PIN_2
#define MUX_WR_N_GPIO_Port GPIOB
#define LCD_DCX_Pin GPIO_PIN_10
#define LCD_DCX_GPIO_Port GPIOB
#define ADC_CS_N_Pin GPIO_PIN_11
#define ADC_CS_N_GPIO_Port GPIOB
#define ENC_SW_N_Pin GPIO_PIN_12
#define ENC_SW_N_GPIO_Port GPIOB
#define MUX_A2_Pin GPIO_PIN_6
#define MUX_A2_GPIO_Port GPIOC
#define JOY_CENTER_N_Pin GPIO_PIN_7
#define JOY_CENTER_N_GPIO_Port GPIOC
#define MUX_A3_Pin GPIO_PIN_8
#define MUX_A3_GPIO_Port GPIOC
#define MUX_A4_Pin GPIO_PIN_9
#define MUX_A4_GPIO_Port GPIOC
#define FLASH_CS_N_Pin GPIO_PIN_8
#define FLASH_CS_N_GPIO_Port GPIOA
#define LCD_CS_N_Pin GPIO_PIN_9
#define LCD_CS_N_GPIO_Port GPIOA
#define BUZZER_EN_Pin GPIO_PIN_10
#define BUZZER_EN_GPIO_Port GPIOA
#define MUX_FB_EN_N_Pin GPIO_PIN_11
#define MUX_FB_EN_N_GPIO_Port GPIOA
#define MUX_SA_EN_N_Pin GPIO_PIN_12
#define MUX_SA_EN_N_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define MUX_SB_EN_N_Pin GPIO_PIN_15
#define MUX_SB_EN_N_GPIO_Port GPIOA
#define ADC_DRDY_N_Pin GPIO_PIN_2
#define ADC_DRDY_N_GPIO_Port GPIOD
#define ENC_A_Pin GPIO_PIN_3
#define ENC_A_GPIO_Port GPIOB
#define JOY_DOWN_N_Pin GPIO_PIN_4
#define JOY_DOWN_N_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_5
#define ENC_B_GPIO_Port GPIOB
#define JOY_LEFT_N_Pin GPIO_PIN_6
#define JOY_LEFT_N_GPIO_Port GPIOB
#define MUX_FA_EN_N_Pin GPIO_PIN_7
#define MUX_FA_EN_N_GPIO_Port GPIOB
#define MUX_SB_CS_N_Pin GPIO_PIN_8
#define MUX_SB_CS_N_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
