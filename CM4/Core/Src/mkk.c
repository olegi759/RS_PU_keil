/*
 * mkk.c
 *
 *  Created on: 19 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "mkk.h"
#include "mkk_data.h"
#include "setting.h"
#include "stm_util.h"
#include "net_util.h"
#include <string.h>
#include "mki.h"
#include "str_util.h"
#include "math_util.h"
#include "cm4core.h"
#include "fault.h"
#include "fault_data.h"
#include "common.h"
#include "console.h"


////контроль исправностей линий звука
//uint8_t mkk_rcv_sound[24] = {0};
//контроль исправностей линий пожарной сигнализации
//uint8_t mkk_rcv_insig[24] = {0};
//контроль исправностей линий старт
//uint8_t mkk_rcv_start[2] = {0};


static uint16_t mkk_tx_buf[MAX_UART2_PACKET_SIZE];


//Послать комманду на МКК по протоколу
void mkk_send(uart2_mux_t mux, uint8_t cmd, uint8_t * data, uint8_t data_len)
{
	uint16_t i, crc, * tx;


	//tx = uart2_get_tx_buf();
	tx = mkk_tx_buf;


	tx[0] = 0x100 | cmd;
	tx[1] = data_len;

	crc = cmd + data_len;
	if (data) {
		for (i = 0; i < data_len; ++i) {
			tx[2 + i]  = data[i];
			crc += data[i];
		}
	}

	tx[data_len + 2] = crc >> 8;
	tx[data_len + 3] = crc & 0xFF;

	uart2_transmit(mux, tx, data_len + 4);
}


const uint16_t mkk_cmd_reboot[4] = { 0x100 | MKK_CMD_REBOOT, 0x00,  0x00, MKK_CMD_REBOOT };

//Послать комманду перезагрузись
void mkk_send_reboot(uart2_mux_t mux)
{
	uart2_transmit(mux, (uint16_t *)mkk_cmd_reboot, 4);
}

void mkk_send_version(uart2_mux_t mux)
{
	mkk_send(mux, MKK_CMD_VERSION, NULL, 0);
}
//void mkk_send_cfg(uart2_mux_t mux, uint8_t value)
//{
//	mkk_send(mux, MKK_CMD_CFG, &value, 1);
//}

//Послать комманду включи контроль линий пожарной сигнализации
void mkk_send_insig_on(uart2_mux_t mux, uint16_t value)
{
	mkk_send(mux, MKK_CMD_INSIG_ON, (uint8_t *)&value, 2);
}

//Послать комманду включи контроль линий пожарной сигнализации из настроек
void mkk_send_insig_on_from_setting(uart2_mux_t mux)
{
	uint16_t val;

	if (mux == UART2_MUX_MKKL_1) {
		val = setting.mkk_insig_lines & 0xFFF;
		if (GETBIT(setting.mkk_start_lines, 0)) SETBIT(val, 12);
		mkk_send_insig_on(mux, HTONS(val));
	}
	else if (mux == UART2_MUX_MKKL_2) {
		val = (setting.mkk_insig_lines >> 12) & 0xFFF;
		if (GETBIT(setting.mkk_start_lines, 1)) SETBIT(val, 12);
		mkk_send_insig_on(mux, HTONS(val));
	}
}


//Послать комманду включи контроль линий звука
void mkk_send_lin_on(uart2_mux_t mux, uint16_t value)
{
	mkk_send(mux, MKK_CMD_LIN_ON, (uint8_t *)&value, 2);
}

//Послать комманду включи контроль линий звука из настроек
void mkk_send_lin_on_from_setting(uart2_mux_t mux)
{
	uint16_t val;


	if (mux == UART2_MUX_MKKL_1) {
		val = setting.mkk_sound_lines & 0xFFF;
		mkk_send_lin_on(mux, HTONS(val));
	} else if (mux == UART2_MUX_MKKL_2) {
		val = (setting.mkk_sound_lines >> 12) & 0xFFF;
		mkk_send_lin_on(mux, HTONS(val));
	}
}


//const uint16_t mkk_cmd_start_on[4] = { 0x100 | MKK_CMD_START_ON, 0x00,  0x00, MKK_CMD_START_ON };

//void mkk_send_start_on(uart2_mux_t mux)
//{
//	uart2_transmit(mux, (uint16_t *)mkk_cmd_start_on, 4);
//}

const uint16_t mkk_cmd_insig_lin[4] = { 0x100 | MKK_CMD_INSIG_LIN, 0x00,  0x00, MKK_CMD_INSIG_LIN };

//Послать запрос контроля исправности входов сигнализации и контроля исправности линий оповещения и входа запуск.
void mkk_send_insig_lin(uart2_mux_t mux)
{
	uart2_transmit(mux, (uint16_t *)mkk_cmd_insig_lin, 4);
}

//void mkk_send_start(uart2_mux_t mux)
//{
//	mkk_send(mux, MKK_CMD_START, NULL, 0);
//}

//Послать команду управления коммутацией оповещения
void mkk_send_lin(uart2_mux_t mux, uint16_t um1, uint16_t um2)
{
	uint8_t data[4] = {(um1 >> 8) & 0xFF, um1 & 0xFF, (um2 >> 8) & 0xFF, um2 & 0xFF};

	mkk_send(mux, MKK_CMD_LIN, data, 4);
}

///Послать команду управления коммутацией УМ1 и УМ2
void mkk_send_lin_from_setting(uart2_mux_t mux)
{

	uint16_t val1, val2;


	if (mux == UART2_MUX_MKKL_1) {
		val1 = setting.mkk_um1 & 0xFFF;
		val2 = setting.mkk_um2 & 0xFFF;
	} else if (mux == UART2_MUX_MKKL_2) {
		val1 = (setting.mkk_um1 >> 12) & 0xFFF;
		val2 = (setting.mkk_um2 > 12) & 0xFFF;
	} else return;

	mkk_send_lin(mux, HTONS(val1), HTONS(val2));
}


void mkk_enable_sound_lines(uint32_t lines)
{
	uint16_t ls;
	uint16_t val1, val2;

	//МКК1
	ls = lines & 0xFFF;
	val1 = ls & setting.mkk_um1;
	val2 = ls & setting.mkk_um2;
	mkk_send_lin(UART2_MUX_MKKL_1, HTONS(val1), HTONS(val2));


	if (setting.lines_count > 12) {

		HAL_Delay(50);

		//МКК2
		ls = (lines >> 12) & 0xFFF;
		val1 = ls & (setting.mkk_um1 >> 12);
		val2 = ls & (setting.mkk_um2 >> 12);
		mkk_send_lin(UART2_MUX_MKKL_1, HTONS(val1), HTONS(val2));
	}
}



//Послать выполнить замер опорных значений параметров линий оповещения (value - %)
void mkk_send_lin_met(uart2_mux_t mux, uint8_t value)
{
	mkk_send(mux, MKK_CMD_LIN_MET, &value, 1);
}

//Послать команду управления реле
void mkk_send_relay(uart2_mux_t mux, uint8_t state)
{
	mkk_send(mux, MKK_CMD_RELAY, &state, 1);
}


static uint8_t mkk_relay_values[4] = {0};

void mkk_send_relay_mux(uart2_mux_t mux, uint8_t relay_number, BOOL on)
{

	if (on) SETBIT(mkk_relay_values[mux], relay_number - 1);
	else CLRBIT(mkk_relay_values[mux], relay_number - 1);

	mkk_send_relay(mux, mkk_relay_values[mux]);
}

BOOL mkk_get_relay_mux(uart2_mux_t mux, uint8_t relay_number)
{
	return GETBIT(mkk_relay_values[mux], relay_number - 1) ? TRUE : FALSE;
}


/*
//сработавшие входов линий пожарной сигнализации
uint32_t mkk_get_fire_lines(void)
{

	//int i;
	uint32_t res;

	//if (fire_lines_imit) return fire_lines_imit;

	res = 0;
	//for (i = 0; i < 24; i++) {
	//	if (GETBIT(pu_option_otkluchenie_lines, i)) continue;
	//	if (mkk_rcv_insig[i] == MKK_INSIG_STATE_NORM_YES_SIGNAL) SETBIT(res, i);
	//}
	return res;
}


//кол-во сработавших входов линий пожарной сигнализации
uint8_t mkk_fire_lines_cnt(void)
{
	uint32_t afire_lines;

	afire_lines = mkk_get_fire_lines();

	return bitis1cnt(&afire_lines, 24);
}

//ошибочные входы линий пожарной сигнализации
uint32_t mkk_get_fault_insig_lines(void)
{
	int i;
	uint32_t res;


	res = 0;
	for (i = 0; i < 24; i++) {
		if (GETBIT(setting.mkk_insig_lines, i)) { //проверяемые линии
			if ((mkk_insig_lin_replay.insig[i] == MKK_INSIG_STATE_BREAK) || (mkk_insig_lin_replay.insig[i] == MKK_INSIG_STATE_UNCERTAINTY) || (mkk_insig_lin_replay.insig[i] == MKK_INSIG_STATE_SHORT_CIRCUIT)) SETBIT(res, i);
		}
	}
	return res;
}

*/

