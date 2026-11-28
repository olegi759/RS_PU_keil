/*
 * console.c
 *
 *  Created on: 15 РјР°СЂ. 2024 Рі.
 *      Author: Rafael Boltachev
 */

#include "console.h"
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "stm_util.h"
#include "str_util.h"
#include "dictionary_util.h"
#include "common.h"
#include "softuart.h"
#include "cm4core.h"
#include "fatfs_util.h"
#include "txt_file_util.h"
#include "shared_memory.h"
#include "mki.h"
#include "mx25lm51245g.h"
#include "setting.h"
#include "ch378.h"
#include "external_flash.h"
#include "uart2.h"
#include "mkk.h"
#include "i2c2.h"
#include "pd_i2c.h"

//static char console_buf[MAX_STR_SIZE] = {'\0'};

void ulog(char *str)
{
/*
	strcat_s(console_buf, sizeof(console_buf), str);

	if (!SoftUartGetTxNComplated(CONSOLE_SOFT_UART_NUMBER)) { //TX Р·Р°РЅСЏС‚
		if (console_buf[0] != '\0') {
			SoftUartPuts(CONSOLE_SOFT_UART_NUMBER, (uint8_t *)console_buf, strlen(console_buf));
			console_buf[0] = '\0';
		}

	}
*/
	uint16_t len;

	if (str == NULL) return;
	len = (uint16_t)strlen(str);
	if (len == 0) return;

	/* Non blocking log: bytes are appended to the TX ring buffer and are
	   shifted out by the TIM7 interrupt. The caller never waits for the
	   previous line, no data is lost on overflow (counted in TxDropped). */
	SoftUartPutsRing(CONSOLE_SOFT_UART_NUMBER, (uint8_t *)str, len);
}



void clear_clrf(char * data, uint16_t data_len)
{
	char * ptr;

	data[data_len] = '\0';
	ptr = strstr_s((char *)data, data_len, "\r");
	if (ptr) *ptr = '\0';
	ptr = strstr_s((char *)data, data_len, "\n");
	if (ptr) *ptr = '\0';
}



void console_replay_bool_cmd(BOOL value, const char * cmd)
{

	ulog_fmt(CONM"%s_%s"CRLF, (value ? STR_OK : STR_ERROR), cmd);
}



/* ===================== LOG SPEED TEST (LOGTEST n) ==========================
 * Measures how long the firmware is blocked inside ulog() while it pushes
 * N lines of a fixed length. This is the application-visible log cost: with
 * the blocking soft-UART log it equals the wire time of the previous line,
 * with a buffered log it must be near zero.
 * Test code is identical for the old and the new log implementation.
 * ========================================================================= */
#define LOGTEST_LINE_LEN		64		/* bytes per line, CR LF included */
#define LOGTEST_MAX_N			60		/* keep the burst below the IWDG limit */

static char logtest_line[LOGTEST_LINE_LEN + 1];

