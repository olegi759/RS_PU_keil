/*
 * common.c
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include "str_util.h"
#include "math_util.h"
#include "mki_data.h"
#include "setting_data.h"
#include <string.h>
#include <stdio.h>
#include "mkk_data.h"
#include "fault_data.h"
#include "logfile_util.h"
#include "stm_util.h"

setting_t setting = {0};


uint8_t faults[MAX_FAULTS / 8] = {0};

uint8_t date_time[5] = {0}; //rtc date time

uint32_t pu_enable_lines = 0;

pu_mode_t pu_mode = PU_MODE_DEJURNIY;

char safety_pin_code[9] = {'\0'};

shared_memory_t * sm = (shared_memory_t *)SM_BASE_ADDRESS;

pu_option_t pu_option = PU_OPTION_NONE;

mkk_insig_lin_replay_t mkk_insig_lin_replay = {0};


uint32_t stm32_uid[3] = {0};


void shared_memory_init(void)
{
	  ringbuff_init(&sm->rb_cm7_to_cm4, sm->rb_cm7_to_cm4_data, sizeof(sm->rb_cm7_to_cm4_data));
	  ringbuff_init(&sm->rb_cm4_to_cm7, sm->rb_cm4_to_cm7_data, sizeof(sm->rb_cm4_to_cm7_data));
}

char * get_date_time_str(void)
{
	static char __get_date_time_str[24];
	snprintf(__get_date_time_str, sizeof(__get_date_time_str), "20%02d-%02d-%02d %02d:%02d", date_time[0], date_time[1], date_time[2], date_time[3], date_time[4]);
	return __get_date_time_str;
}

char * get_time_str(void)
{
	static char __get_time_str[9];
	snprintf(__get_time_str, sizeof(__get_time_str), "%02d:%02d", date_time[3], date_time[4]);
	return __get_time_str;
}



uint32_t get_time_stamp(void)
{
	return ((2000 + date_time[0]) - 1970) * 31536000 + (date_time[1] - 1) *  2678400 + (date_time[2] - 1) * 86400 + date_time[3] * 3600 + date_time[4] * 60;

}



//Номер кнопки в название
const char * mki_get_key_name(uint8_t key)
{
	switch (key) {

		case MKI_KEY_1: return "1";
		case MKI_KEY_2: return "2";
		case MKI_KEY_3: return "3";
		case MKI_KEY_4: return "4";
		case MKI_KEY_5: return "5";
		case MKI_KEY_6: return "6";
		case MKI_KEY_7: return "7";
		case MKI_KEY_8: return "8";
		case MKI_KEY_9: return "9";
		case MKI_KEY_10: return "10";
		case MKI_KEY_11: return "11";
		case MKI_KEY_12: return "12";
		case MKI_KEY_13: return "13";
		case MKI_KEY_14: return "14";
		case MKI_KEY_15: return "15";
		case MKI_KEY_16: return "16";
		case MKI_KEY_17: return "17";
		case MKI_KEY_18: return "18";
		case MKI_KEY_19: return "19";
		case MKI_KEY_20: return "20";
		case MKI_KEY_21: return "21";
		case MKI_KEY_22: return "22";
		case MKI_KEY_23: return "23";
		case MKI_KEY_24: return "24";


		case MKI_KEY_TEST: return "TEST";
		case MKI_KEY_MUTE: return "MUTE";
		case MKI_KEY_RESET: return "RESET";
		case MKI_KEY_MIC: return "MIC";
		case MKI_KEY_MENU: return "MENU";
		case MKI_KEY_LEFT: return "LEFT";
		case MKI_KEY_UP: return "UP";
		case MKI_KEY_RIGHT: return "DOWN";
		case MKI_KEY_DELETE: return "DELETE";
		case MKI_KEY_EXIT: return "EXIT";
		case MKI_KEY_DOWN: return "DOWN";
		case MKI_KEY_ENTER: return "ENTER";
		case MKI_KEY_ZONES: return "ZONES";
		case MKI_KEY_GROUPS: return "GROUPS";
		case MKI_KEY_ALLZONES: return "ALLZONES";
		case MKI_KEY_MANUALSTART: return "MANUALSTART";
		default: return "UNKNOW";
	}

}


//корректен ли введенный пин код
uint8_t safety_get_level_from_pin_code(void)
{
	int i;
	BOOL flag;

	if (strequals(safety_pin_code, FACTORY_PIN_CODE_LEVEL_5)) return 5;

	flag = TRUE;
	for (i = 1; i <= 10; ++i) {
		if (setting.users[i].pin_code[0] != '\0') { flag = FALSE; break; }
	}
	if (flag) return 4; //не заполнен ни один

	for (i = 1; i <= 10; ++i) {
		if (setting.users[i].pin_code[i] == '\0') continue;
		if (strequals(safety_pin_code, setting.users[i].pin_code)) return setting.users[i].level;
	}

	return 1;
}


//имя пользователя из пин кода
char * safety_get_name_from_pin_code(void)
{
	int i;

	if (strequals(safety_pin_code, FACTORY_PIN_CODE_LEVEL_5)) return "root";


	for (i = 1; i <= 10; ++i) {
		if (setting.users[i].pin_code[i] == '\0') continue;
		if (strequals(safety_pin_code, setting.users[i].pin_code)) return setting.users[i].name;
	}

	return NULL;
}


const char * mkk_lin_state2str(uint8_t val)
{
	switch (val) {
			//Неизвестно
			case MKK_LIN_STATE_UNKNOW: return "Неизвестно";
			//Линия исправна
			case MKK_LIN_STATE_OK: return "Линия исправна";
			//Обрыв
			case MKK_LIN_STATE_BREAK: return "Обрыв";
			//Линия неисправна, сопротивление линии увеличено (или сопротивление линии выше нормы при инсталляции)
			case MKK_LIN_STATE_UP: return "Сопротивление выше нормы";
			//Линия неисправна, сопротивление линии снижено (или сопротивление линии меньше нормы при инсталляции)
			case MKK_LIN_STATE_DOWN: return "Сопротивление ниже нормы";
			//КЗ линии
			case MKK_LIN_STATE_SHORT_CIRCUIT: return "КЗ на линии";
			default: return "";
	}
}




const char * mkk_insig_state2str(uint8_t val)
{
	switch (val) {
		//Неизвестно
		case MKK_INSIG_STATE_UNKNOW: return "Неизвестно";
		//Неисправность обрыв, сопротивление линии более 5,4 кОм
		case MKK_INSIG_STATE_BREAK: return  "Обрыв";
		//Норма сигнала нет, сопротивление 4,7 кОм (4…5,4 кОм)
		case MKK_INSIG_STATE_NORM_NO_SIGNAL: return "Норма сигнала нет";
		//Неисправность зона неопределенности, сопротивление 3…4 кОм
		case MKK_INSIG_STATE_UNCERTAINTY: return  "Зона неопределенности";
		//Норма сигнал есть, сопротивление 2,35 кОм (2…3 кОм)
		case MKK_INSIG_STATE_NORM_YES_SIGNAL: return "Норма сигнал есть";
		//Неисправность КЗ, сопротивление линии менее 2 кОм
		case MKK_INSIG_STATE_SHORT_CIRCUIT: return  "КЗ на линии";
		default: return "";
	}
}




const char * mkk_start_state2str(uint8_t val)
{
	switch (val) {
		//Неизвестно
		case MKK_START_STATE_UNKNOW: return "Неизвестно";
		//Неисправность обрыв, сопротивление линии более 5,4 кОм
		case MKK_START_STATE_BREAK: return  "Обрыв";
		//Норма сигнала нет, сопротивление 4,7 кОм (4…5,4 кОм)
		case MKK_START_STATE_NORM_NO_SIGNAL: return "Норма сигнала нет";
		//Неисправность зона неопределенности, сопротивление 3…4 кОм
		case MKK_START_STATE_UNCERTAINTY: return  "Эона неопределенности";
		//Норма сигнал есть, сопротивление 2,35 кОм (2…3 кОм)
		case MKK_START_STATE_NORM_YES_SIGNAL: return "Норма сигнал есть";
		//Неисправность КЗ, сопротивление линии менее 2 кОм
		case MKK_START_STATE_SHORT_CIRCUIT: return  "КЗ на линии";
		default: return "";
	}
}


//Из pu_mode_t в стринг
const char * pu_mode2str(pu_mode_t mode)
{
	switch (mode) {
		case PU_MODE_DEJURNIY: return "ДЕЖУРНЫЙ";
		case PU_MODE_VESHANIE_PD: return "ВЕЩ.ПД";
		case PU_MODE_ZVUKOVOE_VESHANIE: return "ЗВУК.ВЕЩ.";
		case PU_MODE_VESHANIE_MCHS: return "ВЕЩ.МЧС";
		case PU_MODE_VESHANIE_POJAR: return "ВЕЩ.ПОЖАР";
		case PU_MODE_RUCHNOI_PUSK: return "РУЧ.ПУСК";
		case PU_MODE_RUCHNOI_PUSK_PD: return "РУЧ.ПУСК.ПД";
		case PU_MODE_UPRAVLENIE_DRUGIMI_PU: return "СЕТ.УПРАВЛ.";
		default: return "НЕИЗЕСТНО";
	}

}

//Из pu_mode_t в цвет
const uint32_t pu_mode2ledcolor(pu_mode_t mode)
{
	switch (mode) {
		case PU_MODE_DEJURNIY: return LED_COLOR_BLACK;
		case PU_MODE_VESHANIE_PD: return LED_COLOR_AQUA;
		case PU_MODE_ZVUKOVOE_VESHANIE: return LED_COLOR_BLUE;
		case PU_MODE_VESHANIE_MCHS: return LED_COLOR_YELLOW;
		case PU_MODE_VESHANIE_POJAR: return LED_COLOR_RED;
		case PU_MODE_RUCHNOI_PUSK: return LED_COLOR_GREEN;
		case PU_MODE_RUCHNOI_PUSK_PD: return LED_COLOR_GREEN;
		case PU_MODE_UPRAVLENIE_DRUGIMI_PU: return LED_COLOR_FUCHSIA;
		default: return LED_COLOR_BLACK;
	}

}


//Установлена ли ошибка
BOOL fault_is_set(uint8_t num)
{
	return bitget(faults, num) ? TRUE : FALSE;
}

//Первая ошибка
uint8_t fault_first(void)
{
	uint8_t i;

	for (i = 0; i < MAX_FAULTS; ++i) {
		if (fault_is_set(i)) return i;
	}
	return 0xFF;
}

//Всего ошибок
uint8_t fault_count(void)
{
	uint8_t i, cnt;

	cnt = 0;
	for (i = 0; i < MAX_FAULTS; ++i) {
		if (fault_is_set(i)) cnt++;
	}
	return cnt;
}

//Имеются ли ошибки
//BOOL fault_have(void)
//{
//	return fault_count() > 0;
//}

//Ошибки в строку
const char * fault2str(uint8_t num)
{
	switch (num) {
		case FAULT_MUP_220V: return  MUP_BIT_ERROR_220V_STR;
		//авария источника
		case FAULT_MUP_ERROR_PS: return MUP_BIT_ERROR_PS_STR;
		//авария акк
		case FAULT_MUP_ERROR_ACC: return MUP_BIT_ERROR_ACC_STR;
		//акк разряжен
		case FAULT_MUP_DISCHARGE_ACC: return MUP_BIT_DISCHARGE_ACC_STR;
		//потеря емкости
		case FAULT_MUP_CAPACITY_ACC: return MUP_BIT_CAPACITY_ACC_STR;
		//авария цепей измерений
		case FAULT_MUP_METERAGE: return MUP_BIT_METERAGE_STR;
		//авария тепература БП
		case FAULT_MUP_TD_BP: return MUP_BIT_TD_BP_STR;
		//авария тепература акк
		case FAULT_MUP_TD_ACC: return MUP_BIT_TD_ACC_STR;


		case FAULT_MKK_INSIG_1: return MKK_INSIG_FAULT"1";
		case FAULT_MKK_INSIG_2: return MKK_INSIG_FAULT"2";
		case FAULT_MKK_INSIG_3: return MKK_INSIG_FAULT"3";
		case FAULT_MKK_INSIG_4: return MKK_INSIG_FAULT"4";
		case FAULT_MKK_INSIG_5: return MKK_INSIG_FAULT"5";
		case FAULT_MKK_INSIG_6: return MKK_INSIG_FAULT"6";
		case FAULT_MKK_INSIG_7: return MKK_INSIG_FAULT"7";
		case FAULT_MKK_INSIG_8: return MKK_INSIG_FAULT"8";
		case FAULT_MKK_INSIG_9: return MKK_INSIG_FAULT"9";
		case FAULT_MKK_INSIG_10: return MKK_INSIG_FAULT"10";
		case FAULT_MKK_INSIG_11: return MKK_INSIG_FAULT"11";
		case FAULT_MKK_INSIG_12: return MKK_INSIG_FAULT"12";
		case FAULT_MKK_INSIG_13: return MKK_INSIG_FAULT"13";
		case FAULT_MKK_INSIG_14: return MKK_INSIG_FAULT"14";
		case FAULT_MKK_INSIG_15: return MKK_INSIG_FAULT"15";
		case FAULT_MKK_INSIG_16: return MKK_INSIG_FAULT"16";
		case FAULT_MKK_INSIG_17: return MKK_INSIG_FAULT"17";
		case FAULT_MKK_INSIG_18: return MKK_INSIG_FAULT"18";
		case FAULT_MKK_INSIG_19: return MKK_INSIG_FAULT"19";
		case FAULT_MKK_INSIG_20: return MKK_INSIG_FAULT"20";
		case FAULT_MKK_INSIG_21: return MKK_INSIG_FAULT"21";
		case FAULT_MKK_INSIG_22: return MKK_INSIG_FAULT"22";
		case FAULT_MKK_INSIG_23: return MKK_INSIG_FAULT"23";
		case FAULT_MKK_INSIG_24: return MKK_INSIG_FAULT"24";

		case FAULT_MKK_SOUND_1: return MKK_SOUND_FAULT"1";
		case FAULT_MKK_SOUND_2: return MKK_SOUND_FAULT"2";
		case FAULT_MKK_SOUND_3: return MKK_SOUND_FAULT"3";
		case FAULT_MKK_SOUND_4: return MKK_SOUND_FAULT"4";
		case FAULT_MKK_SOUND_5: return MKK_SOUND_FAULT"5";
		case FAULT_MKK_SOUND_6: return MKK_SOUND_FAULT"6";
		case FAULT_MKK_SOUND_7: return MKK_SOUND_FAULT"7";
		case FAULT_MKK_SOUND_8: return MKK_SOUND_FAULT"7";
		case FAULT_MKK_SOUND_9: return MKK_SOUND_FAULT"9";
		case FAULT_MKK_SOUND_10: return MKK_SOUND_FAULT"10";
		case FAULT_MKK_SOUND_11: return MKK_SOUND_FAULT"11";
		case FAULT_MKK_SOUND_12: return MKK_SOUND_FAULT"12";
		case FAULT_MKK_SOUND_13: return MKK_SOUND_FAULT"13";
		case FAULT_MKK_SOUND_14: return MKK_SOUND_FAULT"14";
		case FAULT_MKK_SOUND_15: return MKK_SOUND_FAULT"15";
		case FAULT_MKK_SOUND_16: return MKK_SOUND_FAULT"16";
		case FAULT_MKK_SOUND_17: return MKK_SOUND_FAULT"17";
		case FAULT_MKK_SOUND_18: return MKK_SOUND_FAULT"18";
		case FAULT_MKK_SOUND_19: return MKK_SOUND_FAULT"19";
		case FAULT_MKK_SOUND_20: return MKK_SOUND_FAULT"20";
		case FAULT_MKK_SOUND_21: return MKK_SOUND_FAULT"21";
		case FAULT_MKK_SOUND_22: return MKK_SOUND_FAULT"22";
		case FAULT_MKK_SOUND_23: return MKK_SOUND_FAULT"23";
		case FAULT_MKK_SOUND_24: return MKK_SOUND_FAULT"24";

		case FAULT_MKK_START_1: return MKK_START_FAULT"1";
		case FAULT_MKK_START_2: return MKK_START_FAULT"2";

		case FAULT_MKK_DEVICE_1: return MKK_DEVICE_FAULT"1";
		case FAULT_MKK_DEVICE_2: return MKK_DEVICE_FAULT"2";

		case FAULT_PD_CON_1: return PD_CON_FAULT"1";
		case FAULT_PD_CON_2: return PD_CON_FAULT"2";

		case FAULT_UM_CON_1: return UN_CON_FAULT"1";
		case FAULT_UM_CON_2: return UN_CON_FAULT"2";
		case FAULT_UM_CON_3: return UN_CON_FAULT"3";
		case FAULT_UM_CON_4: return UN_CON_FAULT"4";
		case FAULT_UM_CON_5: return UN_CON_FAULT"5";
		case FAULT_UM_CON_6: return UN_CON_FAULT"6";



		case FAULT_UM_PROTECT_CH1_1: return UM_PROTECT_CH1" УМ-1";
		case FAULT_UM_PROTECT_CH1_2: return UM_PROTECT_CH1" УМ-2";
		case FAULT_UM_PROTECT_CH1_3: return UM_PROTECT_CH1" УМ-3";
		case FAULT_UM_PROTECT_CH1_4: return UM_PROTECT_CH1" УМ-4";
		case FAULT_UM_PROTECT_CH1_5: return UM_PROTECT_CH1" УМ-5";
		case FAULT_UM_PROTECT_CH1_6: return UM_PROTECT_CH1" УМ-6";

		case FAULT_UM_PROTECT_CH2_1: return UM_PROTECT_CH2" УМ-1";
		case FAULT_UM_PROTECT_CH2_2: return UM_PROTECT_CH2" УМ-2";
		case FAULT_UM_PROTECT_CH2_3: return UM_PROTECT_CH2" УМ-3";
		case FAULT_UM_PROTECT_CH2_4: return UM_PROTECT_CH2" УМ-4";
		case FAULT_UM_PROTECT_CH2_5: return UM_PROTECT_CH2" УМ-5";
		case FAULT_UM_PROTECT_CH2_6: return UM_PROTECT_CH2" УМ-6";

		case FAULT_UM_BATTERY_1: return UM_BATTERY" УМ-1";
		case FAULT_UM_BATTERY_2: return UM_BATTERY" УМ-2";
		case FAULT_UM_BATTERY_3: return UM_BATTERY" УМ-3";
		case FAULT_UM_BATTERY_4: return UM_BATTERY" УМ-4";
		case FAULT_UM_BATTERY_5: return UM_BATTERY" УМ-5";
		case FAULT_UM_BATTERY_6: return UM_BATTERY" УМ-6";

		case FAULT_UM_LINE_OUTAGE_1: return UM_LINE_OUTAGE" УМ-1";
		case FAULT_UM_LINE_OUTAGE_2: return UM_LINE_OUTAGE" УМ-2";
		case FAULT_UM_LINE_OUTAGE_3: return UM_LINE_OUTAGE" УМ-3";
		case FAULT_UM_LINE_OUTAGE_4: return UM_LINE_OUTAGE" УМ-4";
		case FAULT_UM_LINE_OUTAGE_5: return UM_LINE_OUTAGE" УМ-5";
		case FAULT_UM_LINE_OUTAGE_6: return UM_LINE_OUTAGE" УМ-6";

		case FAULT_UM_CH1_OUTAGE_1: return UM_CH1_OUTAGE" УМ-1";
		case FAULT_UM_CH1_OUTAGE_2: return UM_CH1_OUTAGE" УМ-2";
		case FAULT_UM_CH1_OUTAGE_3: return UM_CH1_OUTAGE" УМ-3";
		case FAULT_UM_CH1_OUTAGE_4: return UM_CH1_OUTAGE" УМ-4";
		case FAULT_UM_CH1_OUTAGE_5: return UM_CH1_OUTAGE" УМ-5";
		case FAULT_UM_CH1_OUTAGE_6: return UM_CH1_OUTAGE" УМ-6";

		case FAULT_UM_CH2_OUTAGE_1: return UM_CH2_OUTAGE" УМ-1";
		case FAULT_UM_CH2_OUTAGE_2: return UM_CH2_OUTAGE" УМ-2";
		case FAULT_UM_CH2_OUTAGE_3: return UM_CH2_OUTAGE" УМ-3";
		case FAULT_UM_CH2_OUTAGE_4: return UM_CH2_OUTAGE" УМ-4";
		case FAULT_UM_CH2_OUTAGE_5: return UM_CH2_OUTAGE" УМ-5";
		case FAULT_UM_CH2_OUTAGE_6: return UM_CH2_OUTAGE" УМ-6";


		case FAULT_UM_TEST_LINE1_1: return UM_TEST_LINE1" УМ-1";
		case FAULT_UM_TEST_LINE1_2: return UM_TEST_LINE1" УМ-2";
		case FAULT_UM_TEST_LINE1_3: return UM_TEST_LINE1" УМ-3";
		case FAULT_UM_TEST_LINE1_4: return UM_TEST_LINE1" УМ-4";
		case FAULT_UM_TEST_LINE1_5: return UM_TEST_LINE1" УМ-5";
		case FAULT_UM_TEST_LINE1_6: return UM_TEST_LINE1" УМ-6";

		case FAULT_UM_TEST_LINE2_1: return UM_TEST_LINE2" УМ-1";
		case FAULT_UM_TEST_LINE3_2: return UM_TEST_LINE2" УМ-2";
		case FAULT_UM_TEST_LINE2_3: return UM_TEST_LINE2" УМ-3";
		case FAULT_UM_TEST_LINE2_4: return UM_TEST_LINE2" УМ-4";
		case FAULT_UM_TEST_LINE2_5: return UM_TEST_LINE2" УМ-5";
		case FAULT_UM_TEST_LINE2_6: return UM_TEST_LINE2" УМ-6";

		case FAULT_PD_RING: return "ПД РАЗРЫВ КОЛЬЦА";

		case FAULT_PU_RING_CMD:	return "ПУ РАЗРЫВ КОЛЬЦА(КОММАНДЫ)";
		case FAULT_PU_RING_AUDIO: return "ПУ РАЗРЫВ КОЛЬЦА(АУДИО)";

		case FAULT_ZASH_UM1: return "ЗАЩИТА УМ1";
		case  FAULT_PER_UM1: return "ПЕРЕГРУЗКА УМ1";

		case FAULT_ZASH_UM2: return "ЗАЩИТА УМ2";
		case FAULT_PER_UM2: return "ПЕРЕГРУЗКА УМ2";

		case FAULT_FATFS_NOT_MOUNT: return "ОШИБКА ФАЙЛОВОЙ СИСТЕМЫ";
		case FAULT_MKI_NOT_RECV: return "ОШИБКА СВЯЗИ С МКИ";

		case FAULT_MKK1_NOT_RECV: return "ОШИБКА СВЯЗИ С МКК1";
		case FAULT_MKK2_NOT_RECV: return "ОШИБКА СВЯЗИ С МКК2";

		default: return "НЕИЗВСЕТНО";
	}
}


void fill_crc16(uint8_t * packet, uint16_t packet_len)
{
	uint16_t crc;

	if (packet_len < 2) return;

	crc = Crc16(packet, packet_len - 2);

	packet[packet_len - 2] = crc >> 8;
	packet[packet_len - 1] = crc & 0xFF;
}
/*
uint16_t create_piv_packet_cmd(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uint16_t len, i, crc;

	buf[0] = daddr;
	buf[1] = saddr;

	len = 1 + cmd_data_len; //sizeof(cmd) + cmd_data_len
	buf[2] = len >> 8;
	buf[3] = len & 0xFF;

	buf[4] = cmd;

	if (cmd_data) {
		for (i = 0; i < cmd_data_len; ++i) {
			buf[5 + i] = cmd_data[i];
		}
	}

	len = 5 + cmd_data_len; // sizeof(daddr) + sizeof(saddr) + sizeof(len) + sizeof(cmd)
	crc = Crc16(buf, len);

	buf[len] = crc >> 8;
	buf[len + 1] = crc & 0xFF;

	return len + 2;
}
*/
uint16_t create_piv_packet_cmd2(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len, uint16_t packet_id)
{
	uint16_t len, i, crc;

	buf[0] = daddr;
	buf[1] = saddr;

	len = 1 + cmd_data_len; //sizeof(cmd) + cmd_data_len
	buf[2] = packet_id >> 8;
	buf[3] = packet_id & 0xFF;

	buf[4] = cmd;

	if (cmd_data) {
		for (i = 0; i < cmd_data_len; ++i) {
			buf[5 + i] = cmd_data[i];
		}
	}

	len = 5 + cmd_data_len; // sizeof(daddr) + sizeof(saddr) + sizeof(len) + sizeof(cmd)
	crc = Crc16(buf, len);

	buf[len] = crc >> 8;
	buf[len + 1] = crc & 0xFF;

	return len + 2;
}


