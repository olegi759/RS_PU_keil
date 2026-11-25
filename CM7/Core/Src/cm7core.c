/*
 * cm7core.c
 *
 *  Created on: 21 Р°РїСЂ. 2026 Рі.
 *      Author: Rafael Boltachev
 */
#include "cm7core.h"
#include <string.h>
#include <stdio.h>
#include "shared_memory.h"
#include "common.h"
#include "ringbuff_util.h"
#include "str_util.h"
#include "stm_util.h"
#include "mkk_data.h"
#include "pu.h"
#include "fault_data.h"
#include "screens.h"
#include "mki_data.h"
#include "pd.h"
#include "beep.h"
#include "dictionary_util.h"
#include "playopus.h"
#include "sendopus.h"
#include "mchs_mic.h"
#include "display.h"
#include "pd_data.h"
#include "math_util.h"
#include "playmp3.h"
#include "uarts.h"
#include "screen3.h"






//---------------------------- RECV ----------------------------------------------

void mkk_change_insig_lin(void)
{
	int i;

	fire_lines_insig = 0;

	for (i = 0; i < 24; i++) {
		if (GETBIT(pu_option_otkluchenie_lines, i)) continue;
		if (mkk_insig_lin_replay.insig[i] == MKK_INSIG_STATE_NORM_YES_SIGNAL) SETBIT(fire_lines_insig, i);
	}


	if ( (pu_mode < PU_MODE_VESHANIE_POJAR)  && fire_lines_insig) {

		fire_lines_imit = 0;
		pu_set_mode(PU_MODE_VESHANIE_POJAR);

	} else 	if ( (pu_mode == PU_MODE_VESHANIE_POJAR)  && !fire_lines_insig) {
		ulog("mkk: !!DELETE goto mode DEJURNIY from POJAR, where fire_lines = 0"CRLF);
		//fire_lines_imit = old_fire_lenes;
		//pu_set_mode(PU_MODE_DEJURNIY);
	} else {
		screen0_update();
	}

}

void cm7_console_replay_bool_cmd(BOOL value, const char * cmd)
{

	ulog_fmt(CONM"%s_%s"CRLF, (value ? STR_OK : STR_ERROR), cmd);
}