void console_logtest(int n)
{
	int i;
	uint32_t c0, c1, t0, t1, us;
	BOOL dwt_ok;

	if (n < 1) n = 1;
	if (n > LOGTEST_MAX_N) n = LOGTEST_MAX_N;

	/* fixed length line: "LOGTEST " + 3 digits + space + 'L' * k + CR LF */
	memset(logtest_line, 'L', LOGTEST_LINE_LEN);
	memcpy(logtest_line, "LOGTEST ", 8);
	logtest_line[8]  = '0' + (n / 100) % 10;
	logtest_line[9]  = '0' + (n / 10) % 10;
	logtest_line[10] = '0' + n % 10;
	logtest_line[11] = ' ';
	logtest_line[LOGTEST_LINE_LEN - 2] = '\r';
	logtest_line[LOGTEST_LINE_LEN - 1] = '\n';
	logtest_line[LOGTEST_LINE_LEN] = 0;

	/* DWT cycle counter: core debug unit, no peripheral init required */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	dwt_ok = (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) != 0;

	t0 = HAL_GetTick();
	c0 = DWT->CYCCNT;

	for (i = 0; i < n; i++) {
		logtest_line[9]  = '0' + (i / 10) % 10;
		logtest_line[10] = '0' + i % 10;
		ulog(logtest_line);
		if ((i & 7) == 7) HAL_IWDG_Refresh(&hiwdg2);	/* burst can be long */
	}

	c1 = DWT->CYCCNT;
	t1 = HAL_GetTick();

	us = dwt_ok ? (c1 - c0) / (BASE_CM4_FREQUENCY / 1000000u) : (t1 - t0) * 1000u;

	ulog_fmt(CONM"LOGTEST_RESULT n=%d bytes=%lu dwt=%d us=%lu ms=%lu us_per_line=%lu lines_per_sec=%lu"CRLF,
			n,
			(unsigned long)((uint32_t)n * LOGTEST_LINE_LEN),
			(int)dwt_ok,
			(unsigned long)us,
			(unsigned long)(t1 - t0),
			(unsigned long)(us / (uint32_t)n),
			(unsigned long)(us ? ((uint32_t)n * 1000000ull / us) : 0));
}



