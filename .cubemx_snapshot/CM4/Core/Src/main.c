/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
 //CORE_CM4,USE_HAL_DRIVER,STM32H747xx,USE_PWR_LDO_SUPPLY
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdio.h>
#include "common.h"
#include "stm_util.h"
#include "softuart.h"
#include "console.h"
#include "timer_util.h"
#include "cm4core.h"
#include "console.h"
#include "mx25lm51245g.h"
#include "fatfs_util.h"
#include "setting.h"
#include "str_util.h"
#include "external_flash.h"
#include "mki.h"
#include "mkk.h"
#include "mup.h"
#include "shared_memory.h"
#include "dhcp.h"
#include "httpServer.h"
#include "net.h"
#include "rtc.h"
#include "safety.h"
#include "mkk_data.h"
#include "uart6.h"
#include "math_util.h"
#include "net_util.h"
#include "ch378.h"
#include "speaker.h"
#include "fault.h"
#include "fault_data.h"
#include "common.h"
#include "pu.h"
#include "i2c2.h"

/* USER CODE END Includes */

/* ------------------------------------------------------------------------------
 * ARM C library: main() takes no argc/argv.
 *
 * Without this marker the linker selects the semihosting variant of the argv
 * initialisation code (argv_veneer.o -> _get_argv.o -> _sys_command_string ->
 * "BKPT 0xAB"). With no debugger attached that breakpoint escalates to a
 * HardFault and the CM4 never reaches main(): no console output and no network.
 * Under Keil it "works" only because the debugger emulates the semihosting call.
 * Defining __ARM_use_no_argv makes the linker use no_argv.o instead, so the
 * startup does not touch semihosting at all.
 * ---------------------------------------------------------------------------- */
void __ARM_use_no_argv(void) { }

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* DUAL_CORE_BOOT_SYNC_SEQUENCE: Define for dual core boot synchronization    */
/*                             demonstration code based on hardware semaphore */
/* This define is present in both CM7/CM4 projects                            */
/* To comment when developping/debugging on a single core                     */
//#define DUAL_CORE_BOOT_SYNC_SEQUENCE

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg2;

QSPI_HandleTypeDef hqspi;

SPI_HandleTypeDef hspi6;
DMA_HandleTypeDef hdma_spi6_rx;
DMA_HandleTypeDef hdma_spi6_tx;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_DMA_Init(void);
static void MX_GPIO_Init(void);
static void MX_BDMA_Init(void);
static void MX_TIM7_Init(void);
static void MX_IWDG2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_SPI6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

volatile uint32_t cm4_tick_now = 0;
volatile uint32_t cm4_tick_old = 0;
volatile uint32_t cm4_tick_50ms = 0;
static uint8_t cm4_tick_1s = 0;
static uint8_t cm4_tick_1m = 0;
static BOOL net_int_state = FALSE;


BOOL fatfs_mount = FALSE;
BOOL need_save_setting	= FALSE;

char cm7_version[32]  = {'\0'};
char mki_version[32]  = {'\0'};
char mkk1_version[32] = {'\0'};
char mkk2_version[32] = {'\0'};


void UART_flush(UART_HandleTypeDef *huart) {
  // Flush UART RX buffer if RXNE is set
  if READ_BIT(huart->Instance->ISR, UART_FLAG_RXNE) {
    SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);
  }

  // Not available on F030xx devices!
  SET_BIT(huart->Instance->RQR, UART_TXDATA_FLUSH_REQUEST);

  // Clear All Errors (if needed)
  if (READ_BIT(huart->Instance->ISR, UART_FLAG_ORE | UART_FLAG_FE | UART_FLAG_NE)) {

	__HAL_UART_CLEAR_PEFLAG(huart);
	__HAL_UART_CLEAR_OREFLAG(huart);
	__HAL_UART_CLEAR_FEFLAG(huart);
	__HAL_UART_CLEAR_NEFLAG(huart);
	__HAL_UART_CLEAR_IDLEFLAG(huart);
	__HAL_UART_CLEAR_TXFECF(huart);

  }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	UART_flush(huart);
	//fulog("Err"CRLF);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == &huart2 ) {
		HAL_UART2_RxEventCallback(Size);
	}
	else if (huart == &huart6 ) {
		HAL_UART6_RxEventCallback(Size);
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == MKI_INT_P_Pin) {
		mki_int_change();
	}
	else if (GPIO_Pin == E_INT_SPI_Pin) {
		//net_int_change();
		net_int_state = TRUE;
	}

}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart6 ) {
		HAL_UART6_TxCpltCallback();
	}

}