uint16_t create_piv_packet_cmd(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	return create_piv_packet_cmd2(buf, daddr, saddr, cmd, cmd_data, cmd_data_len, 1 + cmd_data_len);
}

//создать пакет ПИВ в buf,
//daddr - адрес кому
//saddr - адрес от кого
//data - данные,  data_len - их количество
//возвращает длину пакета
uint16_t create_piv_packet(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t * data, uint16_t data_len)
{
	return create_piv_packet_cmd(buf, daddr, saddr, data[0], &data[1], data_len ? data_len - 1: 0);
}


BOOL is_correct_crc16(uint8_t * packet, uint16_t packet_len)
{
	uint16_t rcrc, crc;

	rcrc = (packet[packet_len - 2] << 8) | packet[packet_len - 1] ;
	crc = Crc16(packet, packet_len - 2);
	return rcrc == crc;
}

BOOL is_correct_datalen(uint8_t * packet, uint16_t packet_len)
{
	return 6 + ((uint16_t)packet[2] << 8) + packet[3] == packet_len;
}

//Парсит пакет ПМВ
pu_packet_cmd_t * parse_piv_packet(uint8_t * packet, uint16_t packet_len)
{
	if (packet_len < 7) return NULL;
	if (!is_correct_datalen(packet, packet_len)) return NULL;
	if (!is_correct_crc16(packet, packet_len)) return NULL;

	return (pu_packet_cmd_t *)packet;
}



