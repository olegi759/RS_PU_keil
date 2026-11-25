/*
 * twocore.c
 *
 *  Created on: 20 Р°РїСЂ. 2026 Рі.
 *      Author: Rafael Boltachev
 */


#include "cm4core.h"
#include "common.h"
#include "ringbuff_util.h"
#include "str_util.h"
#include "stm_util.h"
#include "fatfs_util.h"
#include "external_flash.h"
#include "mki.h"
#include "fault.h"
#include "fault_data.h"
#include "uart6.h"
#include "um.h"
#include "uart2.h"
#include "mkk.h"
#include "logfile_util.h"
#include "setting.h"
#include "main.h"
#include "ch378.h"
#include "softuart.h"
#include "console.h"
#include "pd_i2c.h"


static FIL cm4core_file = {0};
//#define cm4core_file	f_global_file

uint32_t cm4_core_ping_tick = 0;

//РЈСЃС‚Р°РЅРѕРІРєР° С‚СЂР°РЅСЃР»РёСЂСѓСЋС‰РёС… Р»РёРЅРёР№ РЅР° РњРљРљ
void cm4core_enable_lines(uint32_t lines)
{
	int i;
	uint32_t um1, um2, res;
	uint8_t line_on, um1_on, um2_on;


	um1 = 0;
	um2 = 0;

	for (i = 0; i < 24; i++) {
		line_on = GETBIT(lines, i); //РІРєР»СЋС‡РµРЅР° Р»Рё Р»РёРЅРёСЏ
		um1_on = GETBIT(setting.mkk_um1, i); //РІРєР»СЋС‡РµРЅ РЅР° РЈРњ1
		um2_on = GETBIT(setting.mkk_um2, i); //РІРєР»СЋС‡РµРЅ РЅР° РЈРњ2
		if (line_on && um1_on) {
			SETBIT(um1, i);
		}
		if (line_on && um2_on) {
			SETBIT(um2, i);
		}
	}

	mkk_send_lin(UART2_MUX_MKKL_1, um1 & 0xFFF, um2 & 0xFFF);
	HAL_Delay(50);
	mkk_send_lin(UART2_MUX_MKKL_2, (um1 >> 12) & 0xFFF, (um2 >> 12) & 0xFFF);
	HAL_Delay(50);
	um_snd_modes_all(um1 ? CH_MODE_TRANSLATION : CH_MODE_STANDBY, um2 ? CH_MODE_TRANSLATION :CH_MODE_STANDBY);

	res = um1 | um2;

	if (res != pu_enable_lines) ulog_fmt(CONM"pu: enable sound lines %s"CRLF, u24l2bitstr(res));

	pu_enable_lines = res;

	if (pu_option == PU_OPTION_NONE) {

		mki_enable_led_from_lines(pu_enable_lines, pu_mode2ledcolor(pu_mode));
	}

	cm4core_send_to_CM7_cmd(CORE_CMD_SET_ENABLE_LINES_RESULT, (uint8_t * )&pu_enable_lines, sizeof(pu_enable_lines));

	//if (pu_enable_lines) filelog_fmt("Р›РёРЅРёРё РѕРїРѕРІ.: %s", pu_lines2str(pu_enable_lines));
}


//---------------------------- RECV ----------------------------------------------
BOOL on_enum_opus_files(int index, char * name, unsigned long size,  void * param)
{

	char * buf;

	buf = (char *)param;

	if ( (strstr_s(name, strlen(name), ".opu") == NULL) && (strstr_s(name, strlen(name), ".OPU") == NULL) ) return FALSE;

	strcat_s(buf, SM_MAX_PACKET_SIZE, name);
	strcat_s(buf, SM_MAX_PACKET_SIZE, CRLF);

	return FALSE;
}
//static BOOL cm7_ready = FALSE;

BOOL on_enum_mp3_opus_files(int index, char * name, unsigned long size,  void * param)
{
	char * buf;

	buf = (char *)param;


	if ( is_file_name_opus(name) || is_file_name_mp3(name) )  {
		strcat_s(buf, SM_MAX_PACKET_SIZE, name);
		strcat_s(buf, SM_MAX_PACKET_SIZE, CRLF);
	}

	return FALSE;
}

