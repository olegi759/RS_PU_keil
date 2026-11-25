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
#define BASE_CM4_FREQUENCY	240000000
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
void MX_QUADSPI_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FLASH_Q2_Pin GPIO_PIN_2
#define FLASH_Q2_GPIO_Port GPIOE
#define CONSOL_RX_Pin GPIO_PIN_14
#define CONSOL_RX_GPIO_Port GPIOC
#define CONSOL_TX_Pin GPIO_PIN_15
#define CONSOL_TX_GPIO_Port GPIOC
#define UART_S_RX_Pin GPIO_PIN_3
#define UART_S_RX_GPIO_Port GPIOA
#define UCN_NSS_Pin GPIO_PIN_4
#define UCN_NSS_GPIO_Port GPIOA
#define UCN_SCK_Pin GPIO_PIN_5
#define UCN_SCK_GPIO_Port GPIOA
#define UCN_MISO_Pin GPIO_PIN_6
#define UCN_MISO_GPIO_Port GPIOA
#define UCN_MOSI_Pin GPIO_PIN_7
#define UCN_MOSI_GPIO_Port GPIOA
#define UCN_RST_Pin GPIO_PIN_4
#define UCN_RST_GPIO_Port GPIOC
#define EN_SPK_Pin GPIO_PIN_5
#define EN_SPK_GPIO_Port GPIOC
#define A0_SPK_Pin GPIO_PIN_0
#define A0_SPK_GPIO_Port GPIOB
#define A1_SPK_Pin GPIO_PIN_1
#define A1_SPK_GPIO_Port GPIOB
#define FLASH_CLK_Pin GPIO_PIN_2
#define FLASH_CLK_GPIO_Port GPIOB
#define LED_E10_Pin GPIO_PIN_10
#define LED_E10_GPIO_Port GPIOE
#define UART_SAD_A0_Pin GPIO_PIN_10
#define UART_SAD_A0_GPIO_Port GPIOD
#define FLASH_Q0_Pin GPIO_PIN_11
#define FLASH_Q0_GPIO_Port GPIOD
#define FLASH_Q1_Pin GPIO_PIN_12
#define FLASH_Q1_GPIO_Port GPIOD
#define FLASH_Q3_Pin GPIO_PIN_13
#define FLASH_Q3_GPIO_Port GPIOD
#define UART_SAD_A2_Pin GPIO_PIN_14
#define UART_SAD_A2_GPIO_Port GPIOD
#define UART_SAD_A1_Pin GPIO_PIN_15
#define UART_SAD_A1_GPIO_Port GPIOD
#define ZASH_UM2_P_Pin GPIO_PIN_2
#define ZASH_UM2_P_GPIO_Port GPIOG
#define PER_UM2_P_Pin GPIO_PIN_3
#define PER_UM2_P_GPIO_Port GPIOG
#define ZASH_UM1_P_Pin GPIO_PIN_4
#define ZASH_UM1_P_GPIO_Port GPIOG
#define PER_UM1_P_Pin GPIO_PIN_5
#define PER_UM1_P_GPIO_Port GPIOG
#define UART_SAD_EN_Pin GPIO_PIN_7
#define UART_SAD_EN_GPIO_Port GPIOG
#define E_CS_SPI_Pin GPIO_PIN_8
#define E_CS_SPI_GPIO_Port GPIOG
#define UART_SAD_TX_Pin GPIO_PIN_6
#define UART_SAD_TX_GPIO_Port GPIOC
#define UART_SAD_RX_Pin GPIO_PIN_7
#define UART_SAD_RX_GPIO_Port GPIOC
#define VBUS_FAL_Pin GPIO_PIN_8
#define VBUS_FAL_GPIO_Port GPIOC
#define VBUS_ON_Pin GPIO_PIN_9
#define VBUS_ON_GPIO_Port GPIOC
#define UCN_INT_Pin GPIO_PIN_8
#define UCN_INT_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define MKI_INT_P_Pin GPIO_PIN_10
#define MKI_INT_P_GPIO_Port GPIOC
#define UART_S_TX_Pin GPIO_PIN_5
#define UART_S_TX_GPIO_Port GPIOD
#define UART_S_A0_Pin GPIO_PIN_7
#define UART_S_A0_GPIO_Port GPIOD
#define E_INT_SPI2_Pin GPIO_PIN_9
#define E_INT_SPI2_GPIO_Port GPIOG
#define E_CS_SPI2_Pin GPIO_PIN_10
#define E_CS_SPI2_GPIO_Port GPIOG
#define E_RST_Pin GPIO_PIN_11
#define E_RST_GPIO_Port GPIOG
#define E_MISO_SPI_Pin GPIO_PIN_12
#define E_MISO_SPI_GPIO_Port GPIOG
#define E_CLK_SPI_Pin GPIO_PIN_13
#define E_CLK_SPI_GPIO_Port GPIOG
#define E_MOSI_SPI_Pin GPIO_PIN_14
#define E_MOSI_SPI_GPIO_Port GPIOG
#define E_INT_SPI_Pin GPIO_PIN_15
#define E_INT_SPI_GPIO_Port GPIOG
#define UART_S_A2_Pin GPIO_PIN_3
#define UART_S_A2_GPIO_Port GPIOB
#define UART_S_A1_Pin GPIO_PIN_4
#define UART_S_A1_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_6
#define FLASH_CS_GPIO_Port GPIOB
#define CLOCK_SDA_Pin GPIO_PIN_7
#define CLOCK_SDA_GPIO_Port GPIOB
#define CLOCK_SCL_Pin GPIO_PIN_8
#define CLOCK_SCL_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

extern QSPI_HandleTypeDef hqspi;
extern IWDG_HandleTypeDef hiwdg2;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi6;
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart6;

#define CONM 	"CM4."
#define CORE_NUMBER 	2

extern BOOL fatfs_mount;
extern BOOL need_save_setting;

#define SETTING_FILE_NAME	"pu.cfg"

#define	HIWDG_HANDLE hiwdg2

extern char cm7_version[32];
extern char mki_version[32];
extern char mkk1_version[32];
extern char mkk2_version[32];



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