const char * mkk_get_lin_state_str(uint8_t line_number)
{
	return  mkk_lin_state2str(mkk_insig_lin_replay.sound[line_number]);
}


const char * mkk_get_insig_state_str(uint8_t line_number)
{
	return  mkk_insig_state2str(mkk_insig_lin_replay.insig[line_number]);
}



const char * mkk_get_start_state_str(uint8_t line_number)
{
	return  mkk_start_state2str(mkk_insig_lin_replay.start[line_number]);
}


BOOL is_fault_insig_line(uint8_t value)
{
	 return ((value == MKK_INSIG_STATE_BREAK) || (value == MKK_INSIG_STATE_UNCERTAINTY) || (value == MKK_INSIG_STATE_SHORT_CIRCUIT));
}

BOOL is_fault_sound_line(uint8_t value)
{
	return ((value == MKK_LIN_STATE_BREAK) || (value == MKK_LIN_STATE_UP) || (value == MKK_LIN_STATE_DOWN) || (value == MKK_LIN_STATE_SHORT_CIRCUIT));
}

BOOL is_fault_start_line(uint8_t value)
{
	return  ((value ==  MKK_START_STATE_BREAK) || (value ==  MKK_START_STATE_UNCERTAINTY) || (value ==  MKK_START_STATE_SHORT_CIRCUIT));

}