uint32_t timers_get_tick(void)
{
	return HAL_GetTick();
}

#include "logfile_util.h"

static char filelog_buf[MAX_STR_SIZE * 4] = {0};

//static uint32_t filelog_tick = 0;

void filelog(char * str)
{

	//filelog_tick = HAL_GetTick();
	strcat_s(filelog_buf, sizeof(filelog_buf), str);
	/*
	int need, free;

	filelog_tick = HAL_GetTickNoZero();

	need = strlen(str);
	free = (sizeof(filelog_buf) - 1) - strlen(filelog_buf);

	if (free > need) {
		strcat_s(filelog_buf, sizeof(filelog_buf), str);
		return;
	}
	logfile_add(filelog_buf);
	filelog_buf[0] = '\0';
	strcat_s(filelog_buf, sizeof(filelog_buf), str);
	*/
}


uint8_t zash_um1_old = 1;
uint8_t per_um1_old = 1;
uint8_t zash_um2_old = 1;
uint8_t per_um2_old = 1;

//Активный уровень у сигналов 0
void zash_per_um_task(void)
{
	uint8_t zash_um1_now;
	uint8_t per_um1_now;
	uint8_t zash_um2_now;
	uint8_t per_um2_now;

	zash_um1_now = PIN_GET(ZASH_UM1_P);
	if (zash_um1_now != zash_um1_old) {
		if (zash_um1_now) fault_unset(FAULT_ZASH_UM1);
		else fault_set(FAULT_ZASH_UM1);
		zash_um1_old = zash_um1_now;
	}

	per_um1_now = PIN_GET(PER_UM1_P);
	if (per_um1_now != per_um1_old) {
		if (per_um1_now) fault_unset(FAULT_PER_UM1);
		else fault_set(FAULT_PER_UM1);
		per_um1_old = per_um1_now;
	}

	zash_um2_now = PIN_GET(ZASH_UM2_P);
	if (zash_um2_now != zash_um2_old) {
		if (zash_um2_now) fault_unset(FAULT_ZASH_UM2);
		else fault_set(FAULT_ZASH_UM2);
		zash_um2_old = zash_um2_now;
	}

	per_um2_now = PIN_GET(PER_UM2_P);
	if (per_um2_now != per_um2_old) {
		if (per_um2_now) fault_unset(FAULT_PER_UM2);
		else fault_set(FAULT_PER_UM2);
		per_um2_old = per_um2_now;
	}

	//ulog_fmt("z1:%d p1:%d z2:%d p2:%d"CRLF, zash_um1_now, per_um1_now, zash_um2_now, per_um2_now);
}

void update_data_time(void)
{
	RTC_BQ32000_ReadTime(&hi2c1, &globalRTC_Time);

	date_time[0] = globalRTC_Time.year;
	date_time[1] = globalRTC_Time.month;
	date_time[2] = globalRTC_Time.day;
	date_time[3] = globalRTC_Time.hours;
	date_time[4] = globalRTC_Time.minutes;

	cm4core_send_to_CM7_cmd(CORE_CMD_RTC_UPDATE, date_time, sizeof(date_time));
}

