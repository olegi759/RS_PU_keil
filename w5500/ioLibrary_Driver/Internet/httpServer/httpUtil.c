/**
 * @file	httpUtil.c
 * @brief	HTTP Server Utilities	
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpUtil.h"
#include "httpUtil.h"
#include "wizchip_conf.h"
#include "str_util.h"
#include "http_util.h"
#include "loopback.h"
#include "net.h"
#include "net_util.h"
#include "setting.h"
#include "rtc.h"
#include "mkk.h"
#include "mkk_data.h"
#include "ch378.h"
#include "external_flash.h"
#include "mup.h"
#include "webpage.h"
#include "fault.h"
#include "safety.h"
#include "um.h"
#include "cm4core.h"
#include "speaker.h"
#include "common.h"
#include "flash_io.h"



#define CGI_SCRIPT_INDEX_NUM		1
#define CGI_SCRIPT_NETWORK_NUM		2
#define CGI_SCRIPT_SETTING_NUM		3
#define CGI_SCRIPT_CONTROL_NUM		4
#define CGI_SCRIPT_MODE_NUM			5
#define CGI_SCRIPT_FILEMANAGER_NUM	6
#define CGI_SCRIPT_SAFETY_NUM		7
#define CGI_SCRIPT_AUTH_NUM			8
#define CGI_SCRIPT_NETFUNC_NUM		9
#define CGI_SCRIPT_ERROR_USER_LEVEL_NUM		10

uint8_t cgi_page_num = 0;



void add_json_key_value(const char * key, const char * value, char * buf, uint16_t  buf_size)
{
	if (strlen(buf) > 1) {
		strcat_s(buf, buf_size, ",");

	}
	strcat_s(buf, buf_size, "\"");
	strcat_s(buf, buf_size, key);
	strcat_s(buf, buf_size, "\":\"");
	strcat_s(buf, buf_size, value);
	strcat_s(buf, buf_size, "\"");
}



char * get_insig_state_html(uint8_t val)
{
	switch (val) {
		//Неизвестно
		case MKK_INSIG_STATE_UNKNOW: return HTML_SQR_GRAY;
		//Неисправность обрыв, сопротивление линии более 5,4 кОм
		case MKK_INSIG_STATE_BREAK: return  HTML_SQR_RED;
		//Норма сигнала нет, сопротивление 4,7 кОм (4…5,4 кОм)
		case MKK_INSIG_STATE_NORM_NO_SIGNAL: return HTML_SQR_GREEN;
		//Неисправность зона неопределенности, сопротивление 3…4 кОм
		case MKK_INSIG_STATE_UNCERTAINTY: return  HTML_SQR_RED;
		//Норма сигнал есть, сопротивление 2,35 кОм (2…3 кОм)
		case MKK_INSIG_STATE_NORM_YES_SIGNAL: return HTML_SQR_GREEN;
		//Неисправность КЗ, сопротивление линии менее 2 кОм
		case MKK_INSIG_STATE_SHORT_CIRCUIT: return  HTML_SQR_RED;
		default: return HTML_SQR_BLACK;
	}
}

char * get_lin_state_html(uint8_t val)
{
	switch (val) {
		//Неизвестно
		case MKK_LIN_STATE_UNKNOW: return HTML_SQR_GRAY;
		//Линия исправна
		case MKK_LIN_STATE_OK: return HTML_SQR_GREEN;
		//Обрыв
		case MKK_LIN_STATE_BREAK: return HTML_SQR_RED;
		//Линия неисправна, сопротивление линии увеличено (или сопротивление линии выше нормы при инсталляции)
		case MKK_LIN_STATE_UP: return HTML_SQR_RED;
		//Линия неисправна, сопротивление линии снижено (или сопротивление линии меньше нормы при инсталляции)
		case MKK_LIN_STATE_DOWN: return HTML_SQR_RED;
		//КЗ линии
		case MKK_LIN_STATE_SHORT_CIRCUIT: return HTML_SQR_RED;
		default: return HTML_SQR_BLACK;

	}

}

char * get_start_state_html(uint8_t val)
{
	switch (val) {

		//Неизвестно
		case MKK_START_STATE_UNKNOW: return HTML_SQR_GRAY;
		//Неисправность обрыв, сопротивление линии более 5,4 кОм
		case MKK_START_STATE_BREAK: return  HTML_SQR_RED;
		//Норма сигнала нет, сопротивление 4,7 кОм (4…5,4 кОм)
		case MKK_START_STATE_NORM_NO_SIGNAL: return HTML_SQR_GREEN;
		//Неисправность зона неопределенности, сопротивление 3…4 кОм
		case MKK_START_STATE_UNCERTAINTY: return  HTML_SQR_RED;
		//Норма сигнал есть, сопротивление 2,35 кОм (2…3 кОм)
		case MKK_START_STATE_NORM_YES_SIGNAL: return HTML_SQR_GREEN;
		//Неисправность КЗ, сопротивление линии менее 2 кОм
		case MKK_START_STATE_SHORT_CIRCUIT: return  HTML_SQR_RED;
		default: return HTML_SQR_BLACK;
	}
}

BOOL change_cis	= FALSE;
BOOL change_csl	= FALSE;
BOOL change_um = FALSE;

//static uint8_t mode_manual_uo = 1;
static uint8_t mode_fire_uo = 1;

static char get_or_set_item_buf[MAX_STR_SIZE] = {'\0'};

static uint8_t mkk_rcv_sound_old[25] = {0};
static int tmp_value = 0;

//static uint32_t get_or_set_item_f_enum_size = 0;

unsigned long long all_files_size = 0;

/* Ключи вида net0_ip / net1_dhcp -> setting.chipsNetInfo[N].
   Возвращает строковое значение поля или NULL, если ключ не наш. */