//РїР°СЂСЃРёРЅРі РѕС‚ СЏРґСЂР° РЎРњ7 РєРѕРјРјР°РЅРґС‹
void cm4core_rcv_from_CM7(uint8_t * packet, uint16_t packet_len)
{
	uint8_t buf[SM_MAX_PACKET_SIZE];
	unsigned int uival;
	unsigned short usval;
	//int i;
//	uint8_t u8;


	packet[packet_len] = 0;

	switch (packet[0]) {
		case CORE_CMD_CONSOLE_SND: //РїРµС‡Р°С‚Р°РµРј РґР°РЅРЅС‹Рµ РІ Р»РѕРі РѕС‚ CM7
			ulog((char *)&packet[1]);
		break;
		case CORE_CMD_OPEN_FILE:
			if (f_is_open(&cm4core_file)) f_close(&cm4core_file);
			if (external_flash_is_mount) external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);

			buf[0] =  CORE_CMD_OPEN_FILE_RESULT;
			buf[1] =  0;

			if (f_open(&cm4core_file, (char *)&packet[1], FA_READ) == FR_OK) {
				buf[1] =  1;
			} else {
				if (external_flash_is_mount) {
					if (external_flash_open_file((char *)&packet[1])) {
						buf[1] =  1;
					}
				}
			}

			cm4core_send_to_CM7(buf, 2);
			//ulog_fmt(CONM"CORE_CMD_CM4_OPEN_FILE_RESULT %d"CRLF, buf[1]);
		break;
		case CORE_CMD_READ_FILE:
			usval = *(unsigned short *)&packet[1];

			//ulog_fmt(CONM"CORE_CMD_CM4_READ_FILE_RESULT usval=%d"CRLF, usval);

			buf[0] =  CORE_CMD_READ_FILE_RESULT;

			uival = 0;
			if (f_is_open(&cm4core_file)) {
				if (f_read(&cm4core_file, &buf[1], usval, &uival) != FR_OK) {
					uival = 0;
				}
			} else {
				//uival = external_flash_read_file(&buf[1], MIN(255, usval));
				uival = external_flash_stream_read(NULL, &buf[1], usval);
			}
			cm4core_send_to_CM7(buf, 1 + uival);
			//ulog_fmt(CONM"CORE_CMD_CM4_READ_FILE_RESULT %d %lu"CRLF, usval, uival);


		break;
		case CORE_CMD_CLOSE_FILE:
			if (f_is_open(&cm4core_file)) {
				f_close(&cm4core_file);
			} else {
				if (external_flash_is_mount) external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
			}
		break;
		case CORE_CMD_FILE_EXIST:
			if (f_is_open(&cm4core_file)) f_close(&cm4core_file);
			if (external_flash_is_mount) external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);

			buf[0] =  CORE_CMD_FILE_EXIST_RESULT;
			buf[1] =  0;

			if (f_open(&cm4core_file, (char *)&packet[1], FA_READ) == FR_OK) {
				f_close(&cm4core_file);
				buf[1] =  1;
			} else {
				if  (external_flash_is_mount) {
					if (external_flash_open_file((char *)&packet[1])) {
						external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
						buf[1] =  1;
					}
				}
			}

			cm4core_send_to_CM7(buf, 2);

		break;
		case CORE_CMD_GET_OPUS_LIST:
			 buf[0] = 0;
			 f_enum_files(F_DEFAULT_PATH, on_enum_opus_files, buf);
			 external_flash_enum_files(CH378_ENUM_ROOT_PATH, on_enum_opus_files, buf);
			 cm4core_send_to_CM7_cmd(CORE_CMD_GET_OPUS_LIST_RESULT, buf, strlen((char *)buf) + 1);
		break;

		case CORE_CMD_GET_MP3_OPUS_LIST:
			 buf[0] = 0;
			 f_enum_files(F_DEFAULT_PATH, on_enum_mp3_opus_files, buf);
			 external_flash_enum_files(CH378_ENUM_ROOT_PATH, on_enum_mp3_opus_files, buf);
			 cm4core_send_to_CM7_cmd(CORE_CMD_GET_MP3_OPUS_LIST_RESULT, buf, strlen((char *)buf) + 1);
		break;

		case CORE_CMD_MKI_SET_DISPLAY:
			mki_send_display_hd44780((const char *)&packet[1]);
		break;


		case CORE_CMD_FAULT_SET:
			fault_set(packet[1]);
		break;
		case CORE_CMD_FAULT_UNSET:
			fault_unset(packet[1]);
		break;
		case CORE_CMD_PD_SEND:
			uart6_transmit_to_pd_nomux(&packet[1], packet_len - 1);
		break;

		case CORE_CMD_PD_I2C_SEND:
			/* I2C2 line (same PD ring as UART4/UART7):
			   send frame to slave, read answer, pass it back to CM7 */
			{
				uint8_t pd_answer[PD_I2C_READ_SIZE];
				uint16_t pd_answer_len = 0;

				if (PD_I2C_SendReceive(&packet[1], (uint16_t)(packet_len - 1), pd_answer, &pd_answer_len)) {
					cm4core_pd_i2c_rcv(pd_answer, pd_answer_len);
				}
			}
		break;
		case CORE_CMD_ADD_LOG:
			filelog((char *)&packet[1]);
		break;
		case CORE_CMD_CHANGE_MODE:
			pu_mode = *(pu_mode_t *)&packet[1];

			if (pu_mode == PU_MODE_VESHANIE_POJAR) {
				mkk_send_relay_mux(UART2_MUX_MKKL_1, 2, TRUE);
				HAL_Delay(50);
				mkk_send_relay_mux(UART2_MUX_MKKL_2, 2, TRUE);
			} else {

				if (mkk_get_relay_mux(UART2_MUX_MKKL_1, 2)) {
					mkk_send_relay_mux(UART2_MUX_MKKL_1, 2, FALSE);
					HAL_Delay(50);
				}

				if (mkk_get_relay_mux(UART2_MUX_MKKL_2, 2)) {
					mkk_send_relay_mux(UART2_MUX_MKKL_2, 2, FALSE);
				}

			}

		break;
		case CORE_CMD_MKI_TEST:
			mki_send_test(packet[1]);
		break;
		case CORE_CMD_CHANGE_SAFETY_PINCODE_CM4:
			strcpy_s(safety_pin_code, sizeof(safety_pin_code), (char *)&packet[1]);
		break;
		case CORE_CMD_GET_VERSION_RESULT:
			strcpy_s(cm7_version, sizeof(cm7_version), (char *)&packet[1]);
		break;
		case CORE_CMD_SET_ENABLE_LINES:
			cm4core_enable_lines(*(uint32_t *)&packet[1]);
		break;
		case CORE_CMD_CHANGE_OPTION:
			pu_option = *(pu_option_t *)&packet[1];
		break;

		case CORE_CMD_MKI_PAINT_LED_FROM_LINES:
			uival = 0;
			memcpy(&uival, &packet[1], sizeof(uival));
			mki_enable_led_from_lines(uival, packet[5]);
		break;

		case CORE_CMD_MKI_SET_LED:
			mki_set_small_led(packet[1], packet[2]);
		break;

		case CORE_CMD_MKI_SEND_NEED_LEDS:
			mki_send_small_leds();
		break;
		case CORE_CMD_UID:
			memcpy(stm32_uid, &packet[1], MIN(sizeof(stm32_uid), (packet_len - 1)));
			ulog_fmt(CONM"UID: %lu"CRLF, (unsigned long)stm32_uid[0]);
		break;
		case CORE_CMD_PING:
			cm4_core_ping_tick = HAL_GetTickNoZero();
			ulog(CONM"CORE_CMD_PING"CRLF);

		break;
		case CORE_CMD_UM_TEST:
			HAL_Delay(50);
			um_transmit_piv(0xFF, setting.uart_local_addr , UM_CMD_TEST_LINE, NULL, 0);
		break;


		default: break;
	}
}