void filelog_fmt(const char * fmt, ... )
{
	va_list arg;
	char buf[MAX_STR_SIZE];

	strcpy_s(buf, sizeof(buf), get_date_time_str());

	//strcat_s(buf, sizeof(buf), " Режим: ");
	//strcat_s(buf, sizeof(buf), pu_mode2str(pu_mode));
	//strcat_s(buf, sizeof(buf), ", ");


	va_start(arg, fmt);

	vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, arg);

	va_end(arg);

	strcat_s(buf, sizeof(buf), CRLF);

	buf[sizeof(buf) - 1] = 0;

	filelog(buf);

}

//Линии в строку
void pu_lines_to_str(uint32_t lines, char * buf, uint16_t buf_size)
{
	int i;

	buf[0] = '\0';
	for (i = 0; i < 24; ++i) {
		if (GETBIT(lines, i)) {
			if (buf[0] != '\0') strcat_s(buf, buf_size, ",");
			strcat_s(buf, buf_size, int2str(i + 1));
		}
	}
}

//Линии в строку с буфером
char * pu_lines2str(uint32_t lines)
{
	static uint8_t __pu_lines2str_index = 0;
	static char __pu_lines2str[2][24 * 2 + 24 + 1];

	__pu_lines2str_index = !__pu_lines2str_index;

	pu_lines_to_str(lines, __pu_lines2str[__pu_lines2str_index], sizeof(__pu_lines2str[__pu_lines2str_index]));

	return __pu_lines2str[__pu_lines2str_index];
}