static char * net_item_by_key(const char * key, char * val, BOOL set)
{
	uint8_t ch;
	const char * field;

	if (key[0] != 'n' || key[1] != 'e' || key[2] != 't' || key[4] != '_') return NULL;
	if (key[3] < '0' || key[3] >= (char)('0' + _WIZCHIP_CHIP_NUM_)) return NULL;

	ch    = (uint8_t)(key[3] - '0');
	field = &key[5];

	if (!strcmp(field, "mac")) {
		if (set) memcpy(setting.chipsNetInfo[ch].mac, str2mac(val), sizeof(setting.chipsNetInfo[ch].mac));
		return mac2str(setting.chipsNetInfo[ch].mac);
	}
	if (!strcmp(field, "dhcp")) {
		if (set) setting.chipsNetInfo[ch].dhcp = str2bool(val);
		return bool2str(setting.chipsNetInfo[ch].dhcp);
	}
	if (!strcmp(field, "ip")) {
		if (set) setting.chipsNetInfo[ch].ip = str2ip(val);
		return ip2str(setting.chipsNetInfo[ch].ip);
	}
	if (!strcmp(field, "mask")) {
		if (set) setting.chipsNetInfo[ch].mask = str2ip(val);
		return ip2str(setting.chipsNetInfo[ch].mask);
	}
	if (!strcmp(field, "gw")) {
		if (set) setting.chipsNetInfo[ch].gw = str2ip(val);
		return ip2str(setting.chipsNetInfo[ch].gw);
	}
	if (!strcmp(field, "dns")) {
		if (set) setting.chipsNetInfo[ch].dns = str2ip(val);
		return ip2str(setting.chipsNetInfo[ch].dns);
	}
	return NULL;
}

BOOL get_or_set_item_f_enum_files_proc(int index, char * name, unsigned long size, void * param)
{
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<a href='");
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), name);
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "'>");
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), name);
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "</a>");
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
	//strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<tr><td>");
	//strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), name);
	//strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "</td><td>");
	//strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), int2str(size));
	//strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "</td></tr>");
	//get_or_set_item_f_enum_size += size;

	all_files_size += size;
	return FALSE;
}


BOOL external_flash_get_or_set_item_fun(int index, char * file_name,  unsigned long file_size, void * param)
{
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), file_name);
	strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
	return FALSE;
}