void console_rcv(char * msg)
{

	char * ptr, * ptr2;
	//size_t strlen(msg);
	uint8_t buf[MAX_PACKET_SIZE];
	int rc;
	int sz;
	int i;
	//uint16_t cmd_data_len;


	clear_clrf(msg, strlen(msg));


	if (strequals(msg, STR_REBOOT)) {
		//cm4core_send_to_CM7_cmd_only(CORE_CMD_REBOOT);
		//if ( cm4core_wait_cmd_from_CM7(CORE_CMD_CONSOLE_SND, buf, sizeof(buf), 1000, &cmd_data_len) ) {
		//	buf[cmd_data_len] = 0;
		//	ulog((char *)buf);
		//}

		console_replay_bool_cmd(TRUE, STR_REBOOT);
		SoftUartWaitUntilTxComplate(CONSOLE_SOFT_UART_NUMBER);
		NVIC_SystemReset();
	}
	if (strequals(msg, STR_REBOOT"_CM4")) {
		console_replay_bool_cmd(TRUE, STR_REBOOT);
		SoftUartWaitUntilTxComplate(CONSOLE_SOFT_UART_NUMBER);
		NVIC_SystemReset();
	}
	else if (strequals(msg, STR_HELP)) {
		console_replay_bool_cmd(TRUE, STR_HELP);
		ulog(STR_REBOOT""CRLF);
		ulog(STR_LIST""CRLF);
		ulog(STR_FORMAT""CRLF);
		ulog(STR_DELETE""CRLF);
		ulog(STR_RENAME""CRLF);
		ulog(STR_PRINT""CRLF);
		ulog(STR_CFGADD""CRLF);
		ulog(STR_CFGDEL""CRLF);
		ulog(STR_VERSION""CRLF);
		ulog(STR_KEY""CRLF);
	}


	// -------------------------- INTERNAL FLASH BEGIN -----------------------
	else if (strequals(msg, STR_LIST)) {
		f_print_dir(F_DEFAULT_PATH);
		console_replay_bool_cmd(TRUE, STR_LIST);
	}
	else if (strequals(msg, STR_FORMAT)) {
		fatfs_mount = f_format_and_mount(&f_global_fatfs, F_MKFS_FMT_DEFAULT);
		console_replay_bool_cmd(fatfs_mount, STR_FORMAT);
	}
	else if (strcmp_s(msg, strlen(msg), STR_DELETE)) {
		console_replay_bool_cmd((f_unlink(msg + strlen(STR_DELETE) + 1) == FR_OK), STR_DELETE);
	}
	else if (strcmp_s(msg, strlen(msg), STR_PRINT)) {
		console_replay_bool_cmd(txt_file_print(msg + strlen(STR_PRINT) + 1), STR_PRINT);
	}
	else if (strcmp_s(msg, strlen(msg), STR_RENAME)) {
		ptr = msg + strlen(STR_RENAME) + 1;
		ptr2 = strstr_s(ptr, strlen(ptr), " ");
		if (!ptr2) {
			ulog(STR_ERROR_CREATE(STR_RENAME)CRLF);
			return;
		}

		*ptr2 = '\0';
		ptr2++;

		console_replay_bool_cmd((f_rename(ptr, ptr2) == FR_OK), STR_RENAME);
	}
	else if (strcmp_s(msg, strlen(msg), STR_CFGADD)) {
		console_replay_bool_cmd(txt_file_add_str(SETTING_FILE_NAME, msg + strlen(STR_CFGADD) + 1), STR_CFGADD);
	}
	else if (strcmp_s(msg, strlen(msg), STR_CFGDEL)) {
		console_replay_bool_cmd(txt_file_remove_by_start_of_line(SETTING_FILE_NAME, msg + strlen(STR_CFGDEL) + 1), STR_CFGDEL);
	}
	else if (strcmp_s(msg, strlen(msg), STR_COPY)) {
		ptr = msg + strlen(STR_COPY) + 1;
		console_replay_bool_cmd(external_flash_copy_from_local(ptr), STR_COPY);
	}
	else if (strcmp_s(msg, strlen(msg), "ERASE_FLASH")) {

		HAL_IWDG_Refresh(&hiwdg2);
		rc = mx25_QSPI_Erase_Chip(&hqspi);
		HAL_IWDG_Refresh(&hiwdg2);

		ulog_fmt(CONM"OK_ERASE_FLASH %d"CRLF, rc);
	}
	else if (strequals(msg, STR_FAULTS)) {
			for (rc = 0; rc < 128; rc++) {
				if (fault_is_set(rc)) {
					ulog(int2str(rc));
					ulog(" ");
				}
			}
			ulog(STR_OK_CREATE(STR_FAULTS)CRLF);
		}

	// -------------------------- INTERNAL FLASH END -----------------------

	// -----------------  external flash begin ---------------------------------------

	else if (strequals(msg, "E"STR_LIST)) {
		external_flash_enum_files(CH378_ENUM_ROOT_PATH, external_flash_get_file_list_item_func_log, NULL);
		ulog(STR_OK_CREATE("E"STR_LIST)CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "E"STR_PRINT)) {
		ptr = msg + strlen("E"STR_PRINT) + 1;
		if (!external_flash_open_file(ptr)) {
			ulog(STR_ERROR_CREATE("E"STR_PRINT)CRLF);
			return;
		}
		sz = 0;
		while (TRUE) {
			rc = external_flash_read_file(buf, 250);
			if (!rc) break;
			sz += rc;
			buf[rc] = '\0';
			ulog((char *)buf);
			HAL_IWDG_Refresh(&hiwdg2);
		}

		external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
		ulog(CRLF);
		ulog(STR_OK_CREATE("E"STR_PRINT)CRLF);
		ulog_fmt("File Read Bytes Count = %lu"CRLF, (unsigned long)sz);
	}

	else if (strcmp_s(msg, strlen(msg), "E"STR_SIZE)) {
		ptr = msg + strlen("E"STR_SIZE) + 1;
		if (!external_flash_open_file(ptr)) {
			ulog(STR_ERROR_CREATE("E"STR_SIZE)CRLF);
			return;
		}
		sz = 0;
		while (TRUE) {
			rc = external_flash_read_file((uint8_t *)buf, 250);
			if (!rc) break;
			sz += rc;
			HAL_IWDG_Refresh(&hiwdg2);
		}

		external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
		ulog(CRLF);
		ulog(STR_OK_CREATE("E"STR_SIZE)CRLF);
		ulog_fmt("File Read Bytes Count = %lu"CRLF, (unsigned long)sz);
	}
	else if (strcmp_s(msg, strlen(msg), "E"STR_COPY)) {
		ptr = msg + strlen("E"STR_COPY) + 1;
		console_replay_bool_cmd( external_flash_copy_to_local(ptr), "E"STR_COPY );
	}

	else if (strcmp_s(msg, strlen(msg), "E"STR_DELETE)) {
		ptr = msg + strlen("E"STR_DELETE) + 1;
		external_flash_file_delete(ptr);
		ulog(STR_OK_CREATE("E"STR_DELETE)CRLF);
	}


	// -----------------  external flash end ---------------------------------------



	else if (strequals(msg, STR_VERSION)) {
		ulog_fmt(CONM"OK_VERSION %s"CRLF, VERSION);
		cm4core_send_to_CM7_cmd(CORE_CMD_CONSOLE_RCV, (uint8_t *)msg, strlen(msg));
  	    mki_send_version();
  	    HAL_Delay(50);
		mkk_send_version(UART2_MUX_MKKL_1);
  	    HAL_Delay(50);
		mkk_send_version(UART2_MUX_MKKL_2);
	}
	else if (strcmp_s(msg, strlen(msg), "LED ")) {
		ptr = msg + strlen("LED ");
		ptr2 = strright(ptr, strlen(ptr), " ");
		mki_send_one_small_led(str2int(ptr), (ptr2 ? str2int(ptr2) : 0));
		ulog(CONM""STR_OK_CREATE("LED")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "LEDS ")) {
		ptr = msg + strlen("LEDS ");
		rc = str2int(ptr);
		for (i = MKI_LED_1; i <= MKI_LED_MANUALSTART; i++) {
			mki_set_small_led(i, rc);
		}
		mki_send_small_leds();
		ulog(CONM""STR_OK_CREATE("LEDS")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "LOGTEST ")) {
		console_logtest(str2int(msg + strlen("LOGTEST ")));
	}
	else if (strequals(msg, "LOAD_SETTING")) {
		load_setting(SETTING_FILE_NAME);
		ulog(CONM""STR_OK_CREATE("LOAD_SETTING")CRLF);

	}
	else if (strequals(msg, "SAVE_SETTING")) {
		save_setting(SETTING_FILE_NAME);
		ulog(CONM""STR_OK_CREATE("SAVE_SETTING")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "I2C2")) {
		I2C2_Console(msg);
	}
	else if (strcmp_s(msg, strlen(msg), "PDI2C")) {
		/* PD-протокол по I2C2: PING / SEND / STRESS / INFO */
		PD_I2C_Console(msg);
	}


	else {
		cm4core_send_to_CM7_cmd(CORE_CMD_CONSOLE_RCV, (uint8_t *)msg, strlen(msg)); //РґСЂСѓРіРёРµ РєРѕРјРјР°РЅРґС‹ РѕС‚РїСЂР°РІР»СЏРµРј СЏРґСЂСѓ РЎРњ7
	}

/*
	else if (strcmp_s(msg, strlen(msg), STR_SEND)) {
		ptr = msg + strlen(STR_SEND) + 1;
		ptr2 = strstr_s(ptr, strlen(ptr), " ");
		if (ptr2) {
			*ptr2 = '\0';
			ptr2++;
		}
		console_replay_bool( sendopus_start(ptr, FALSE, ptr2) );
	}
	else if (strcmp_s(msg, strlen(msg), STR_PLAY)) {
		ptr = msg + strlen(STR_PLAY) + 1;
		ptr2 = strstr_s(ptr, strlen(ptr), " ");
		if (ptr2) {
			*ptr2 = '\0';
			ptr2++;
		}
		console_replay_bool( playopus_start(ptr, (ptr2 ? str2int(ptr2) : 100), FALSE) );
	}
	//else if (strcmp_s(msg, strlen(msg), STR_CRC)) {
	//	ulog(ull2str(f_crc(msg + strlen(STR_CRC) + 1)));
	//}
	else if (strcmp_s(msg, strlen(msg), STR_COPY)) {
		ptr = msg + strlen(STR_COPY) + 1;
		console_replay_bool( external_flash_copy_from_local(ptr) );
	}






	else if (strcmp_s(msg, strlen(msg), "MUP")) {
		mup_send_query();
		ulog(STR_OK_CREATE("MUP")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "MIC ")) {
		ptr = msg + strlen("MIC ");
		if (str2int(ptr)) mchs_mic_start(TRUE);
		else mchs_mic_stop();
		ulog(STR_OK_CREATE("MIC")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "MCHS ")) {
		ptr = msg + strlen("MCHS ");
		if (str2int(ptr)) mchs_mic_start(FALSE);
		else mchs_mic_stop();
		ulog(STR_OK_CREATE("MCHS")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "MKK ")) {
		ptr = msg + strlen("MKK ");
		sz = hexstr2mem(ptr, strlen(ptr), " ", buf, sizeof(buf));
		mkk_send(UART2_MUX_MKKL_1, buf[0], (uint8_t *)&buf[1], sz - 1);
		ulog(STR_OK_CREATE("MKK")CRLF);
	}


	else if (strcmp_s(msg, strlen(msg), "MODE ")) {
		ptr = msg + strlen("MODE ");
		rc = str2int(ptr);
		ptr = strright(ptr, strlen(ptr), " ");

		if (rc == PU_MODE_VESHANIE_POJAR) {
			if (ptr) fire_lines_imit = str2ul(ptr);
		}
		else if (rc == PU_MODE_VESHANIE_PD) {
			pu_set_lines(str2ul(ptr));
			pd_start_translation(16000, FALSE);
		}

		pu_set_mode(rc);
	}
	else if (strcmp_s(msg, strlen(msg), "FSET ")) {
		ptr = msg + strlen("FSET ");
		rc = str2int(ptr);
		fault_set(rc);
		ulog(STR_OK_CREATE("FSET")CRLF);
	}
	else if (strcmp_s(msg, strlen(msg), "FUNSET ")) {
		ptr = msg + strlen("FUNSET ");
		rc = str2int(ptr);
		fault_unset(rc);
		ulog(STR_OK_CREATE("FUNSET")CRLF);
	}

*/
}