//какойлибо пин код введен
BOOL is_set_setting_pin_code(void)
{
	int i;

	for (i = 1; i <= 10; i++) {
		if (setting.users[i].pin_code[0] != '\0') return TRUE;
	}
	return FALSE;
}


void ulogmem(void * mem, uint16_t mem_size, BOOL is_us)
{
	uint16_t i;
	char str[100];
	uint8_t * u8mem = (uint8_t *)mem;
	uint16_t * u16mem = (uint16_t *)mem;


	if (!mem_size) return;

	str[0] = 0;

	for (i = 0; i < mem_size; i++) {
		if (is_us) {
			strcat_s(str, sizeof(str), us2hexstr(u16mem[i]));
		} else {
			strcat_s(str, sizeof(str), uc2hexstr(u8mem[i]));
		}
		strcat_s(str, sizeof(str), " ");
	}

	str[sizeof(str) - 1] = '\0';
	ulog(str);
}


BOOL is_file_name_opus(const char * filename)
{
	char buf[MAX_STR_SIZE];

	str2lower(buf, sizeof(buf), filename);

	return strstr_s(buf, strlen(buf), ".opus") != NULL;
}


BOOL is_file_name_mp3(const char * filename)
{
	char buf[MAX_STR_SIZE];

	str2lower(buf, sizeof(buf), filename);

	return strstr_s(buf, strlen(buf), ".mp3") != NULL;
}