char * get_or_set_item(char * key, char * val, BOOL set)
{

	char * ptr;
	int ival, ival2;
	//ulog_fmt("key:%s val:%s set:%s"CRLF, key, (val ? val : "NULL"), bool2str(set));

	ptr = net_item_by_key(key, val, set);      /* net0_* / net1_* — оба канала */
	if (ptr) return ptr;

	switch (cgi_page_num) {

		//------------------------- Информация ------------------------------------------
		case CGI_SCRIPT_INDEX_NUM:

			//index.html
			if (strequals(key, KEY_NAME)) {
				return  get_pu_name();
			}
			else if (strequals(key, KEY_VERSION)) {
				return VERSION;
			}
			else if (strequals(key, KEY_CM7_VERSION)) {
				return cm7_version;
			}

			// else if (strequals(key, KEY_NETINFO_MAC)) {
			// 	//return mac2str(net_info.mac);
			// 	return mac2str(setting.chipsNetInfo[g_wizchip_sel].mac);
			// }
			// else if (strequals(key, KEY_NETINFO_IP)) {
			// 	//return ip2str(*(uint32_t *)net_info.ip);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].ip);
			// }
			// else if (strequals(key, KEY_NETINFO_GW)) {
			// 	//return ip2str(*(uint32_t *)net_info.gw);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].gw);
			// }
			// else if (strequals(key, KEY_NETINFO_MASK)) {
			// 	//return ip2str(*(uint32_t *)net_info.sn);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].mask);
			// }
			// else if (strequals(key, KEY_NETINFO_DNS)) {
			// 	//return ip2str(*(uint32_t *)net_info.dns);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].dns);
			// }
			// else if (strequals(key, KEY_NETINFO_DHCP)) {
			// 	//return (net_info.dhcp == NETINFO_DHCP ? "Включено" : "Выключено");
			// 	return (bool2str(setting.chipsNetInfo[g_wizchip_sel].dhcp) ? "Включено" : "Выключено");
			// }ъ
			
			else if (strequals(key, KEY_MODE_NOW)) {
				return (char *)pu_mode2str(pu_mode);
			}
			else if (strequals(key, KEY_INFO_FAULTS)) {
				return fault_count() ? int2str(fault_count()) : "НЕТ";
			}
			else if (strequals(key, KEY_LEVEL_NOW)) {
				return int2str(safety_get_level_from_pin_code());
			}
			else if (strequals(key, "reboot")) {
				NVIC_SystemReset();
				return "";
			}
			else if (strequals(key, KEY_MKI_VERSION)) {
				return mki_version;
			}
			else if (strequals(key, KEY_MKK1_VERSION)) {
				return mkk1_version;
			}
			else if (strequals(key, KEY_MKK2_VERSION)) {
				return mkk2_version;
			}

			else if (strequals(key, KEY_STM32_UID)) {
				return ul2str(stm32_uid[0]);
			}

			else return NULL;

		break;

		//------------------------- Ннастройки сети ------------------------------------------
		case CGI_SCRIPT_NETWORK_NUM:

			//network.html
			// if (strequals(key, KEY_MAC)) {
			// 	if (set) memcpy(setting.chipsNetInfo[g_wizchip_sel].mac, str2mac(val), sizeof(setting.chipsNetInfo[g_wizchip_sel].mac));
			// 	return mac2str(setting.chipsNetInfo[g_wizchip_sel].mac);
			// }
			// else if (strequals(key, KEY_IP)) {
			// 	if (set) setting.chipsNetInfo[g_wizchip_sel].ip = str2ip(val);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].ip);
			// }
			// else if (strequals(key, KEY_GW)) {
			// 	if (set) setting.chipsNetInfo[g_wizchip_sel].gw = str2ip(val);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].gw);
			// }
			// else if (strequals(key, KEY_MASK)) {
			// 	if (set) setting.chipsNetInfo[g_wizchip_sel].mask = str2ip(val);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].mask);
			// }
			// else if (strequals(key, KEY_DNS)) {
			// 	if (set) setting.chipsNetInfo[g_wizchip_sel].dns = str2ip(val);
			// 	return ip2str(setting.chipsNetInfo[g_wizchip_sel].dns);
			// }
			// else if (strequals(key, KEY_DHCP)) {
			// 	if (set) setting.chipsNetInfo[g_wizchip_sel].dhcp = str2bool(val);
			// 	return bool2str(setting.chipsNetInfo[g_wizchip_sel].dhcp);
			// } else
			if (strequals(key, KEY_MODE_NOW)) {
				return (char *)pu_mode2str(pu_mode);
			}
			else return NULL;


		break;

		//------------------------- Общие настройки ------------------------------------------
		case CGI_SCRIPT_SETTING_NUM:

			//setting.html
			if (strequals(key, KEY_UART_LOCAL_ADDR)) {
				if (set) setting.uart_local_addr = str2int(val);
				return int2str(setting.uart_local_addr);
			}
			else if (strequals(key, KEY_LINES_COUNT)) {
				if (set) {
					//if (safety_get_level_from_pin_code() > 4) { //5 и выше
						setting.lines_count = str2int(val);
					//}
				}
				return int2str(setting.lines_count);
			}
			else if (strequals(key, KEY_SAI_VOLUME)) {
				if (set) setting.sai_volume = str2int(val);
				return int2str(setting.sai_volume);
			}
			else if (strequals(key, KEY_DATE_TIME)) {
				if (set) rtc_set_datetime_str(val);
				return rtc_read_datetime_str(FALSE);
			}
			//else if (strequals(key, KEY_NAME)) {
			//	if (set) strcpy_s(setting.name, sizeof(setting.name), val);
			//	return setting.name;
			//}
			else if (strequals(key, KEY_MIC_VOLUME)) {
				if (set) setting.mic_volume = str2int(val);
				return int2str(setting.mic_volume);
			}
			else if (strequals(key, KEY_MCHS_VOLUME)) {
				if (set) setting.mchs_volume = str2int(val);
				return int2str(setting.mchs_volume);
			}
			else if (strequals(key, KEY_PD_COUNT)) {
				if (set) {
					setting.pd_count = str2int(val);
					fault_unset(FAULT_PD_CON_1);
					fault_unset(FAULT_PD_CON_2);
				}
				return int2str(setting.pd_count);
			}
			else if (strequals(key, KEY_UM_COUNT)) {
				if (set) {
					setting.um_count = str2int(val);
					fault_unset(FAULT_UM_CON_1);
					fault_unset(FAULT_UM_CON_2);
					fault_unset(FAULT_UM_CON_3);
					fault_unset(FAULT_UM_CON_4);
					fault_unset(FAULT_UM_CON_5);
					fault_unset(FAULT_UM_CON_6);
				}
				return int2str(setting.um_count);
			}
			else if (strequals(key, KEY_LOGFILE_SIZE)) {
				if (set) {
					setting.log_file_size = str2ul(val);
				}
				return ul2str(setting.log_file_size);

			}
			else if (strequals(key, KEY_MIC_FIX)) {
				if (set) setting.mic_is_fix = str2bool(val);
				return bool2str(setting.mic_is_fix);
			}
			else if (strcmp_s(key, strlen(key), "um_")) { //Распределение линий вещания между усилителями мощности (УМ)
				ptr = key + strlen("um_");
				ival = str2int(ptr); //номер линии
				if ((ival < 1) || (ival > 24)) return "0";
				ival--;

				if (set) {
					ival2 = str2int(val);
					switch (ival2) {
						case 0:
							CLRBIT(setting.mkk_um1, ival);
							CLRBIT(setting.mkk_um2, ival);
						break;
						case 1:
							SETBIT(setting.mkk_um1, ival);
							CLRBIT(setting.mkk_um2, ival);
						break;
						case 2:
							CLRBIT(setting.mkk_um1, ival);
							SETBIT(setting.mkk_um2, ival);
						break;
						default: break;
					}
				}
				return GETBIT(setting.mkk_um1, ival) ? "1" : (GETBIT(setting.mkk_um2, ival) ? "2" : "0");;
			}
			else if (strcmp_s(key, strlen(key), "lzn_")) { //Номера зон
				ptr = key + strlen("lzn_");
				ival = str2int(ptr);
				if ((ival < 1) || (ival > 12)) return "0";
				//if (set) setting.zone_numbers[ival] = str2int(val);
				//return int2str(setting.zone_numbers[ival]);
			}
			else if (strcmp_s(key, strlen(key), "lzd_")) { //Описание зон
				ptr = key + strlen("lzd_");
				ival = str2int(ptr);
				if ((ival < 1) || (ival > 12)) return "";
				//if (set) strcpy_s(setting.zone_descriptions[ival], sizeof(setting.zone_descriptions[ival]), val);
				//return setting.zone_descriptions[ival];
			}
			else if (strcmp_s(key, strlen(key), "group_")) {
				ptr = key + strlen("group_");
				ival = str2int(ptr);
				if (set) setting.groups[ival] = str2ul(val);
				return ul2str(setting.groups[ival]);
			}
			else if (strequals(key, "umad")) {
				tmp_value = str2int(val);
				return int2str(tmp_value);
			}
			else if (strequals(key, "umas")) {
				ival = str2int(val);
				um_snd_set_addr(tmp_value, ival);
				return int2str(ival);
			}
			else if (strequals(key, "int_spk")) {
				if (set) {
					setting.int_spk = str2int(val);
					internal_speaker_set_mux(setting.int_spk);
				}
				return int2str(setting.int_spk);
			}
			else return NULL;

		break;

		//------------------------- Контроль исправности ------------------------------------------
		case CGI_SCRIPT_CONTROL_NUM:


			if (strcmp_s(key, strlen(key), "msl_")) { //Контроль исправности линий звукового оповещения - измерение
				ptr = key + strlen("msl_");
				ival = str2int(ptr);
				if ((ival < 1) || (ival > 24)) return "";
				ival--;
				if (GETBIT(setting.mkk_sound_lines, ival)) {

					if (GETBIT(pu_enable_lines, ival))
					{
						return  get_lin_state_html(mkk_rcv_sound_old[ival]);
					}
					else {
						mkk_rcv_sound_old[ival] = mkk_insig_lin_replay.sound[ival];
						return  get_lin_state_html(mkk_insig_lin_replay.sound[ival]);
					}
				}
				else return  get_lin_state_html(MKK_LIN_STATE_UNKNOW);
			}
			else if (strcmp_s(key, strlen(key), "mis_")) { //Контроль исправности входов сигнализации - измерение
				ptr = key + strlen("msl_");
				ival = str2int(ptr);
				if ((ival < 1) || (ival > 24)) return "";
				ival--;
				if GETBIT(setting.mkk_insig_lines, ival) return  get_insig_state_html(mkk_insig_lin_replay.insig[ival]);
				else return  get_insig_state_html(MKK_INSIG_STATE_UNKNOW);
			}
			else if (strequals(key, "moi_1")) { //Контроль исправности входов дополнительных функций - измерение
				if GETBIT(setting.mkk_start_lines, 0)	return  get_start_state_html(mkk_insig_lin_replay.start[0]);
				else return  get_start_state_html(MKK_START_STATE_UNKNOW);
			}
			else if (strequals(key, "moi_2")) { //Контроль исправности входов дополнительных функций - измерение
				if GETBIT(setting.mkk_start_lines, 1)	return  get_start_state_html(mkk_insig_lin_replay.start[1]);
				else return  get_start_state_html(MKK_START_STATE_UNKNOW);
			}
			else if (strequals(key, "csls")) { //Контроль исправности линий звукового оповещения - включение
				if (set) {
					setting.mkk_sound_lines = str2ul(val);
					change_csl = TRUE;
				}
				return  ul2str(setting.mkk_sound_lines);
			}
			else if (strequals(key, "ciss")) { //Контроль исправности входов сигнализации - включение
				if (set) {
					setting.mkk_insig_lines = str2ul(val);
					change_cis = TRUE;
				}
				return ul2str(setting.mkk_insig_lines);
			}
			else if (strequals(key, "lsl")) { //Контроль исправности линий звукового оповещения - допуск
				ptr = key + strlen("lsl");
				ival = str2int(ptr);
				if (set) {
					setting.mkk_procent = str2ul(val);
					mkk_send_lin_met(UART2_MUX_MKKL_1, setting.mkk_procent);
				}
				return  ul2str(setting.mkk_procent);
			}

			else if (strequals(key, "coi_1")) { //Контроль исправности входов дополнительных функций -включение
				if (set) {
					if (str2bool(val)) SETBIT(setting.mkk_start_lines, 0);
					else CLRBIT(setting.mkk_start_lines, 0);
					change_cis = TRUE;
				}
				return  GETBIT(setting.mkk_start_lines, 0) ? "true" : "false";
			}
			else if (strequals(key, "coi_2")) { //Контроль исправности входов дополнительных функций -включение
				if (set) {
					if (str2bool(val)) SETBIT(setting.mkk_start_lines, 1);
					else CLRBIT(setting.mkk_start_lines, 1);
					change_cis = TRUE;
				}
				return  GETBIT(setting.mkk_start_lines, 1) ? "true" : "false";
			}

			else if (strequals(key, "pw_220")) { //Авария 220
				if (mup_errrors == 0xFF) return HTML_SQR_GRAY;
				if (GETBIT(mup_errrors, MUP_BIT_ERROR_220V)) return  HTML_SQR_RED;
				else return  HTML_SQR_GREEN;


				//if (fault_is_set(FAULT_MUP_220V)) return  HTML_SQR_RED;
				//else return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "pw_akb")) { //Авария АКБ
				if (mup_errrors == 0xFF) return HTML_SQR_GRAY;
				if (GETBIT(mup_errrors, MUP_BIT_ERROR_ACC)) return  HTML_SQR_RED;
				else return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "pw_charge")) { //АКБ разряжен
				if (mup_errrors == 0xFF) return HTML_SQR_GRAY;
				if (GETBIT(mup_errrors, MUP_BIT_DISCHARGE_ACC)) return  HTML_SQR_RED;
				else return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "pw_cap")) { //Потеря емкости
				if (mup_errrors == 0xFF) return HTML_SQR_GRAY;
				if (GETBIT(mup_errrors, MUP_BIT_CAPACITY_ACC)) return  HTML_SQR_RED;
				else return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "pw_temp")) { //Авария температуры БП
				if (mup_errrors == 0xFF) return HTML_SQR_GRAY;
				if (GETBIT(mup_errrors, MUP_BIT_TD_BP)) return  HTML_SQR_RED;
				else return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "pdcon1")) { //Соединение ПД1
				switch (setting.pd_count) {
					case 1:
					case 3:
						if (fault_is_set(FAULT_PD_CON_1)) return  HTML_SQR_RED;
						else return  HTML_SQR_GREEN;
					break;
					default: return HTML_SQR_GRAY;
				}

			}
			else if (strequals(key, "pdcon2")) { //Соединение ПД2
				switch (setting.pd_count) {
					case 2:
					case 3:
						if (fault_is_set(FAULT_PD_CON_2)) return  HTML_SQR_RED;
						else return  HTML_SQR_GREEN;
					break;
					default: return HTML_SQR_GRAY;
				}

			}
			else if (strequals(key, "pdring")) { //Кольцо ПД
				switch (setting.pd_count) {
					case 1:
					case 2:
					case 3:
						if (fault_is_set(FAULT_PD_RING)) return  HTML_SQR_RED;
						else return  HTML_SQR_GREEN;
					break;
					default: return HTML_SQR_GRAY;
				}

			}
			else if (strcmp_s(key, strlen(key), "umf_")) { //Неисправность УМ1 - УМ6
				ptr = key + strlen("umf_");
				ival = str2int(ptr);
				if (ival > setting.um_count) return HTML_SQR_GRAY;
				if (fault_is_set(FAULT_UM_CON_1 + ival - 1)) return  HTML_SQR_RED;
				if (fault_is_set(FAULT_UM_PROTECT_CH1_1 + ival - 1)) return  HTML_SQR_RED;
				if (fault_is_set(FAULT_UM_PROTECT_CH2_1 + ival - 1)) return  HTML_SQR_RED;
				if (fault_is_set(FAULT_UM_BATTERY_1 + ival - 1)) return  HTML_SQR_RED;
				if (fault_is_set(FAULT_UM_LINE_OUTAGE_1 + ival - 1)) return  HTML_SQR_RED;
				if (fault_is_set(FAULT_UM_CH1_OUTAGE_1 + ival - 1)) return  HTML_SQR_RED;
				return  HTML_SQR_GREEN;
			}
			else if (strequals(key, "fums")) {
				get_or_set_item_buf[0] = '\0';
				for (ival =  FAULT_UM_CON_1; ival <=  FAULT_UM_TEST_LINE2_6; ival++) {
					if (!fault_is_set(ival)) continue;
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), fault2str(ival));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
				}
				return get_or_set_item_buf;
			}
			else if (strequals(key, "fall")) {
				get_or_set_item_buf[0] = '\0';
				for (ival =  0; ival <= 127; ival++) {
					if (!fault_is_set(ival)) continue;
					if (ival == FAULT_PD_RING) continue;
					if ((ival >= FAULT_PD_CON_1) && (ival <= FAULT_PD_CON_2)) continue;
					if ((ival >= FAULT_MUP_220V) && (ival <= FAULT_MUP_TD_ACC)) continue;
					if ((ival >= FAULT_MKK_START_1) && (ival <= FAULT_MKK_START_2)) continue;
					if ((ival >= FAULT_MKK_INSIG_1) && (ival <= FAULT_MKK_INSIG_24)) continue;
					if ((ival >= FAULT_MKK_SOUND_1) && (ival <= FAULT_MKK_SOUND_24)) continue;
					if ((ival >= FAULT_UM_CON_1) && (ival <= FAULT_UM_TEST_LINE2_6)) continue;
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), fault2str(ival));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
				}
				return get_or_set_item_buf;
			}
			else if (strequals(key, "fsound")) {
				get_or_set_item_buf[0] = '\0';
				for (ival =  FAULT_MKK_SOUND_1; ival <=  FAULT_MKK_SOUND_24; ival++) {
					if (!fault_is_set(ival)) continue;
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), fault2str(ival));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), " ");
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), mkk_get_lin_state_str(ival - FAULT_MKK_SOUND_1));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
				}
				return get_or_set_item_buf;
			}

			else if (strequals(key, "finsig")) {
				get_or_set_item_buf[0] = '\0';
				for (ival = FAULT_MKK_INSIG_1; ival <=  FAULT_MKK_INSIG_24; ival++) {
					if (!fault_is_set(ival)) continue;
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), fault2str(ival));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), " ");
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), mkk_get_insig_state_str(ival - FAULT_MKK_INSIG_1));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
				}
				return get_or_set_item_buf;
			}

			else if (strequals(key, "fstart")) {
				get_or_set_item_buf[0] = '\0';
				for (ival = FAULT_MKK_START_1; ival <=  FAULT_MKK_START_2; ival++) {
					if (!fault_is_set(ival)) continue;
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), fault2str(ival));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), " ");
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), mkk_get_start_state_str(ival - FAULT_MKK_START_1));
					strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>");
				}
				return get_or_set_item_buf;
			}
			else if (strequals(key, KEY_LINES_COUNT)) {
				if (set) setting.lines_count = str2int(val);
				return int2str(setting.lines_count);
			}
			else if (strequals(key, KEY_CONTROL_TIME)) {
				if (set) setting.control_time_sec = str2ul(val);
				return ul2str(setting.control_time_sec);
			}



			else return NULL;

	break;

	//------------------------- Режим работы ------------------------------------------
	case CGI_SCRIPT_MODE_NUM:

		//Режим МЧС
		if (strequals(key, "mchs_lines")) { //Режим Контроль МЧС
			if (set) {
				setting.mode_mchs.sound_lines = str2ul(val);
			}
			return ul2str(setting.mode_mchs.sound_lines);
		}

		//Звуковое вещание
		else if (strequals(key, "sound_file")) {
			if (set) strcpy_s(setting.mode_sound.file, sizeof(setting.mode_sound.file), val);
			return setting.mode_sound.file;
		}
		else if (strequals(key, "sound_src")) {
			if (set) setting.mode_sound.soundsrc = str2int(val);
			return int2str(setting.mode_sound.soundsrc);
		}
		else if (strequals(key, "sound_tb")) {
			if (set) setting.mode_sound.tilibom = str2bool(val);
			return bool2str(setting.mode_sound.tilibom);
		}



		else if (strequals(key, "sound_lines")) {
			if (set) {
				setting.mode_sound.sound_lines = str2ul(val);
			}
			return ul2str(setting.mode_sound.sound_lines);
		}




		//Ручной ПУСК
		else if (strequals(key, "manual_tb_mic")) {
			if (set) setting.mode_manual_tilibom_mic = str2bool(val);
			return bool2str(setting.mode_manual_tilibom_mic);
		}
		else if (strequals(key, "manual_tb_file")) {
			if (set) setting.mode_manual_tilibom_file = str2bool(val);
			return bool2str(setting.mode_manual_tilibom_file);
		}