void cm7_console_rcv(char * msg)
{
	char * ptr, * ptr2;
	size_t msg_size;
	int ival;


	msg_size = strlen(msg);

	if (strcmp_s(msg, msg_size, STR_PLAY)) {
		ptr = msg + strlen(STR_PLAY) + 1;

		pu_enable_sound_lines(0x00FFFFFF);
		cm7_console_replay_bool_cmd( pu_play_audio_file(ptr), STR_PLAY);
	}
	else if (strcmp_s(msg, msg_size, STR_SEND)) {
		ptr = msg + strlen(STR_SEND) + 1;
		ptr2 = strstr_s(ptr, strlen(ptr), " ");
		if (ptr2) {
			*ptr2 = '\0';
			ptr2++;
		}
		pu_enable_sound_lines(0x00FFFFFF);

		ival = str2int(ptr2);


		cm7_console_replay_bool_cmd( sendopus_start(ptr, str2int(ptr2)),  STR_SEND);

		uarts_transmit_veshanie_pd(ival, sendopus_samplerate, sendopus_channels == 2, 0x00FFFFFF);
	}

	else if (strequals(msg, "MIC ON")) {
		mchs_mic_start(TRUE, 0xFF);
		ulog(STR_OK_CREATE("MIC ON")CRLF);
	}
	else if (strequals(msg, "MCHS ON")) {
		mchs_mic_start(FALSE, 0xFF);
		ulog(STR_OK_CREATE("MCHS ON")CRLF);
	}
	else if (strequals(msg, "MIC OFF")) {
		mchs_mic_stop();
		ulog(STR_OK_CREATE("MIC OFF")CRLF);
	}
	else if (strequals(msg, "MCHS OFF")) {
		mchs_mic_stop();
		ulog(STR_OK_CREATE("MCHS OFF")CRLF);
	}
	else if (strcmp_s(msg, msg_size, STR_DISPLAY" ")) {
		ptr = msg + strlen(STR_DISPLAY" ");
		set_display_all(ptr);
		send_display();
		ulog(STR_OK_CREATE(STR_DISPLAY)CRLF);
	}
	else if (strequals(msg, STR_VERSION)) {
		ulog_fmt(CONM"OK_VERSION %s"CRLF, VERSION);
	}
	else if (strcmp_s(msg, msg_size, "KEY ")) {
		ptr = msg + strlen("KEY ");

		if (strcmp_s(ptr, strlen(ptr), "TEST")) ival =  25;
		else if (strcmp_s(ptr, strlen(ptr), "MUTE")) ival = 26;
		else if (strcmp_s(ptr, strlen(ptr), "RESET")) ival = 27;
		else if (strcmp_s(ptr, strlen(ptr), "MIC")) ival = 28;
		else if (strcmp_s(ptr, strlen(ptr), "MENU")) ival = 29;
		else if (strcmp_s(ptr, strlen(ptr), "LEFT")) ival = 30;
		else if (strcmp_s(ptr, strlen(ptr), "UP")) ival = 31;
		else if (strcmp_s(ptr, strlen(ptr), "RIGHT")) ival = 32;
		else if (strcmp_s(ptr, strlen(ptr), "DEL")) ival = 33;
		else if (strcmp_s(ptr, strlen(ptr), "EXIT")) ival = 34;
		else if (strcmp_s(ptr, strlen(ptr), "DOWN")) ival = 35;
		else if (strcmp_s(ptr, strlen(ptr), "ENTER")) ival = 36;
		else if (strcmp_s(ptr, strlen(ptr), "ZONES")) ival = 37;
		else if (strcmp_s(ptr, strlen(ptr), "GROUPS")) ival = 38;
		else if (strcmp_s(ptr, strlen(ptr), "ALLZONES")) ival = 39;
		else if (strcmp_s(ptr, strlen(ptr), "MANUALSTART")) ival = 40;
		else ival = str2int(ptr);

		ulog_fmt(CONM"press key %s(%d)"CRLF,  mki_get_key_name(ival), ival);

		screens_button_press(ival);
	}
	else if (strcmp_s(msg, msg_size, "UNKEY ")) {
		ptr = msg + strlen("UNKEY ");

		if (strcmp_s(ptr, strlen(ptr), "TEST")) ival =  25;
		else if (strcmp_s(ptr, strlen(ptr), "MUTE")) ival = 26;
		else if (strcmp_s(ptr, strlen(ptr), "RESET")) ival = 27;
		else if (strcmp_s(ptr, strlen(ptr), "MIC")) ival = 28;
		else if (strcmp_s(ptr, strlen(ptr), "MENU")) ival = 29;
		else if (strcmp_s(ptr, strlen(ptr), "LEFT")) ival = 30;
		else if (strcmp_s(ptr, strlen(ptr), "UP")) ival = 31;
		else if (strcmp_s(ptr, strlen(ptr), "RIGHT")) ival = 32;
		else if (strcmp_s(ptr, strlen(ptr), "DEL")) ival = 33;
		else if (strcmp_s(ptr, strlen(ptr), "EXIT")) ival = 34;
		else if (strcmp_s(ptr, strlen(ptr), "DOWN")) ival = 35;
		else if (strcmp_s(ptr, strlen(ptr), "ENTER")) ival = 36;
		else if (strcmp_s(ptr, strlen(ptr), "ZONES")) ival = 37;
		else if (strcmp_s(ptr, strlen(ptr), "GROUPS")) ival = 38;
		else if (strcmp_s(ptr, strlen(ptr), "ALLZONES")) ival = 39;
		else if (strcmp_s(ptr, strlen(ptr), "MANUALSTART")) ival = 40;
		else ival = str2int(ptr);

		ulog_fmt(CONM"unpress key %s(%d)"CRLF,  mki_get_key_name(ival), ival);
		screens_button_unpress(ival);
	}
	else if (strcmp_s(msg, msg_size, "SETLINES ")) {
		ptr = msg + strlen("SETLINES ");
		pu_enable_sound_lines(str2int(ptr));
		ulog(CONM"OK_SETLINES"CRLF);
	}
	else if (strcmp_s(msg, msg_size, "MODE ")) {
		ptr = msg + strlen("MODE ");
		ival = str2int(ptr);
		ptr = strright(ptr, strlen(ptr), " ");

		if (ival == PU_MODE_VESHANIE_POJAR) {
			if (ptr) fire_lines_imit = str2ul(ptr);
		}

		if (ival == PU_MODE_VESHANIE_PD) {
			pu_prepare_audio_play(16000, FALSE);
		}

		pu_set_mode(ival);
		if (ptr) pu_enable_sound_lines(str2int(ptr));

	}
	else if (strequals(msg, "IWDG_CM7")) {
		ulog(CONM"OK_IWDG"CRLF);
		while (TRUE) {
			ival++;
		}
	}
	else if (strequals(msg, "SPISOK")) {

		pu_small_info_t  small_info;

		pu_get_local_small_info(&small_info);

		uart4_uart7_snd_piv(0xFF, 1 + setting.uart_local_addr, PD_CMD_SPISOK_PU, (uint8_t *)&small_info, sizeof(small_info)); //РЅРµ РґРѕС…РѕРґРёС‚ РґРѕ РџРЈ2
		/* second line is covered by uart4_uart7_snd_piv() above */

		ulog(CONM"OK_SPISOK"CRLF);
	}



}