//РїСЂРёРµРј РѕС‚ СЏРґСЂР° РЎРњ7 РєРѕРјРјР°РЅРґС‹
void cm4core_task_data(void)
{
	int16_t packet_len;
	uint8_t packet[SM_MAX_PACKET_SIZE];

	sm->rb_cm7_to_cm4.busy = 1;
	packet_len = ringbuff_read_packet(&sm->rb_cm7_to_cm4, packet, sizeof(packet));
	sm->rb_cm7_to_cm4.busy = 0;
	if (packet_len) cm4core_rcv_from_CM7(packet, packet_len);


}

void cm4core_task(void)
{
	if ( (!sm->rb_cm7_to_cm4.busy) && (ringbuff_get_full(&sm->rb_cm7_to_cm4) > 4) ) {
		cm4core_task_data();
	}
}




BOOL cm4core_wait_cmd_from_CM7(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_size, uint32_t timeout, uint16_t * cmd_data_len)
{
	uint32_t start_tick;
	uint16_t packet_len, len;
	uint8_t packet[SM_MAX_PACKET_SIZE];

	start_tick = HAL_GetTick();

	while (HAL_DiffTick(start_tick) < timeout) {
		if ( (!sm->rb_cm7_to_cm4.busy) && (ringbuff_get_full(&sm->rb_cm7_to_cm4) > 4) ) {
			sm->rb_cm7_to_cm4.busy = 1;
			packet_len = ringbuff_read_packet(&sm->rb_cm7_to_cm4, packet, sizeof(packet));
			sm->rb_cm7_to_cm4.busy = 0;
			if (packet_len)  {
				if (packet[0] == cmd) {
					len = packet_len - 1;
					if (len > cmd_data_size) len = cmd_data_size;
					if (len) memcpy(cmd_data, &packet[1], len);
					*cmd_data_len = len;
					return TRUE;
				}
				cm4core_rcv_from_CM7(packet, packet_len);
			}

		}
		HAL_IWDG_Refresh(&hiwdg2);
	}
	return FALSE;
}