/*
		else if (strequals(key, "manual_uo")) {
		//if (strlen(val)) mode_manual_uo = str2int(val);
			if (set) {
				mode_manual_uo = str2int(val);
				ulog_fmt("set manual_uo %d"CRLF, mode_manual_uo);
			}
			else {
				if (val && str2int(val)) {
					mode_manual_uo = str2int(val);
					ulog_fmt("get manual_uo %d"CRLF, mode_manual_uo);
				}
			}
			return int2str(mode_manual_uo);
		}
		else if (strequals(key, "manual_enable")) {
			if (set) setting.mode_manual[mode_manual_uo].enable = str2bool(val);
			return bool2str(setting.mode_manual[mode_manual_uo].enable);
		}
		else if (strequals(key, "manual_delay")) {
			if (set) setting.mode_manual[mode_manual_uo].delay = str2int(val);
			return int2str(setting.mode_manual[mode_manual_uo].delay);
		}
		else if (strequals(key, "manual_repeat")) {
			if (set) setting.mode_manual[mode_manual_uo].repeat = str2int(val);
			return int2str(setting.mode_manual[mode_manual_uo].repeat);
		}
		else if (strequals(key, "manual_file")) {
			if (set) strcpy_s(setting.mode_manual[mode_manual_uo].file, sizeof(setting.mode_manual[mode_manual_uo].file), val);
			return setting.mode_manual[mode_manual_uo].file;
		}
		else if (strequals(key, "manual_cycle")) {
			if (set) setting.manual_cycle = str2bool(val);
			return bool2str(setting.manual_cycle);
		}
*/

		//Режим ПОЖАР
		else if (strequals(key, "fire_uo")) {
			//if (strlen(val)) mode_manual_uo = str2int(val);
			if (set) {
				mode_fire_uo = str2int(val);
				ulog_fmt("set fire_uo %d"CRLF, mode_fire_uo);
			}
			else {
				if (val && str2int(val)) {
					mode_fire_uo = str2int(val);
					ulog_fmt("get fire_uo %d"CRLF, mode_fire_uo);
				}
			}
			return int2str(mode_fire_uo);
		}
		else if (strequals(key, "fire_enable")) {
			if (set) setting.mode_fire[mode_fire_uo].enable = str2bool(val);
			return bool2str(setting.mode_fire[mode_fire_uo].enable);
		}
		else if (strequals(key, "fire_tb")) {
			if (set) setting.mode_fire[mode_fire_uo].tilibom = str2bool(val);
			return bool2str(setting.mode_fire[mode_fire_uo].tilibom);
		}
		else if (strequals(key, "fire_delay")) {
			if (set) setting.mode_fire[mode_fire_uo].delay = str2int(val);
			return int2str(setting.mode_fire[mode_fire_uo].delay);
		}
		else if (strequals(key, "fire_repeat")) {
			if (set) setting.mode_fire[mode_fire_uo].repeat = str2int(val);
			return int2str(setting.mode_fire[mode_fire_uo].repeat);
		}
		else if (strequals(key, "fire_file")) {
			if (set) strcpy_s(setting.mode_fire[mode_fire_uo].file, sizeof(setting.mode_fire[mode_fire_uo].file), val);
			return setting.mode_fire[mode_fire_uo].file;
		}
		else if (strcmp_s(key, strlen(key), "fls_")) {
			ptr = key + strlen("fls_");
			ival = str2int(ptr);
			if (set) {
				setting.mode_fire[mode_fire_uo].input_lines[ival] = str2ul(val);
			}
			return ul2str(setting.mode_fire[mode_fire_uo].input_lines[ival]);
		}
		else if (strequals(key, "fire_cycle")) {
			if (set) setting.fire_cycle = str2bool(val);
			return bool2str(setting.fire_cycle);
		}
		else if (strequals(key, KEY_LINES_COUNT)) {
			if (set) setting.lines_count = str2int(val);
			return int2str(setting.lines_count);
		}
		else return NULL;

	break;

	//------------------------- Фаловый менеджер ------------------------------------------
	case CGI_SCRIPT_FILEMANAGER_NUM:

		if (strequals(key, "local_files")) {  //локальные файлы
			if (fatfs_mount) {
				get_or_set_item_buf[0] = '\0';
				all_files_size = 0;
				f_enum_files(F_DEFAULT_PATH, get_or_set_item_f_enum_files_proc, NULL);
				strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), "<br>Занято: ");
				strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), ull2str(all_files_size / 1024));
				strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), " КБ<br>Свободно: ");
				strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), ull2str( (flash_full_size() - all_files_size) / 1024));
				strcat_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), " КБ");
				return get_or_set_item_buf;
			}
			else return "ОШИБКА ФАЙЛОВОЙ СИСТЕМЫ";
		}
		else if (strequals(key, "local_delete")) {
			if (!val) return "Отсутствует имя";
			return FR_OK == f_unlink(val) ? "Файл удален" : "Ошибка удаления";
		}
		else if (strequals(key, "local_format")) {
			f_format_and_mount(&f_global_fatfs, F_MKFS_FMT_DEFAULT);
			return "";

		}
		else if (strequals(key, "flash_files")) { //файлы на внешней флэш
			if (external_flash_is_mount) {
				get_or_set_item_buf[0] = '\0';
				external_flash_enum_files(CH378_ENUM_ROOT_PATH, external_flash_get_or_set_item_fun, NULL);
				return get_or_set_item_buf;
			}
			else return "ВНЕШНЯЯ ФЛЭШ НЕ ПОДКЛЮЧЕНА";
		}
		else if (strequals(key, "flash_copy")) { //копирование мз внешней флэш на локальную флэш


			if ( strequals(val, "pu.bin") && (safety_get_level_from_pin_code() < 4) ) {//Перепрошивка доступна только уровню 4.
				return "Ошибка уровня доступа";
			}


			if (external_flash_copy_to_local(val)) return "Файл скопирован";
			else return "Ошибка копирования";
		}
		else if (strequals(key, "local_copy")) { //копирование мз локальной флэш на внешнюю флэш
			if (external_flash_copy_from_local(val)) return "Файл скопирован";
			else return "Ошибка копирования";
		}
		else if (strequals(key, "local_rename_old")) { //нереименовать файл
			strcpy_s(get_or_set_item_buf, sizeof(get_or_set_item_buf), val);
			return "";
		}
		else if (strequals(key, "local_rename_new")) { //нереименовать файл

			if (f_exists(get_or_set_item_buf)) {
				f_rename(get_or_set_item_buf, val);
			}
			return "";
		}
		else return NULL;

	break;


	//------------------------- Безопасность ------------------------------------------
	case CGI_SCRIPT_SAFETY_NUM:

		if (strcmp_s(key, strlen(key), "uname_")) {
			ptr = key + strlen("uname_");
			ival = str2int(ptr);
			if (set) {
				strcpy_s(setting.users[ival].name, sizeof(setting.users[ival].name), val);
			}
			return setting.users[ival].name;
		}
		else if (strcmp_s(key, strlen(key), "upin_")) {
			ptr = key + strlen("upin_");
			ival = str2int(ptr);
			if (set) {
				if (strlen(val) < 2) {
					setting.users[ival].pin_code[0] = '\0';
				}
				else {
					strcpy_s(setting.users[ival].pin_code, sizeof(setting.users[ival].pin_code), val);
				}
			}
			return setting.users[ival].pin_code;
		}
		else if (strcmp_s(key, strlen(key), "ulevel_")) {
			ptr = key + strlen("ulevel_");
			ival = str2int(ptr);
			if (set) {
				setting.users[ival].level = str2int(val);
			}
			return int2str(setting.users[ival].level);

		}


		//if (strequals(key, KEY_WEB_LOGIN)) {
		//	if (set) {
		//		strcpy_s(setting.web_login, sizeof(setting.web_login), val);
		//	}
		//	return setting.web_login;
		//}
		//else if (strequals(key,  KEY_WEB_PASSWORD)) {
		//	if (set) {
		//		strcpy_s(setting.web_password, sizeof(setting.web_password), val);
		//	}
		//	return setting.web_password;
		//}
		else return NULL;

	break;



	//------------------------- Вход в WEB ------------------------------------------
	case CGI_SCRIPT_AUTH_NUM:

		//if (strequals(key, KEY_INPUT_WEB_LOGIN)) {
		//	if (set) {
		//		strcpy_s(safety_web_login, sizeof(safety_web_login), val);
		//		http_session_tick = HAL_GetTickNoZero();
		//	}
		//	return safety_web_login;
		//}
		//else if (strequals(key,  KEY_INPUT_WEB_PASSWORD)) {
		//	if (set) {
		//		strcpy_s(safety_web_password, sizeof(safety_web_password), val);
		//		http_session_tick = HAL_GetTickNoZero();
		//	}
		//	return safety_web_password;
		//}
		//else
			return NULL;

	break;

	//------------------------- Журнал ------------------------------------------
	//else if (strcmp_s(key, strlen(key), "journal_")) {
	//	return "KU<br>";
	//}

	//------------------------- Сетевый функции ----------------------------------------

	case CGI_SCRIPT_NETFUNC_NUM:

		//пожар
		if (strcmp_s(key, strlen(key), "nfa_")) {
			ptr = key + strlen("nfa_");
			ival = str2int(ptr);

			if (set) {
				setting.netfunc_fire[ival].pu_addr  = str2ul(val);
			}

			return ul2str(setting.netfunc_fire[ival].pu_addr);
		}
		else if (strcmp_s(key, strlen(key), "nfs_")) {
			ptr = key + strlen("nfs_");
			ival = str2int(ptr);

			if (set) {
				setting.netfunc_fire[ival].sound_lines  = str2ul(val);
			}

			return ul2str(setting.netfunc_fire[ival].sound_lines);
		}
		else if (strcmp_s(key, strlen(key), "nfl_")) {
			ptr = key + strlen("nfl_");
			ival = str2int(ptr);

			if (set) {
				setting.netfunc_fire[ival].fire_line  = str2int(val);
			}

			return int2str(setting.netfunc_fire[ival].fire_line);
		}
		else if (strcmp_s(key, strlen(key), "nff_")) {
			ptr = key + strlen("nff_");
			ival = str2int(ptr);

			if (set) {
				strcpy_s(setting.netfunc_fire[ival].file, sizeof(setting.netfunc_fire[ival].file), val);
			}

			return setting.netfunc_fire[ival].file;
		}

		//МЧС
		else if (strequals(key, "nmchsa")) {

			if (set) {
				setting.netfunc_mchs.pu_addr = str2int(val);
			}

			return int2str(setting.netfunc_mchs.pu_addr);

		}
		else if (strequals(key, "nmchsl")) {
			if (set) {
				setting.netfunc_mchs.sound_lines = str2ul(val);
			}

			return ul2str(setting.netfunc_mchs.sound_lines);

		}
		else if (strequals(key, KEY_LINES_COUNT)) {
			if (set) setting.lines_count = str2int(val);
			return int2str(setting.lines_count);
		}
		else return NULL;

	break;

	case CGI_SCRIPT_ERROR_USER_LEVEL_NUM:

		if (strequals(key, "pincode")) {

			if (set) {
				strcpy_s(safety_pin_code, sizeof(safety_pin_code), val);
				cm4core_send_to_CM7_cmd(CORE_CMD_CHANGE_SAFETY_PINCODE_CM7, (uint8_t *)safety_pin_code, strlen(safety_pin_code));
			}

			return safety_pin_code;

		}
	break;

	} //switch
	return NULL;
}