//static BOOL cm4_ready = FALSE;
static BOOL already_change_setting = FALSE;

void cm7core_change_faults_small(void)
{
	int num;

	faults_small = 0;

	for (num = 0; num < MAX_FAULTS; ++num) {

		if (!bitget(faults, num)) continue;

		if ((num >= FAULT_MUP_220V) && (num <= FAULT_MUP_TD_ACC)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_MUP);
		if ((num >= FAULT_MKK_INSIG_1) && (num <= FAULT_MKK_INSIG_2)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_MKK_INSIG);
		if ((num >= FAULT_MKK_SOUND_1) && (num <= FAULT_MKK_SOUND_24)) 	SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_MKK_SOUND);
		if ((num >= FAULT_MKK_START_1) && (num <= FAULT_MKK_START_2)) 	SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_MKK_START);


		if ((num >= FAULT_PD_CON_1) && (num <= FAULT_PD_CON_2)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_PD_CON);
		if ((num >= FAULT_UM_CON_1) && (num <= FAULT_UM_CON_6)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_UM_CON);
		if ((num >= FAULT_UM_PROTECT_CH1_1) && (num <= FAULT_UM_TEST_LINE2_6)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_UM);

		if ((num >= FAULT_MKK_DEVICE_1) && (num <= FAULT_MKK_DEVICE_2)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_MKK_START_DEVICE);
		if (num == FAULT_PD_RING) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_PD_CON);
		if ((num == FAULT_PU_RING_CMD) || (num == FAULT_PU_RING_AUDIO)) SETBIT(faults_small, PD_CMD_QUERY_FAULT_SMALL_PD_CON);
	}

}