void mkk_change(void)
{
	int i;


	for (i = 0; i < 24; i++) {
		if ( (GETBIT(setting.mkk_insig_lines, i)) && (is_fault_insig_line(mkk_insig_lin_replay.insig[i])) ) { //проверяемые линии и ошибка
			fault_set(FAULT_MKK_INSIG_1 + i);
		} else {
			fault_unset(FAULT_MKK_INSIG_1 +  i);
		}
	}

    //Когда включается усилитель и его выход коммутируется релюшками на линию оповещения то соответственно отключается и тестирование
    //В момент вещания МКК сообщает, что на линии обрыв, а на самом деле просто канал тестирования отключается и подключается выход усилителя.
	for (i = 0; i < 24; i++) {
		if (GETBIT(pu_enable_lines, i)) {
			//fault_unset(FAULT_MKK_SOUND_1 + i);
			continue; //включенные линии не проверяем
		}
		if ( (GETBIT(setting.mkk_sound_lines, i)) && (is_fault_sound_line(mkk_insig_lin_replay.sound[i])) ){ //проверяемые линии
			fault_set(FAULT_MKK_SOUND_1 + i);
		} else {
			fault_unset(FAULT_MKK_SOUND_1 + i);
		}
	}


	for (i = 0; i < 2; i++) {

		if (mkk_insig_lin_replay.start[i] == MKK_START_STATE_NORM_YES_SIGNAL) { //Норма сигнал есть, сопротивление 2,35 кОм (2…3 кОм)
			fault_set(FAULT_MKK_DEVICE_1 + i);
			fault_unset(FAULT_MKK_START_1 + i);
		}
		else if (mkk_insig_lin_replay.start[i] == MKK_START_STATE_NORM_NO_SIGNAL) { //Норма сигнала нет, сопротивление 4,7 кОм (4…5,4 кОм)
			fault_unset(FAULT_MKK_DEVICE_1 + i);
			fault_unset(FAULT_MKK_START_1 + i);
		}

		else if (GETBIT(setting.mkk_start_lines, i)) {
			fault_unset(FAULT_MKK_DEVICE_1 + i);
			if ( is_fault_start_line(mkk_insig_lin_replay.start[i]) )  {
				fault_set(FAULT_MKK_START_1 + i);
			} else {
				fault_unset(FAULT_MKK_START_1 + i);
			}
		}
		else {
			fault_unset(FAULT_MKK_START_1 + i);
			fault_unset(FAULT_MKK_DEVICE_1 + i);
		}
	}


	cm4core_send_to_CM7_cmd(CORE_CMD_CHANGE_MKK_INSIG_LIN, &mkk_insig_lin_replay, sizeof(mkk_insig_lin_replay));

}