void get_item(char * key, char * val, void * params)
{
	//ulog_fmt("get key:%s val:%s"CRLF, key, val);

	add_json_key_value(key, get_or_set_item(key, val, FALSE), (char *)params, DATA_BUF_SIZE);
}


uint8_t get_cgi_num(char * path)
{
	if (strcmp(path, CGI_SCRIPT_INDEX) == 0) return CGI_SCRIPT_INDEX_NUM;
	if (strcmp(path, CGI_SCRIPT_NETWORK) == 0) return CGI_SCRIPT_NETWORK_NUM;
	if (strcmp(path, CGI_SCRIPT_SETTING) == 0) return CGI_SCRIPT_SETTING_NUM;
	if (strcmp(path, CGI_SCRIPT_CONTROL) == 0) return CGI_SCRIPT_CONTROL_NUM;
	if (strcmp(path, CGI_SCRIPT_MODE) == 0) return CGI_SCRIPT_MODE_NUM;
	if (strcmp(path, CGI_SCRIPT_FILEMANAGER) == 0) return CGI_SCRIPT_FILEMANAGER_NUM;
	if (strcmp(path, CGI_SCRIPT_SAFETY) == 0) return CGI_SCRIPT_SAFETY_NUM;
	if (strcmp(path, CGI_SCRIPT_AUTH) == 0) return CGI_SCRIPT_AUTH_NUM;
	if (strcmp(path, CGI_SCRIPT_NETFUNC) == 0) return CGI_SCRIPT_NETFUNC_NUM;
	if (strcmp(path, CGI_SCRIPT_ERROR_USER_LEVEL) == 0) return CGI_SCRIPT_ERROR_USER_LEVEL_NUM;
	return 0;

}