BOOL cm4core_get_cm7_version(char * buf, uint16_t buf_size)
{
	uint16_t cmd_data_len;
	BOOL wait;

	cm4core_send_to_CM7_cmd_only(CORE_CMD_GET_VERSION);
	wait = cm4core_wait_cmd_from_CM7(CORE_CMD_GET_VERSION_RESULT, (uint8_t *)buf, buf_size, 1000, &cmd_data_len);
	if (wait) {
		buf[cmd_data_len] = '\0';
	}

	return wait;
}

//---------------------------- SEND ----------------------------------------------

//РѕС‚РїСЂР°РІРёС‚СЊ РєРѕРјРјР°РЅРґСѓ РІ CM7
void cm4core_send_to_CM7(uint8_t * packet, uint16_t packet_len)
{
	uint32_t cnt = 10000;

	while (cnt--) {
		if (!sm->rb_cm4_to_cm7.busy) break;
	}
	sm->rb_cm4_to_cm7.busy = 1;
	ringbuff_write_packet(&sm->rb_cm4_to_cm7, packet, packet_len);
	sm->rb_cm4_to_cm7.busy = 0;
}

void cm4core_send_to_CM7_cmd(uint8_t cmd, void * cmd_data, uint16_t cmd_data_len)
{
	uint8_t buf[SM_MAX_PACKET_SIZE];

	buf[0] = cmd;

	if (cmd_data && cmd_data_len) {
		if (cmd_data_len > sizeof(buf) - 1) cmd_data_len = sizeof(buf) - 1;
		memcpy(&buf[1], cmd_data, cmd_data_len);
	}

	cm4core_send_to_CM7(buf, 1 + cmd_data_len);
}

void cm4core_send_to_CM7_cmd_only(uint8_t cmd)
{
	cm4core_send_to_CM7_cmd(cmd, NULL, 0);
}

void cm4core_pd_rcv(uint8_t * packet, uint16_t packet_len)
{
	cm4core_send_to_CM7_cmd(CORE_CMD_PD_RECV, packet, packet_len);
}

/* Answer received from the PD device on I2C2 line - send to CM7 as ring packet */
void cm4core_pd_i2c_rcv(uint8_t * packet, uint16_t packet_len)
{
	cm4core_send_to_CM7_cmd(CORE_CMD_PD_I2C_RECV, packet, packet_len);
}

void cm4core_button_press(uint8_t key)
{
	cm4core_send_to_CM7_cmd(CORE_CMD_MKI_BUTTON_PRESS, &key, 1);
}

void cm4core_button_unpress(uint8_t key)
{
	cm4core_send_to_CM7_cmd(CORE_CMD_MKI_BUTTON_UNPRESS, &key, 1);
}

void cm4core_change_setting(void)
{
	  cm4core_send_to_CM7_cmd(CORE_CMD_CHANGE_SETTING, &setting, sizeof(setting));
}
//---------------------------------------------------------------------------

void cm4core_init(void)
{
	while (!ringbuff_is_ready(&sm->rb_cm4_to_cm7) || !ringbuff_is_ready(&sm->rb_cm7_to_cm4)) {
		//HAL_IWDG_Refresh(&hiwdg);
	}
	//cm4core_send_to_CM7_cmd_only(CORE_CMD_CM4_READY);
}