//РїР°СЂСЃРёРЅРі РѕС‚ СЏРґСЂР° РЎРњ4 РєРѕРјРјР°РЅРґС‹
void cm7core_rcv_from_CM4(uint8_t * packet, uint16_t packet_len)
{

	packet[packet_len] = 0;

	switch (packet[0]) {
		case CORE_CMD_CONSOLE_RCV: //РѕС‚ РєРѕРЅСЃРѕР»Рё CM4
			cm7_console_rcv((char *)&packet[1]);
		break;
		case CORE_CMD_CHANGE_SETTING:
			memcpy(&setting, &packet[1], MIN(sizeof(setting_t), (packet_len - 1)));
			ulog(CONM"OK_CHANGE_SETTING"CRLF);
			if (!already_change_setting)	{
				ulog_fmt(CONM"setting: SAI volume=%d"CRLF, setting.sai_volume);
				ulog_fmt(CONM"setting: name=%s"CRLF, get_pu_name());
				already_change_setting = TRUE;

				uarts_transmit_change_mode();
			}
		break;
		break;
		case CORE_CMD_MKI_BUTTON_PRESS:
			screens_button_press(packet[1]);
		break;
		case CORE_CMD_MKI_BUTTON_UNPRESS:
			screens_button_unpress(packet[1]);
		break;
		case CORE_CMD_CHANGE_MKK_INSIG_LIN:
			memcpy(&mkk_insig_lin_replay, &packet[1], MIN(sizeof(mkk_insig_lin_replay), (packet_len - 1)));
			mkk_change_insig_lin();
			ulog(CONM"OK_CHANGE_INSIG_LIN_REPLAY"CRLF);
		break;
		case CORE_CMD_GET_VERSION:
			cm7core_send_to_CM4_cmd(CORE_CMD_GET_VERSION_RESULT, (uint8_t *)VERSION, strlen(VERSION));
		break;
		case CORE_CMD_RTC_UPDATE:
			memcpy(date_time, &packet[1], MIN(sizeof(date_time), (packet_len - 1)));
		break;
		case CORE_CMD_PD_RECV:
			if (packet[1] == setting.uart_local_addr) {
				//РІ pd_rcv РІС…РѕРґРёРј С‚РѕР»СЊРєРѕ СЃ Р»РѕРєР°Р»СЊРЅС‹Рј Р°РґСЂРµСЃРѕРј
				pd_rcv(&packet[1], packet_len - 1, 0);
			} else {
				packet[2] = packet[2] + setting.uart_local_addr; //С€Р»СЋР·
				count_tx_id++;
				packet[3] = count_tx_id >> 8;
				packet[4] = count_tx_id & 0xFF;
				fill_crc16(&packet[1], packet_len - 1);
				uart4_uart7_snd(&packet[1], packet_len - 1);
			}
		break;
		case CORE_CMD_PD_I2C_RECV:
			/* packet received from I2C2 line - same ring logic as UART4/UART7 */
			uart4_uart7_rcv(UART_LINE_I2C2, &packet[1], packet_len - 1);
		break;
		case CORE_CMD_CHANGE_FAULTS:
			memcpy(faults, &packet[1], MIN(sizeof(faults), (packet_len - 1)));

			cm7core_change_faults_small();

			//Р РђРЎРљРћРњР•РќРўРР РћР’РђРўР¬ if (fault_count()) beep_on(BEEP_TIMEOUT_UNLIMIT);
			//Р РђРЎРљРћРњР•РќРўРР РћР’РђРўР¬ else beep_off();

			screen0_update();

		break;
		case CORE_CMD_OPEN_FILE_RESULT:
			ulog_fmt(CONM"CORE_CMD_OPEN_FILE_RESULT %d"CRLF, packet[1]);
		break;
		case CORE_CMD_SET_ENABLE_LINES_RESULT:
			pu_enable_lines = 0;
			memcpy(&pu_enable_lines, &packet[1], MIN(sizeof(pu_enable_lines), (packet_len - 1)));
		break;
		case CORE_CMD_CHANGE_SAFETY_PINCODE_CM7:
			strcpy_s(safety_pin_code, sizeof(safety_pin_code), (char *)&packet[1]);
			screen3_pin_code_enter_tick = HAL_GetTickNoZero();
		break;

		default: break;
	}
}


//РїСЂРёРµРј РѕС‚ СЏРґСЂР° РЎРњ4 РєРѕРјРјР°РЅРґС‹
void cm7core_task_data(void)
{
	int16_t packet_len;
	uint8_t packet[SM_MAX_PACKET_SIZE];

	sm->rb_cm4_to_cm7.busy = 1;
	packet_len = ringbuff_read_packet(&sm->rb_cm4_to_cm7, packet, sizeof(packet));
	sm->rb_cm4_to_cm7.busy = 0;
	if (packet_len)  cm7core_rcv_from_CM4(packet, packet_len);

}


void cm7core_task(void)
{
	if ( (!sm->rb_cm4_to_cm7.busy) && (ringbuff_get_full(&sm->rb_cm4_to_cm7) > 4) ) {
		cm7core_task_data();
	}

}