//аозврщает HTTP_OK или HTTP_FAILED	 или HTTP_RESET
uint8_t http_get_cgi_handler(uint8_t * uri_name, st_http_request * p_http_request, uint8_t * buf, uint32_t * file_len)
{
    char * ptr, * txbuf;


    //ulog(CRLF);
    //ulog((char *)p_http_request->URI);
    //ulog(CRLF);

    cgi_page_num = get_cgi_num((char *)uri_name);


	if (!cgi_page_num) return HTTP_FAILED;

	/*
	switch (cgi_page_num) {



		//http://192.168.0.84/index.cgi?name&version&netinfomac&netinfoip&netinfogw&netinfomask&netinfodns&netinfodhcp&modenow&ifaults
		case  CGI_SCRIPT_INDEX_NUM:
			snprintf((char *)buf, DATA_BUF_SIZE, "{\"name\":\"%s\",\"version\":\"%s\",\"netinfomac\":\"%s\",\"netinfoip\":\"%s\",\"netinfogw\":\"%s\",\"netinfomask\":\"%s\",\"netinfodns\":\"%s\",\"netinfodhcp\":\"%s\",\"modenow\":\"%s\",\"ifaults\":\"%d\"}",
					setting.name, VERSION,
					mac2str(setting.mac), ip2str(*(uint32_t *)net_info.ip), ip2str(*(uint32_t *)net_info.gw), ip2str(*(uint32_t *)net_info.sn), ip2str(*(uint32_t *)net_info.dns), (net_info.dhcp == NETINFO_DHCP ? "Включено" : "Выключено"),
					pu_mode2str(pu_mode), fault_count());
			*file_len = strlen((char *)buf);
		return  HTTP_OK;


		//http://192.168.0.84/network.cgi?mac&ip&gw&mask&dns&dhcp
		case CGI_SCRIPT_NETWORK_NUM:
			snprintf((char *)buf, DATA_BUF_SIZE, "{\"mac\":\"%s\",\"ip\":\"%s\",\"gw\":\"%s\",\"mask\":\"%s\",\"dns\":\"%s\",\"dhcp\":\"%s\"}",
					mac2str(setting.mac), ip2str(setting.ip), ip2str(setting.gw), ip2str(setting.mask), ip2str(setting.dns), bool2str(setting.dhcp));
			*file_len = strlen((char *)buf);
		return  HTTP_OK;

		//http://192.168.0.84/safety.cgi?pincode2&pincode3&pincode4&login&password
		//case CGI_SCRIPT_SAFETY_NUM:
		//	snprintf((char *)buf, DATA_BUF_SIZE, "{pincode2='%s'&pincode3='%s'&pincode4='%s'&login='%s'&password='%s'}",
		//			setting.pin_code_level_2, setting.pin_code_level_3, setting.pin_code_level_4, setting.web_login, setting.web_password);
		//	*file_len = strlen((char *)buf);

		return  HTTP_OK;
	}
	 */

	txbuf = (char *)buf;
	ptr = strstr((char *)p_http_request->URI, "?");
	if (!ptr) {
		strcpy(txbuf, "{}");
		*file_len = 2;
		return HTTP_OK;
	}
	ptr++;


    //ulog(CRLF);
    //ulog("GET IN ");
    //ulog(ptr);
    //ulog(CRLF);

	txbuf[0] = '{';
	txbuf[1] = '\0';
	parse_http_params(ptr, get_item, (void *)txbuf);
	strcat_s(txbuf, DATA_BUF_SIZE, "}");
	*file_len = strlen(txbuf);

    //ulog(CRLF);
    //ulog("GET OUT ");
    //ulog(txbuf);
    //ulog(CRLF);

	return HTTP_OK;
}


