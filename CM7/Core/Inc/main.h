/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "def_util.h"

#ifndef HAL_SAI_MODULE_ENABLED
#define HAL_SAI_MODULE_ENABLED	1
#endif


#define hiwdg hiwdg1

#define MAX_UART_RX_BUFFER_SIZE		MAX_PACKET_SIZE
#define DELAY_US_CYCLE_TIME		19

//#define USE_RINGBUF	1
//#define	USE_DYNBUF	1


#define _BANK1
//#define _BANK1 __attribute__((section(".bank1")))
//#define _BANK2 __attribute__((section(".bank2")))
//#define _BANK3 __attribute__((section(".bank3")))

#define APB1_TIMER_CLOCKS 	120000000
#define APB2_TIMER_CLOCKS 	240000000

#define TIM1_CLOCK 		APB2_TIMER_CLOCKS
#define TIM8_CLOCK 		APB2_TIMER_CLOCKS
#define TIM15_CLOCK 	APB2_TIMER_CLOCKS
#define TIM16_CLOCK 	APB2_TIMER_CLOCKS
#define TIM17_CLOCK 	APB2_TIMER_CLOCKS



#define TIM2_CLOCK 		APB1_TIMER_CLOCKS
#define TIM3_CLOCK 		APB1_TIMER_CLOCKS
#define TIM4_CLOCK 		APB1_TIMER_CLOCKS
#define TIM5_CLOCK 		APB1_TIMER_CLOCKS
#define TIM6_CLOCK 		APB1_TIMER_CLOCKS
#define TIM7_CLOCK 		APB1_TIMER_CLOCKS
#define TIM9_CLOCK 		APB1_TIMER_CLOCKS
#define TIM10_CLOCK 	APB1_TIMER_CLOCKS
#define TIM11_CLOCK 	APB1_TIMER_CLOCKS
#define TIM12_CLOCK 	APB1_TIMER_CLOCKS
#define TIM13_CLOCK 	APB1_TIMER_CLOCKS
#define TIM14_CLOCK 	APB1_TIMER_CLOCKS

//#define BASE_FREQUENCY 	240000000
#define CM7_BASE_FREQUENCY	480000000

#define CONM 	"CM7."
#define CORE_NUMBER 	1

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

#define UCN_NSS_Pin GPIO_PIN_4
#define UCN_MOSI_GPIO_Port GPIOA
#define A0_SOUND_L_Pin GPIO_PIN_3
#define A0_SOUND_L_GPIO_Port GPIOE
#define A1_SOUND_L_Pin GPIO_PIN_4
#define A1_SOUND_L_GPIO_Port GPIOE
#define EN_SOUND_L_Pin GPIO_PIN_5
#define EN_SOUND_L_GPIO_Port GPIOE
#define BAT_ERR_Pin GPIO_PIN_6
#define BAT_ERR_GPIO_Port GPIOE
#define UART_P1_SOUND_EN_Pin GPIO_PIN_13
#define UART_P1_SOUND_EN_GPIO_Port GPIOC
#define A0_SOUND_R_Pin GPIO_PIN_0
#define A0_SOUND_R_GPIO_Port GPIOC
#define A1_SOUND_R_Pin GPIO_PIN_2
#define A1_SOUND_R_GPIO_Port GPIOC
#define EN_SOUND_R_Pin GPIO_PIN_3
#define EN_SOUND_R_GPIO_Port GPIOC
#define EXT1_CONT_TX_Pin GPIO_PIN_0
#define EXT1_CONT_TX_GPIO_Port GPIOA
#define ADC_MIC_Pin GPIO_PIN_1
#define ADC_MIC_GPIO_Port GPIOA
#define ADC_MCHS_Pin GPIO_PIN_2
#define ADC_MCHS_GPIO_Port GPIOA
#define DAC_DATA_Pin GPIO_PIN_11
#define DAC_DATA_GPIO_Port GPIOF
#define EXT2_CONT_RX_Pin GPIO_PIN_7
#define EXT2_CONT_RX_GPIO_Port GPIOE
#define EXT2_CONT_TX_Pin GPIO_PIN_8
#define EXT2_CONT_TX_GPIO_Port GPIOE
#define OUT_BIP_P_Pin GPIO_PIN_9
#define OUT_BIP_P_GPIO_Port GPIOE
#define DAC_BCK_Pin GPIO_PIN_12
#define DAC_BCK_GPIO_Port GPIOE
#define DAC_LRCLK_Pin GPIO_PIN_13
#define DAC_LRCLK_GPIO_Port GPIOE
#define DAC_SCK_Pin GPIO_PIN_14
#define DAC_SCK_GPIO_Port GPIOE
#define SP_ON_Pin GPIO_PIN_15
#define SP_ON_GPIO_Port GPIOE
#define DAC_MC_Pin GPIO_PIN_10
#define DAC_MC_GPIO_Port GPIOB
#define DAC_MD_Pin GPIO_PIN_15
#define DAC_MD_GPIO_Port GPIOB
#define UART_P1_SOUND__X_Pin GPIO_PIN_8
#define UART_P1_SOUND__X_GPIO_Port GPIOD
#define UART_P1_SOUND_RX_Pin GPIO_PIN_9
#define UART_P1_SOUND_RX_GPIO_Port GPIOD
#define UART_P2_SOUND__X_Pin GPIO_PIN_9
#define UART_P2_SOUND__X_GPIO_Port GPIOA
#define UART_P2_SOUND_RX_Pin GPIO_PIN_10
#define UART_P2_SOUND_RX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define EXT1_CONT_RX_Pin GPIO_PIN_11
#define EXT1_CONT_RX_GPIO_Port GPIOC
#define EXT1_SOUND_TX_Pin GPIO_PIN_12
#define EXT1_SOUND_TX_GPIO_Port GPIOC
#define UART_P2_SOUND_EN_Pin GPIO_PIN_0
#define UART_P2_SOUND_EN_GPIO_Port GPIOD
#define EXT1_SOUND_RX_Pin GPIO_PIN_2
#define EXT1_SOUND_RX_GPIO_Port GPIOD
#define OUT_MCHS_Pin GPIO_PIN_3
#define OUT_MCHS_GPIO_Port GPIOD
#define IN_MCHS_Pin GPIO_PIN_4
#define IN_MCHS_GPIO_Port GPIOD
#define CLOCK_IRQ_Pin GPIO_PIN_5
#define CLOCK_IRQ_GPIO_Port GPIOB
#define DAC_ML_Pin GPIO_PIN_9
#define DAC_ML_GPIO_Port GPIOB
#define EXT2_SOUND_RX_Pin GPIO_PIN_0
#define EXT2_SOUND_RX_GPIO_Port GPIOE
#define EXT2_SOUND_TX_Pin GPIO_PIN_1
#define EXT2_SOUND_TX_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* Пины UCN_NSS/UCN_MOSI не описаны в .ioc для ядра CM7 - держим их здесь,
   чтобы генерация CubeMX их не удаляла. */
#ifndef UCN_NSS_Pin
#define UCN_NSS_Pin GPIO_PIN_4
#endif
#ifndef UCN_MOSI_GPIO_Port
#define UCN_MOSI_GPIO_Port GPIOA
#endif

extern ADC_HandleTypeDef hadc1;

//extern I2C_HandleTypeDef hi2c1;

extern IWDG_HandleTypeDef hiwdg1;
extern QSPI_HandleTypeDef hqspi; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern SAI_HandleTypeDef hsai_BlockB2;


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim15;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

//extern uint32_t speed_test_tick;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