void timers_on_1min(void)
{
	//if (http_session_tick && (HAL_DiffTick(http_session_tick) > 600000)) {
	//	http_session_tick = 0;
	//	memset(safety_web_login, 0, sizeof(safety_web_login));
	//	memset(safety_web_password, 0, sizeof(safety_web_password));
	//	ulog(CONM"net: close session"CRLF);
	//}

	//ulog_fmt("test_cmd_cnt 4=%lu 7=%lu test_snd_cnt 5=%lu 8=%lu"CRLF, test_cmd_cnt[0], test_cmd_cnt[1], test_snd_cnt[0], test_snd_cnt[1]);

	//ulog_fmt("cycles_in_mlsec=%lu"CRLF, (cycle_cnt / 60000));
	//cycle_cnt = 0;

	update_data_time();

	//ulog(CONM"TEST60"CRLF);
	external_flash_task();

	zash_per_um_task();

	//if (http_client_active_tick && (HAL_DiffTick(http_client_active_tick) > 60 * 1000)) {
	//	http_client_active_tick = 0;
	//}


	if (cm7_version[0] == '\0') {
		cm4core_send_to_CM7_cmd_only(CORE_CMD_GET_VERSION);
	}

	if (mki_version[0] == '\0') {
		mki_send_version();
		fault_set(FAULT_MKI_NOT_RECV);
	}

	if (mkk1_version[0] == '\0') {
		mkk_send_version(UART2_MUX_MKKL_1);
		fault_set(FAULT_MKK1_NOT_RECV);
	}

	if (setting.lines_count > 12) {

		if (mkk2_version[0] == '\0') {
			mkk_send_version(UART2_MUX_MKKL_2);
			fault_set(FAULT_MKK2_NOT_RECV);
		}

	}


	if (filelog_buf[0] != '\0') {

//		if (HAL_DiffTick(filelog_tick) > 60000) {
			logfile_add(filelog_buf);
			filelog_buf[0] = '\0';
//		}

	}

	//if (cm4_core_ping_tick && (HAL_DiffTick(cm4_core_ping_tick) > 120000)) {
	//	ulog_fmt(CONM"CM7 core is froze. Reboot!!! %lu %lu %lu"CRLF, cm4_core_ping_tick, HAL_GetTick(), (HAL_GetTick() - cm4_core_ping_tick) );
	//	SoftUartWaitUntilTxComplate(CONSOLE_SOFT_UART_NUMBER);
	//	NVIC_SystemReset();
	//}

	//ulog_fmt(CONM"HAL_GetTick:%lu"CRLF, HAL_GetTick());

}

//void timers_on_10sec(void)
//{
	//ulog(CONM"TEST10"CRLF);
//}

void timers_on_1sec(void)
{

	PIN_TOGGLE(LED_E10);


	HAL_IWDG_Refresh(&hiwdg2);

	DHCP_time_handler();
	httpServer_time_handler();


	net_task();


	if (need_save_setting) {
		need_save_setting = FALSE;
		save_setting(SETTING_FILE_NAME);
	}



	if (fulog_buf[0] != '\0') {
		ulog(fulog_buf);
		fulog_buf[0] = '\0';
	}

	if (cm4_tick_1m < 60) cm4_tick_1m++;
	else {
		cm4_tick_1m = 0;
		timers_on_1min();
	}
 
}



void timers_on_50ms(void)
{
	uart2_task_tx(); //МКИ МКК МУП

	uart6_task_tx(); //ПД УМ


	softuart_console_task();


	if (cm4_tick_1s < 20) cm4_tick_1s++;
	else {
		cm4_tick_1s = 0;
		timers_on_1sec();
	}


}