void mkk_rcv(uart2_mux_t mux, uint16_t * packet, uint16_t packet_len)
{
	uint16_t rcrc, scrc, i;
	uint8_t val;
	BOOL change;


	//ulog_fmt("mkk: rcv %d  %d"CRLF, mux, data_len);

	if (!(packet[0] & 0x0100)) return;
	packet[0] = packet[0] & 0xFF;

	 if (2 + packet[1] + 2 != packet_len) return;

	rcrc = (packet[packet_len - 2] << 8) | packet[packet_len - 1];
	scrc = 0;
	for (i = 0; i < packet_len - 2; ++i) {
		scrc += packet[i];
	}

	if (rcrc != scrc) {
		return;
	}


	switch (packet[0]) {
		case MKK_CMD_INSIG_LIN:

			if (packet[1] != 25) return;


			change = FALSE;
			for (i = 0; i < 25; i++) {
				val = packet[2 + i] & 0xFF;

				if (mux == UART2_MUX_MKKL_1) {
					if (i < 12) {
						if (mkk_insig_lin_replay.insig[i] != val) {
							change = TRUE;
							mkk_insig_lin_replay.insig[i] = val;
						}
					}
					else if (i < 24) {
						if (mkk_insig_lin_replay.sound[i - 12] != val) {
							change = TRUE;
							mkk_insig_lin_replay.sound[i - 12] = val;
						}
					}
					else {
						if (mkk_insig_lin_replay.start[0] != val) {
							change = TRUE;
							mkk_insig_lin_replay.start[0] = val;
						}
					}
				}
				else if (mux == UART2_MUX_MKKL_2) {
					if (i < 12) {
						if (mkk_insig_lin_replay.insig[12 + i] != val) {
							change = TRUE;
							mkk_insig_lin_replay.insig[12 + i] = val;
						}
					}
					else if (i < 24) {
						if (mkk_insig_lin_replay.sound[i] != val) {
							change = TRUE;
							mkk_insig_lin_replay.sound[i] = val;
						}
					}
					else {
						if (mkk_insig_lin_replay.start[1] != val) {
							change = TRUE;
							mkk_insig_lin_replay.start[1] = val;
						}
					}
				}

			}

			if (change) {
				mkk_change(); //сменились данные на МКК


				//ulog(CONM"mkk_sound: ");
				//ulogmem(mkk_insig_lin_replay.sound, 24, FALSE);
				//ulog(CRLF);

				//ulog(CONM"mkk_insig: ");
				//ulogmem(mkk_insig_lin_replay.insig, 24, FALSE);
				//ulog(CRLF);

				//ulog(CONM"mkk_start: ");
				//ulogmem(mkk_insig_lin_replay.start, 2, FALSE);
				//ulog(CRLF);
			}


		break;
		case MKK_CMD_VERSION:
			if (mux == UART2_MUX_MKKL_1) {
				memset(mkk1_version, 0, sizeof(mkk1_version));
				rcrc = sizeof(mkk1_version) - 1;
				if (rcrc > packet[1]) rcrc = packet[1];
				for (i = 0; i < rcrc; ++i) {
					mkk1_version[i] = packet[2 + i] & 0xFF;
				}
				fault_unset(FAULT_MKK1_NOT_RECV);
				ulog_fmt(CONM"MKK1 Version: %s"CRLF, mkk1_version);
			}
			else if (mux == UART2_MUX_MKKL_2) {
				memset(mkk2_version, 0, sizeof(mkk2_version));
				rcrc = sizeof(mkk2_version) - 1;
				if (rcrc > packet[1]) rcrc = packet[1];
				for (i = 0; i < rcrc; ++i) {
					mkk2_version[i] = packet[2 + i] & 0xFF;
				}
				fault_unset(FAULT_MKK2_NOT_RECV);
				ulog_fmt(CONM"MKK2 Version: %s"CRLF, mkk2_version);
			}
		break;
	}




}