static uint8_t console_data[SoftUartRxBufferSizeRX] = {0};
static uint16_t console_data_count = 0;



void softuart_task_consoledata_rx(void)
{
	uint8_t RxAlavailable;
	uint16_t i;

	RxAlavailable = SoftUartRxAlavailable(CONSOLE_SOFT_UART_NUMBER);
	if (console_data_count + RxAlavailable > sizeof(console_data)) {
		console_data_count = 0;
	}

	SoftUartReadRxBuffer(CONSOLE_SOFT_UART_NUMBER, &console_data[console_data_count], RxAlavailable);
	console_data_count += RxAlavailable;


	for (i = 0; i < console_data_count; i++) {
		if ((console_data[i] == '\n') || (console_data[i] == '\r')) {
			console_data_count = 0;
			console_data[i] = 0;
			if (i > 1) console_rcv((char * )console_data);
			break;
		}

	}

}



void softuart_console_task(void)
{
	if (SoftUartRxAlavailable(CONSOLE_SOFT_UART_NUMBER)) {
		softuart_task_consoledata_rx();
	}

/*
	if (!SoftUartGetTxNComplated(CONSOLE_SOFT_UART_NUMBER)) { //TX Р·Р°РЅСЏС‚

		if (console_buf[0] != '\0') {
			SoftUartPuts(CONSOLE_SOFT_UART_NUMBER, (uint8_t *)console_buf, strlen(console_buf));
			console_buf[0] = '\0';
		}
	}
*/

}