BOOL cm7core_wait_cmd_from_CM4(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_size, uint32_t timeout, uint16_t * cmd_data_len)
{
	uint32_t start_tick;
	uint16_t packet_len, len;
	uint8_t packet[SM_MAX_PACKET_SIZE];

	start_tick = HAL_GetTick();

	while (HAL_DiffTick(start_tick) < timeout) {
		if ( (!sm->rb_cm4_to_cm7.busy) && (ringbuff_get_full(&sm->rb_cm4_to_cm7) > 4) ) {
			sm->rb_cm4_to_cm7.busy = 1;
			packet_len = ringbuff_read_packet(&sm->rb_cm4_to_cm7, packet, sizeof(packet));
			sm->rb_cm4_to_cm7.busy = 0;
			if (packet_len)  {
				if (packet[0] == cmd) {
					len = packet_len - 1;
					if (len > cmd_data_size) len = cmd_data_size;
					if (len) memcpy(cmd_data, &packet[1], len);
					*cmd_data_len = len;
					return TRUE;
				}
				cm7core_rcv_from_CM4(packet, packet_len);
			}

		}
		HAL_IWDG_Refresh(&hiwdg);
	}
	return FALSE;
}


//---------------------------- SEND ----------------------------------------------

BOOL cm7core_send_to_CM4(uint8_t * packet, uint16_t packet_len)
{
	uint16_t cnt = 10000;
	BOOL res = FALSE;

	while (cnt--) {
		if (!sm->rb_cm7_to_cm4.busy) break;
	}

	sm->rb_cm7_to_cm4.busy = 1;
	if (ringbuff_get_free(&sm->rb_cm7_to_cm4) > packet_len + 6) {
		cnt = ringbuff_write_packet(&sm->rb_cm7_to_cm4, packet, packet_len);
		if (cnt == packet_len) res = TRUE;
	}
	sm->rb_cm7_to_cm4.busy = 0;

	return res;
}


//РѕС‚РїСЂР°РІРёС‚СЊ РєРѕРјРјР°РЅРґСѓ РІ CM4
BOOL cm7core_send_to_CM4_cmd(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uint8_t buf[SM_MAX_PACKET_SIZE];

	buf[0] = cmd;
	if (cmd_data && cmd_data_len) {
		if (cmd_data_len > sizeof(buf) - 1) cmd_data_len = sizeof(buf) - 1;
		memcpy(&buf[1], cmd_data, cmd_data_len);
	}
	return cm7core_send_to_CM4(buf, 1 + cmd_data_len);
}

BOOL cm7core_send_to_CM4_cmd_only(uint8_t cmd)
{
	return cm7core_send_to_CM4_cmd(cmd, NULL, 0);
}

void ulog(char *str)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_CONSOLE_SND, (uint8_t *)str, strlen(str));
}


BOOL cm7core_open_file(const char * file_name)
{
	uint8_t byte;
	uint16_t cmd_data_len;
	BOOL wait;

	cm7core_send_to_CM4_cmd(CORE_CMD_OPEN_FILE, (uint8_t *)file_name, strlen(file_name));
	wait = cm7core_wait_cmd_from_CM4(CORE_CMD_OPEN_FILE_RESULT, &byte, 1, 10000, &cmd_data_len);

	return wait ? (byte ? TRUE: FALSE ) : FALSE;
}


uint16_t cm7core_read_file(uint8_t * buf, uint16_t buf_len)
{
	BOOL wait;
	uint16_t cmd_data_len;

	cm7core_send_to_CM4_cmd(CORE_CMD_READ_FILE, (uint8_t *)&buf_len, sizeof(buf_len));
	wait = cm7core_wait_cmd_from_CM4(CORE_CMD_READ_FILE_RESULT, buf, buf_len, 10000, &cmd_data_len);
	return wait ? cmd_data_len : 0;
}

void cm7core_close_file(void)
{
	cm7core_send_to_CM4_cmd_only(CORE_CMD_CLOSE_FILE);
}

BOOL cm7core_file_exist(const char * file_name)
{
	uint8_t byte;
	uint16_t cmd_data_len;
	BOOL wait;

	cm7core_send_to_CM4_cmd(CORE_CMD_FILE_EXIST, (uint8_t *)file_name, strlen(file_name));
	wait = cm7core_wait_cmd_from_CM4(CORE_CMD_FILE_EXIST_RESULT, &byte, 1, 10000, &cmd_data_len);

	return wait ? (byte ? TRUE: FALSE ) : FALSE;
}

void cm7core_mki_set_display(char * str)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_MKI_SET_DISPLAY, (uint8_t *)str, 80);

}