BOOL mx25init(void)
{
	  uint8_t mx25_res;

	  uint8_t status_reg, configuration_reg;
	  uint8_t identification_data[3];


	  //mx25_QSPI_MspInit(&hqspi);

	  memset(identification_data, 0, sizeof(identification_data));
	  mx25_res = mx25_QSPI_GetIdentification(&hqspi, identification_data);
	  ulog_fmt(CONM"mx25_QSPI_GetIdentification()=%d (MI:%d DI:[%d %d])"CRLF, mx25_res, identification_data[0], identification_data[1], identification_data[2]);


	  mx25_res = mx25_QSPI_ResetMemory(&hqspi);
	  ulog_fmt(CONM"mx25_QSPI_ResetMemory()=%d"CRLF, mx25_res);



	  status_reg = 0xFF;
	  mx25_res = mx25_QSPI_GetStatus(&hqspi, &status_reg);
	  ulog_fmt(CONM"mx25_QSPI_GetStatus()=%d (status_reg:%d)"CRLF, mx25_res, status_reg);

	  configuration_reg = 0xFF;
	  mx25_res = mx25_QSPI_GetConfiguration(&hqspi, &configuration_reg);
	  ulog_fmt(CONM"mx25_QSPI_GetConfiguration()=%d (configuration_reg:%d)"CRLF, mx25_res, configuration_reg);


	  if ((status_reg != 64) || (configuration_reg != 7)) {

		  ulog(CONM"mx25 not (status=64 and configuration=7) goto set status_reg(64)"CRLF);

		  QSPI_DummyCyclesCfg(&hqspi);

		  status_reg = 0xFF;
		  mx25_res = mx25_QSPI_GetStatus(&hqspi, &status_reg);
		  ulog_fmt(CONM"mx25_QSPI_GetStatus()=%d (status_reg:%d)"CRLF, mx25_res, status_reg);

		  configuration_reg = 0xFF;
		  mx25_res = mx25_QSPI_GetConfiguration(&hqspi, &configuration_reg);
		  ulog_fmt(CONM"mx25_QSPI_GetConfiguration()=%d (configuration_reg:%d)"CRLF, mx25_res, configuration_reg);

		  if ((status_reg != 64) || (configuration_reg != 7)) {
			  status_reg = 64;
			  configuration_reg = 7;
			  mx25_res =  mx25_QSPI_SetStatusAndConfiguration(&hqspi, status_reg, configuration_reg);
			  ulog_fmt(CONM"mx25_QSPI_SetStatusAndConfiguration()=%d"CRLF, mx25_res);
		  }



		  status_reg = 0xFF;
		  mx25_res = mx25_QSPI_GetStatus(&hqspi, &status_reg);
		  ulog_fmt(CONM"mx25_QSPI_GetStatus()=%d (status_reg:%d)"CRLF, mx25_res, status_reg);

		  configuration_reg = 0xFF;
		  mx25_res = mx25_QSPI_GetConfiguration(&hqspi, &configuration_reg);
		  ulog_fmt(CONM"mx25_QSPI_GetConfiguration()=%d (configuration_reg:%d)"CRLF, mx25_res, configuration_reg);

	  }


	  //mx25_QSPI_EnableQPImode(&hqspi);
	  //mx25_EnableQuadMode(&hqspi);



	  return (status_reg == 64) && (configuration_reg == 7);
}