char * get_pu_name(void)
{
	static char __get_pu_name[28 + 1];

	strcpy_s(__get_pu_name, sizeof(__get_pu_name), "РС-ПУ АДР:");
	strcat_s(__get_pu_name, sizeof(__get_pu_name), int2str(setting.uart_local_addr));

	return __get_pu_name;
}

/*
#include "mkk.h"
void get_fault_list(char * buf, uint16_t buf_size)
{
	int i;

	buf[0] = '\0';
	for (i = 0; i < MAX_FAULTS; ++i) {
		if (fault_is_set(i)) {
			strcat_s(buf, buf_size, fault2str(i));



			if ((i >= FAULT_MKK_INSIG_1) && (i <= FAULT_MKK_INSIG_24)) {
				strcat_s(buf, buf_size, " ");
				strcat_s(buf, buf_size, mkk_get_insig_state_str(i - FAULT_MKK_INSIG_1));
			}
			else if ((i >= FAULT_MKK_SOUND_1) && (i <= FAULT_MKK_SOUND_24)) {
				strcat_s(buf, buf_size, " ");
				strcat_s(buf, buf_size, mkk_get_lin_state_str(i - FAULT_MKK_SOUND_1));
			}
			else if ((i >= FAULT_MKK_START_1) && (i <= FAULT_MKK_START_2)) {
				strcat_s(buf, buf_size, " ");
				strcat_s(buf, buf_size, mkk_get_start_state_str(i - FAULT_MKK_START_1));
			}

			strcat_s(buf, buf_size, CRLF);


		}
	}

}


//из реальной частоты в частоту ПИВ
uint8_t freq2byte(uint32_t freq)
{
	if (freq < 9000) return AUDIO_FREQUENCY_8;
	else if (freq < 12000) return AUDIO_FREQUENCY_11;
	else if (freq < 17000) return AUDIO_FREQUENCY_16;
	else if (freq < 23000) return AUDIO_FREQUENCY_22;
	else if (freq < 25000) return AUDIO_FREQUENCY_24;
	else if (freq < 33000) return AUDIO_FREQUENCY_32;
	else if (freq < 45000) return AUDIO_FREQUENCY_44;
	else return AUDIO_FREQUENCY_48;
}

//из частоты ПИВ в реальную частоту
uint32_t byte2freq(uint8_t port)
{
	switch (port) {
		case AUDIO_FREQUENCY_8 : return 8000;
		case AUDIO_FREQUENCY_11 : return 11025;
		case AUDIO_FREQUENCY_16 : return 16000;
		case AUDIO_FREQUENCY_22 : return 22050;
		case AUDIO_FREQUENCY_24 : return 24000;
		case AUDIO_FREQUENCY_32 : return 32000;
		case AUDIO_FREQUENCY_44 : return 44100;
		case AUDIO_FREQUENCY_48 : return 48000;
		default:  return 8000;
	}
}


BOOL uart_9bit_snd(UART_HandleTypeDef *huart, uint8_t * data, uint16_t data_len, uint16_t * buf, uint16_t buf_len, BOOL it)
{
	uint16_t crc, i;

	if (data_len + 2 > buf_len) return FALSE;
	if (huart->gState == HAL_UART_STATE_BUSY_TX) return FALSE;

	crc = 0;
	for (i = 0; i < data_len; ++i) {
		buf[i] = data[i];
		crc += data[i];
	}

	buf[0] |= 0x100;


	buf[data_len] =  crc >> 8;
	buf[data_len + 1] =  crc & 0xFF;

	if (it) HAL_UART_Transmit_IT(huart, (uint8_t *)buf, data_len + 2);
	else HAL_UART_Transmit(huart, (uint8_t *)buf, data_len + 2, 100);

	return TRUE;
}


BOOL chk_str_crc(char * data, uint16_t data_len)
{
	char * ptr;
	int ival, ival2;

	ptr = strstr_s(data, data_len, " CRC");
	if (!ptr) return TRUE;
	ival = str2int(ptr + strlen(" CRC"));
	*ptr = '\0';
	ival2 = CrcSum((unsigned char *)data, ptr - data);

	return (ival == ival2);
}


//void clear_clrf(char * data, uint16_t data_len)
//{
//	char * ptr;

//	data[data_len] = '\0';
//	ptr = strstr_s((char *)data, data_len, "\r");
//	if (ptr) *ptr = '\0';
//	ptr = strstr_s((char *)data, data_len, "\n");
//	if (ptr) *ptr = '\0';
//}

*/
