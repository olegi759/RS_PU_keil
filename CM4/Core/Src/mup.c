/*
 * mup.c
 *
 *  Created on: 12 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "mup.h"
#include "uart2.h"
#include "math_util.h"
#include "stm_util.h"
#include "str_util.h"
#include "fault.h"
#include "fault_data.h"
//#include "screens.h"
#include "mki.h"
#include "cm4core.h"
#include "mup_data.h"

//Скорость 115200.
//UART 9 bit. девятый бит устанавливается только в первом байте.
//запрос: 0xEE/0xAA/0/80/0/0/0/0/0/0/ два байта суммы всех байт
//ответ: 0xAA/0xEE/0/80/аварии/вых реле/0/0/0/0/ два байта суммы всех байт

//const uint16_t mup_query[12] = {0x1EE, 0xAA, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x18};
const uint16_t mup_query[12] = 	{0x1EE, 0x0AA, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE8};

uint8_t mup_errrors = 0xFF;
static uint8_t mup_relays = 0;

//Послать запрос к МУП
void mup_send_query(void)
{
	uart2_transmit(UART2_MUX_MUP, (uint16_t *)mup_query, 12);
}


//Поменялись ошибки
void mup_change_errors(uint8_t new_errors)
{

	ulog_fmt(CONM"mup: change_errors %d(%s)"CRLF, new_errors, uch2bitstr(new_errors));


	if (GETBIT(new_errors, MUP_BIT_ERROR_220V)) fault_set(FAULT_MUP_220V);
	else fault_unset(FAULT_MUP_220V);

	//if (GETBIT(new_errors, MUP_BIT_ERROR_PS)) fault_set(FAULT_MUP_ERROR_PS);
	//else fault_unset(FAULT_MUP_ERROR_PS);

	if (GETBIT(new_errors, MUP_BIT_ERROR_ACC)) fault_set(FAULT_MUP_ERROR_ACC);
	else fault_unset(FAULT_MUP_ERROR_ACC);

	if (GETBIT(new_errors, MUP_BIT_DISCHARGE_ACC)) fault_set(FAULT_MUP_DISCHARGE_ACC);
	else fault_unset(FAULT_MUP_DISCHARGE_ACC);

	//if (GETBIT(new_errors, MUP_BIT_CAPACITY_ACC)) fault_set(FAULT_MUP_CAPACITY_ACC);
	//else fault_unset(FAULT_MUP_CAPACITY_ACC);

	//if (GETBIT(new_errors, MUP_BIT_METERAGE)) fault_set(FAULT_MUP_METERAGE);
	//else fault_unset(FAULT_MUP_METERAGE);

	if (GETBIT(new_errors, MUP_BIT_TD_BP)) fault_set(FAULT_MUP_TD_BP);
	else fault_unset(FAULT_MUP_TD_BP);

	//if (GETBIT(new_errors, MUP_BIT_TD_ACC)) fault_set(FAULT_MUP_TD_ACC);
	//else fault_unset(FAULT_MUP_TD_ACC);



	mki_set_small_led(MKI_LED_BATTERY, (GETBIT(new_errors, MUP_BIT_ERROR_ACC) || GETBIT(new_errors, MUP_BIT_DISCHARGE_ACC)) ?  LED_COLOR_YELLOW : LED_COLOR_GREEN);
	mki_set_small_led(MKI_LED_NET, GETBIT(new_errors, MUP_BIT_ERROR_220V) ? LED_COLOR_YELLOW : LED_COLOR_GREEN);
	mki_send_small_leds();

	//CORE if (screens_now() == 0) screens_update();

	//cm4core_send_to_CM7_cmd(CORE_CMD_MUP_CHANGE_ERROR, &new_errors, 1);

}

//Поменялись реле
void mup_change_relays(uint8_t new_relays)
{
	//ulog_fmt("mup: change_relays %s"CRLF, uc2hexstr(new_relays));

}

//Прием данных от МУП
void mup_rcv(uint16_t * packet, uint16_t packet_len)
{
	uint16_t * ptr, crc_sum, crc_rcv;
	uint16_t i;


	//ulog_fmt("MUP  rcv:%d"CRLF, data_len);


	if (packet_len < 12) return;


	ptr = &packet[packet_len - 12];

	if (ptr[0] != 0x01AA) return;
	if (ptr[1] != 0x00EE) return;

	crc_sum = 0;
	for (i = 0; i < 10; i++) {
		crc_sum += (ptr[i] & 0xFF);
	}

	crc_rcv = ((ptr[10] << 8) | ptr[11]);

	//ulog_fmt("MUP  crc_sum:%d crc_rcv:%d"CRLF, crc_sum, crc_rcv);

	if (crc_sum != crc_rcv) return;

	//ulog_fmt("mup: rcv errors %d(%s)"CRLF, ptr[4], uch2bitstr(ptr[4]));


	if (mup_errrors != ptr[4]) {
		mup_change_errors(ptr[4]);
		mup_errrors = ptr[4]; //аварии
	}

	if (mup_relays != ptr[5]) {
		mup_change_relays(ptr[5]);
		mup_relays = ptr[5]; //выходы реле
	}

}