void set_item(char * key, char * val, void * params)
{
	add_json_key_value(key, get_or_set_item(key, val, TRUE), (char *)params, DATA_BUF_SIZE);
}


#include "mki.h"






uint8_t http_post_cgi_handler(uint8_t * uri_name, st_http_request * p_http_request, uint8_t * buf, uint32_t * file_len)
{
    char* txbuf;


    cgi_page_num = get_cgi_num((char *)uri_name);

	if (!cgi_page_num) return HTTP_FAILED;


    //ulog_fmt("uri_name=%s URI=%s http_request=%s"CRLF, uri_name, p_http_request->URI, http_request);


    //ulog(CRLF);
    //ulog((char *)p_http_request->POST_DATA);
    //ulog(CRLF);

	txbuf = (char *)buf;

	if ( (!p_http_request->POST_DATA) || (strlen((char *)p_http_request->POST_DATA) < 4) || !strstr_s((char *)p_http_request->POST_DATA,  DATA_BUF_SIZE, "=") ) {
		strcpy_s(txbuf, DATA_BUF_SIZE, "{\"result\":\"Ошибка входных данных\"}");
		*file_len = strlen(txbuf);
		return HTTP_OK;
	}


	txbuf[0] = '{';
	txbuf[1] = '\0';
	parse_http_params((char *)p_http_request->POST_DATA, set_item, (void *)txbuf);

	if (strlen(txbuf) < 4) {
		strcat_s(txbuf, DATA_BUF_SIZE, "\"result\":\"Ошибка парсинга входных данных\"");
    } else {
    	strcat_s(txbuf, DATA_BUF_SIZE, ", \"result\":\"Данные сохранены\"");
    }
	strcat_s(txbuf, DATA_BUF_SIZE, "}");
	*file_len = strlen(txbuf);

	//save_setting(SETTING_FILE_NAME);
	need_save_setting = TRUE;

	if (change_csl) { //Контроль исправности линий звукового оповещения - включение
		change_csl = FALSE;
		mkk_send_lin_on_from_setting(UART2_MUX_MKKL_1);
		if (setting.lines_count > 12) {
			mkk_send_lin_on_from_setting(UART2_MUX_MKKL_2);
		}
	    mkk_change();
	}

	if (change_cis) { //Контроль исправности входов сигнализации - включение
		change_cis = FALSE;

		mkk_send_insig_on_from_setting(UART2_MUX_MKKL_1);
		if (setting.lines_count > 12) {
			mkk_send_insig_on_from_setting(UART2_MUX_MKKL_2);
		}
	    mkk_change();
	}

	if (change_um) { //Распределение линий вещания между усилителями мощности (УМ)
		change_um = FALSE;
		//mkk_send_lin_from_setting(UART2_MUX_MKKL_1);
	}

	cm4core_change_setting();

	return HTTP_OK;
}