//uint32_t tmp = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* The CM4 can start in two ways:
       - by hardware, because option byte BCM4 = 1 (it is enabled on this board).
         In that case the CM4 runs BEFORE the CM7 configures the PLL, i.e. at
         HSI 64 MHz, and every peripheral timing (SPI6/QSPI/I2C/TIM) would be
         computed from the wrong clock;
       - by the CM7 via RCC->GCR.BOOT_C2, which is done after SystemClock_Config()
         (and after the shared memory is initialized), so the clocks are already
         at their final values.
     Wait here until SYSCLK is switched to PLL1 - in the second case the loop
     exits immediately, in the first case it waits ~1 ms for the CM7. */
  {
	  uint32_t wait_cnt = 10000000U;
	  while (((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL1) && (wait_cnt != 0U)) {
		  wait_cnt--;
	  }
  }

  // CPU2 goes to STOP mode and waits CPU1 to initialize all the steps first
  // CPU1 will wakeup CPU2 with semaphore take and release events
  // HW semaphore Clock enable
  //__HAL_RCC_HSEM_CLK_ENABLE();
  //HAL_HSEM_ActivateNotification(HSEM_WAKEUP_CPU2_MASK);
  //HAL_PWREx_ClearPendingEvent();
  //HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  //__HAL_HSEM_CLEAR_FLAG(HSEM_WAKEUP_CPU2_MASK);

  cm4core_init();
  //cm4core_send_to_CM7_cmd_only(CORE_CMD_I_REBOOT); //я перегрузился


  /* USER CODE END 1 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /* Activate HSEM notification for Cortex-M4*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
  /*
  Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
  perform system initialization (system clock config, external memory configuration.. )
  */
  HAL_PWREx_ClearPendingEvent();
  HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  /* Clear HSEM flag */
  __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //Активировать уведомления HSEM для Cortex-M4
  //Домен D2 переходит в режим STOP (процессор Cortex-M4 находится в режиме глубокого сна),
  //ожидая завершения инициализации системы процессором Cortex-M7 (настройка системных часов, настройка внешней памяти).


  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_DMA_Init();
  MX_GPIO_Init();
  MX_BDMA_Init();
  MX_TIM7_Init();
  MX_IWDG2_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  MX_SPI6_Init();
  /* USER CODE BEGIN 2 */

  /* Раздел I2C2 (PF0 - SDA, PF1 - SCL): настройка интерфейса.
     Ошибка инициализации не останавливает программу - шина может быть
     не подключена; состояние доступно по команде консоли "I2C2". */
  (void)I2C2_Init();

  // !!! ПРИ СРАБАТЫВАНИИ IWDG ПЕРЕЗАГРУЗЯТЬСЯ ОБА ЯДРА
  // !!! /NVIC_SystemReset() ВЫЗЫВАЕТ ПЕРЕЗАГРУЗКУ ОБОИХ ЯДЕР

  /*
      * Wait for buffers to be ready by CPU1
      *
      * This should not wait and should go straight through
      */
  MX_QUADSPI_Init();
  //mx25_QSPI_MspInit(&hqspi);
  //mx25_QSPI_Erase_Chip(&hqspi);



  PIN_SET(E_RST);

  PIN_SET(LED_E10);
  set_tim_frequency2(&htim7, 9600 * 5, BASE_CM4_FREQUENCY); //18 или 19 при 240 мГц
  HAL_TIM_Base_Start_IT(&htim7);
  SoftUartInit(CONSOLE_SOFT_UART_NUMBER, CONSOL_TX_GPIO_Port, CONSOL_TX_Pin, CONSOL_RX_GPIO_Port, CONSOL_RX_Pin);
  SoftUartEnableRx(CONSOLE_SOFT_UART_NUMBER);




  ulog(" "); //Глюк
  //HAL_Delay(3000);
  ulog(CRLF);
  ulog(CONM"START PU"CRLF);
  ulog_fmt(CONM"Version: %s"CRLF, VERSION);
  cm4core_send_to_CM7_cmd_only(CORE_CMD_GET_VERSION);


  //uint32_t uid[3];
  //uid[0] = HAL_GetUIDw0();
  //uid[1] = HAL_GetUIDw1();
  //uid[2] = HAL_GetUIDw2();
  //ulog_fmt(CONM"read UID %lu %lu %lu"CRLF, uid[0], uid[1], uid[2]);



  //cm4_tick_now = 10;
  //cm7_version[0] = '\0';
  //while	(cm4_tick_now--) {
	  //if (cm4core_get_cm7_version(cm7_version, sizeof(cm7_version))) break;
  //}
  //ulog_fmt("CM7.Version: %s"CRLF, cm7_version);





  mki_send_reboot();
  HAL_Delay(100);
  mkk_send_reboot(UART2_MUX_MKKL_1);
  HAL_Delay(100);
  mkk_send_reboot(UART2_MUX_MKKL_2);
  HAL_Delay(100);


  uart2_set_mux(UART2_MUX_EXT1);

  __HAL_RCC_DMA1_CLK_ENABLE();


  rtc_init();
  update_data_time();



  ulog_fmt(CONM"BQ32000: dattetime now %s"CRLF, get_date_time_str());
  //if ((globalRTC_Time.month > 12) || (globalRTC_Time.day > 31) || (globalRTC_Time.hours > 24)) {
  //	  rtc_set_datetime_str("2026-05-06T11:05:00");
	//  ulog_fmt(CONM"BQ32000: dattetime now %s"CRLF, rtc_read_datetime_str(TRUE));
  //}

  srand(get_time_stamp());



  if (mx25init()) {

	    fatfs_mount = f_mount_cnt(&f_global_fatfs, 3);

	    //if (!fatfs_mount) {
	  	//   ulog(CONM"FatFs: f_mount() error, goto format CD"CRLF);
	  	   //fatfs_mount = f_format_and_mount(&f_global_fatfs, F_MKFS_FMT_DEFAULT);
	    //}

	    ulog_fmt(CONM"FatFs: f_mount() %s"CRLF, fatfs_mount ? "ok" : "error");




  }
  else {
	  ulog(CONM"invalid init mx25 internal flash"CRLF);
	  //default_setting();
  }


  if (fatfs_mount) {
	  load_setting(SETTING_FILE_NAME);
  } else {
	  default_setting();
	  fault_set(FAULT_FATFS_NOT_MOUNT);
  }



  ulog_fmt(CONM"setting: dhcp=%s"CRLF, bool2str(setting.dhcp));
  if (!setting.dhcp) ulog_fmt(CONM"setting: ip=%s"CRLF, ip2str(setting.ip));
  ulog_fmt(CONM"setting: address=%d"CRLF, setting.uart_local_addr);
  //ulog_fmt(CONM"setting: volume=%d"CRLF, setting.sai_volume);
  //ulog_fmt(CONM"setting: name=%s"CRLF, setting.name);
  cm4core_change_setting();


  external_flash_init();

  PIN_CLR(E_RST);
  HAL_Delay(100);
  PIN_SET(E_RST);

  net_init();




  mki_send_led_clear();
  HAL_Delay(50);

  HAL_IWDG_Refresh(&hiwdg2);
  mkk_send_relay(UART2_MUX_MKKL_1, MKK_RELAY_OFF);
  HAL_Delay(300);
  mkk_send_lin_from_setting(UART2_MUX_MKKL_1);
  HAL_Delay(300);
  mkk_send_insig_on_from_setting(UART2_MUX_MKKL_1);
  HAL_Delay(300);
  mkk_send_lin_on_from_setting(UART2_MUX_MKKL_1);
  HAL_Delay(300);
  mkk_send_lin_met(UART2_MUX_MKKL_1, setting.mkk_procent);
  HAL_Delay(300);
  mkk_send_version(UART2_MUX_MKKL_1);
  HAL_Delay(300);



  if (setting.lines_count > 12) {
	  HAL_IWDG_Refresh(&hiwdg2);
	  mkk_send_relay(UART2_MUX_MKKL_2, MKK_RELAY_OFF);
	  HAL_Delay(300);
	  mkk_send_lin_from_setting(UART2_MUX_MKKL_2);
	  HAL_Delay(300);
	  mkk_send_insig_on_from_setting(UART2_MUX_MKKL_2);
	  HAL_Delay(300);
	  mkk_send_lin_on_from_setting(UART2_MUX_MKKL_2);
	  HAL_Delay(300);
	  mkk_send_lin_met(UART2_MUX_MKKL_2, setting.mkk_procent);
	  HAL_Delay(300);
	  mkk_send_version(UART2_MUX_MKKL_2);
	  HAL_Delay(300);
  }


  /*
  HAL_IWDG_Refresh(&hiwdg2);
  mkk_send_insig_on(UART2_MUX_MKKL_1, 0xFFFF);
  HAL_Delay(300);
  mkk_send_insig_on(UART2_MUX_MKKL_2, 0xFFFF);
  HAL_Delay(300);
  mkk_send_lin_on(UART2_MUX_MKKL_1, 0xFFFF);
  HAL_Delay(300);
  mkk_send_lin_on(UART2_MUX_MKKL_2, 0xFFFF);
  HAL_Delay(300);
   */

  mki_send_one_small_led(MKI_LED_NET, LED_COLOR_GREEN);

  internal_speaker_set_mux(setting.int_spk);
  //internal_speaker_set_mux(IS_UM1_UM2);




  zash_um1_old = PIN_GET(ZASH_UM1_P);
  per_um1_old = PIN_GET(PER_UM1_P);
  zash_um2_old = PIN_GET(ZASH_UM2_P);
  per_um2_old = PIN_GET(PER_UM2_P);

  //timers_add(timers_on_1ms, 1);
  //timers_add(timers_on_10ms, 10);
  //timers_add(timers_on_50ms, 50);
  //timers_add(timers_on_250ms, 250);
  //timers_add(timers_on_1sec, 1000);
  //timers_add(timers_on_10sec, 10000);
  //timers_add(timers_on_1min, 60000);

/*
  HAL_IWDG_Refresh(&hiwdg2);
  cm4_tick_now = HAL_GetTick();
  while (HAL_DiffTick(cm4_tick_now) < 3000) {
	  net_task();
	  //if ( (getSn_SR(0) == SOCK_ESTABLISHED) && (getSn_SR(1) == SOCK_ESTABLISHED) && (getSn_SR(2) == SOCK_ESTABLISHED) && (getSn_SR(3) == SOCK_ESTABLISHED)) {
	//	  break;
	  //}
  }
*/



  cm4_tick_old = HAL_GetTick();


  mki_send_version();
  HAL_Delay(50);


  ulog(CONM"PU begin loop "CRLF);



  //cm4core_send_to_CM7_cmd_only(CORE_CMD_I_READY); //я готов

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  cm4_tick_now = HAL_GetTick();
	  if (cm4_tick_now != cm4_tick_old) {


		  if (DIFF_TICKU32(cm4_tick_50ms, cm4_tick_now) > 50) {
			  cm4_tick_50ms = cm4_tick_now;
			  timers_on_50ms();
		  }

		  cm4_tick_old = cm4_tick_now;
	  }

	  cm4core_task();

	  if (net_int_state) {
		  //ulog(CONM"net_int_state"CRLF);
		  net_int_state = FALSE;
		  net_task();
	  }

	  //E_INT_SPI все время в 1 , при прерывании падает в ноль
	  //ulog_fmt("E_INT_SPI %d"CRLF, PIN_GET(E_INT_SPI));
	  if (PIN_GET(E_INT_SPI) == 0) {
		  net_task();
	  }


	  //if (mki_int_button_flag_fast) {
	//	  mki_send_key();
	//	  mki_int_button_flag_fast = FALSE;
	 // }


	  uart2_task_rx(); //МКИ МКК МУП
	  uart6_task_rx(); //ПД УМ



  }
  /* USER CODE END 3 */
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SAI2;
  PeriphClkInitStruct.PLL3.PLL3M = 1;
  PeriphClkInitStruct.PLL3.PLL3N = 13;
  PeriphClkInitStruct.PLL3.PLL3P = 13;
  PeriphClkInitStruct.PLL3.PLL3Q = 13;
  PeriphClkInitStruct.PLL3.PLL3R = 13;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Sai23ClockSelection = RCC_SAI23CLKSOURCE_PLL3;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x307075B1;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG2_Init(void)
{

  /* USER CODE BEGIN IWDG2_Init 0 */

  /* USER CODE END IWDG2_Init 0 */

  /* USER CODE BEGIN IWDG2_Init 1 */

  /* USER CODE END IWDG2_Init 1 */
  hiwdg2.Instance = IWDG2;
  hiwdg2.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg2.Init.Window = 4095;
  hiwdg2.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG2_Init 2 */

  /* USER CODE END IWDG2_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 8;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 25;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_8_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * @brief SPI6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI6_Init(void)
{

  /* USER CODE BEGIN SPI6_Init 0 */

  /* USER CODE END SPI6_Init 0 */

  /* USER CODE BEGIN SPI6_Init 1 */

  /* USER CODE END SPI6_Init 1 */
  /* SPI6 parameter configuration*/
  hspi6.Instance = SPI6;
  hspi6.Init.Mode = SPI_MODE_MASTER;
  hspi6.Init.Direction = SPI_DIRECTION_2LINES;
  hspi6.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi6.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi6.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi6.Init.NSS = SPI_NSS_SOFT;
  hspi6.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi6.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi6.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi6.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi6.Init.CRCPolynomial = 0x0;
  hspi6.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi6.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi6.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi6.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi6.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi6.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi6.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi6.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi6.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi6.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI6_Init 2 */

  /* USER CODE END SPI6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 10;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 100;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_9B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_BDMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_BDMA_CLK_ENABLE();

  /* DMA interrupt init */
  /* BDMA_Channel0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn);
  /* BDMA_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BDMA_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(BDMA_Channel1_IRQn);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CONSOL_TX_Pin|UCN_RST_Pin|EN_SPK_Pin|VBUS_ON_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UCN_NSS_Pin|UCN_SCK_Pin|UCN_MOSI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, A0_SPK_Pin|A1_SPK_Pin|UART_S_A2_Pin|UART_S_A1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_E10_GPIO_Port, LED_E10_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, UART_SAD_A0_Pin|UART_SAD_A2_Pin|UART_SAD_A1_Pin|UART_S_A0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, UART_SAD_EN_Pin|E_CS_SPI_Pin|E_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CONSOL_RX_Pin VBUS_FAL_Pin */
  GPIO_InitStruct.Pin = CONSOL_RX_Pin|VBUS_FAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : CONSOL_TX_Pin UCN_RST_Pin EN_SPK_Pin VBUS_ON_Pin */
  GPIO_InitStruct.Pin = CONSOL_TX_Pin|UCN_RST_Pin|EN_SPK_Pin|VBUS_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : UCN_NSS_Pin UCN_SCK_Pin UCN_MOSI_Pin */
  GPIO_InitStruct.Pin = UCN_NSS_Pin|UCN_SCK_Pin|UCN_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : UCN_MISO_Pin UCN_INT_Pin */
  GPIO_InitStruct.Pin = UCN_MISO_Pin|UCN_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : A0_SPK_Pin A1_SPK_Pin UART_S_A2_Pin UART_S_A1_Pin */
  GPIO_InitStruct.Pin = A0_SPK_Pin|A1_SPK_Pin|UART_S_A2_Pin|UART_S_A1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_E10_Pin */
  GPIO_InitStruct.Pin = LED_E10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_E10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : UART_SAD_A0_Pin UART_SAD_A2_Pin UART_SAD_A1_Pin UART_S_A0_Pin */
  GPIO_InitStruct.Pin = UART_SAD_A0_Pin|UART_SAD_A2_Pin|UART_SAD_A1_Pin|UART_S_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : ZASH_UM2_P_Pin PER_UM2_P_Pin ZASH_UM1_P_Pin PER_UM1_P_Pin */
  GPIO_InitStruct.Pin = ZASH_UM2_P_Pin|PER_UM2_P_Pin|ZASH_UM1_P_Pin|PER_UM1_P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : UART_SAD_EN_Pin E_CS_SPI_Pin E_RST_Pin */
  GPIO_InitStruct.Pin = UART_SAD_EN_Pin|E_CS_SPI_Pin|E_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : MKI_INT_P_Pin */
  GPIO_InitStruct.Pin = MKI_INT_P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MKI_INT_P_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : E_INT_SPI_Pin */
  GPIO_InitStruct.Pin = E_INT_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(E_INT_SPI_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if (htim == &htim7) {
	  SoftUartHandler();
	  //HAL_GPIO_TogglePin(CONSOL_TX_GPIO_Port, CONSOL_TX_Pin);
	    //fcnt++;
  }

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