void cm7core_set_led_color(uint8_t number, uint8_t color)
{
	uint8_t buf[2] = {number, color};

	cm7core_send_to_CM4_cmd(CORE_CMD_MKI_SET_LED, buf, 2);
}

void cm7core_send_leds(void)
{
	cm7core_send_to_CM4_cmd_only(CORE_CMD_MKI_SEND_NEED_LEDS);
}


void cm7core_send_one_led_color(uint8_t number, uint8_t color)
{
	cm7core_set_led_color(number, color);
	cm7core_send_leds();
}

void cm7core_option_change(void)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_CHANGE_OPTION, (uint8_t *)&pu_option, sizeof(pu_option));
}


void cm7core_mki_test(uint8_t enable)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_MKI_TEST, (uint8_t *)&enable, sizeof(enable));
}

void cm7core_fault_set(uint8_t number)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_FAULT_SET, (uint8_t *)&number, sizeof(number));
}

void cm7core_fault_unset(uint8_t number)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_FAULT_UNSET, (uint8_t *)&number, sizeof(number));
}

//void cm7core_um_send_mode_all(uint8_t mode_ch1, uint8_t mode_ch2)
//{
//	uint8_t buf[2] = {mode_ch1, mode_ch2};

//	cm7core_send_to_CM4_cmd(CORE_CMD_UM_SEND_MODE_ALL, buf, 2);
//}

void cm7core_mkk_send_lin(uint8_t cmd, uint16_t um1, uint16_t um2)
{
	uint16_t buf[2] = {um1, um2};

	cm7core_send_to_CM4_cmd(cmd, (uint8_t *)buf, 4);
}

void cm7core_mkk_send_relay_mux(uint8_t cmd, uint8_t relay, BOOL on)
{
	uint8_t buf[2] = {relay, on ? 1 : 0};

	cm7core_send_to_CM4_cmd(cmd, (uint8_t *)buf, 2);
}

void cm7core_pd_send(uint8_t * packet, uint16_t packet_len)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_PD_SEND, packet, packet_len);

}

/* Передать кадр ПД-кольца на линию I2C2 (исполняет CM4 - он мастер I2C2) */
void cm7core_i2c_send(uint8_t * packet, uint16_t packet_len)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_PD_I2C_SEND, packet, packet_len);
}



uint16_t cm7core_get_opus_list(char * buf, uint16_t buf_len)
{
	BOOL wait;
	uint16_t cmd_data_len;

	cm7core_send_to_CM4_cmd(CORE_CMD_GET_OPUS_LIST, (uint8_t *)&buf_len, sizeof(buf_len));
	wait = cm7core_wait_cmd_from_CM4(CORE_CMD_GET_OPUS_LIST_RESULT, (uint8_t *)buf, buf_len, 10000, &cmd_data_len);
	return wait ? cmd_data_len : 0;
}

uint16_t cm7core_get_mp3_and_opus_list(char * buf, uint16_t buf_len)
{
	BOOL wait;
	uint16_t cmd_data_len;

	cm7core_send_to_CM4_cmd(CORE_CMD_GET_MP3_OPUS_LIST, (uint8_t *)&buf_len, sizeof(buf_len));
	wait = cm7core_wait_cmd_from_CM4(CORE_CMD_GET_MP3_OPUS_LIST_RESULT, (uint8_t *)buf, buf_len, 10000, &cmd_data_len);
	return wait ? cmd_data_len : 0;
}

const char * cm7_mkk_get_lin_state_str(uint8_t line_number)
{
	return  mkk_lin_state2str(mkk_insig_lin_replay.sound[line_number]);
}




const char * cm7_mkk_get_insig_state_str(uint8_t line_number)
{
	return  mkk_insig_state2str(mkk_insig_lin_replay.insig[line_number]);
}



const char * cm7_mkk_get_start_state_str(uint8_t line_number)
{
	return  mkk_start_state2str(mkk_insig_lin_replay.start[line_number]);
}

//---------------------------------------------------------------------------------

void cm7core_init(void)
{
	shared_memory_init();
}

void filelog(char * str)
{
	cm7core_send_to_CM4_cmd(CORE_CMD_ADD_LOG, (uint8_t *)str, strlen(str));
}
