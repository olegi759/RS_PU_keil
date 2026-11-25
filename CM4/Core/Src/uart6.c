/*
 * uart6.c
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */



#include "uart6.h"
#include <string.h>
#include <stdio.h>
//#include "mki.h"
#include "math_util.h"
#include "stm_util.h"
#include "dynbuf_util.h"
#include "str_util.h"
//#include "console.h"
#include "um.h"
#include "cm4core.h"
#include "setting.h"
#include "fault.h"
#include "fault_data.h"
#include "pd_data.h"
#include "common.h"
#include "console.h"

//#define DEBUG_UART6 		1
//#define DEBUG_UART6_CMD		UM_CMD_TEST_LINE


static dynamic_buf_t uart6_dyn_buf = DYNAMIC_BUF_STATIC_INIT(20 * 1024) ;

static uint8_t uart6_rx_buf[MAX_UART6_PACKET_RX_SIZE];
static uint8_t uart6_tx_buf[MAX_UART6_PACKET_TX_SIZE];

static uint8_t uart6_tick = 0;
static uint32_t uart6_send_tick = 0;


void uart6_set_mux(uart6_mux_t mux)
{
	PIN_CLR(UART_SAD_A0);
	PIN_CLR(UART_SAD_A1);
	PIN_CLR(UART_SAD_A2);
	switch (mux) {
		case UART6_MUX_EXT_AMP:
			break;
		case UART6_MUX_P1C:
			PIN_SET(UART_SAD_A0);
			break;
		case UART6_MUX_P2C:
			PIN_SET(UART_SAD_A1);
			break;
		case UART6_MUX_TMP:
			PIN_SET(UART_SAD_A0);
			PIN_SET(UART_SAD_A1);
			break;
	}

}

uart6_mux_t uart6_get_mux(void)
{
	uint8_t a0, a1;

	a0 = PIN_ISSET(UART_SAD_A0);
	a1 = PIN_ISSET(UART_SAD_A1);
	if (a0 & a1) return UART6_MUX_TMP;
	else if (a1) return UART6_MUX_P2C;
	else if (a0) return UART6_MUX_P1C;
	else return UART6_MUX_EXT_AMP;
}

void HAL_UART6_TxCpltCallback(void)
{
	PIN_CLR(UART_SAD_EN);

	HAL_UARTEx_ReceiveToIdle_IT(&huart6, uart6_rx_buf, sizeof(uart6_rx_buf));
}

void HAL_UART6_RxEventCallback(uint16_t Size)
{
	if (Size < 7) return;
	if (!is_correct_datalen(uart6_rx_buf, Size)) return;

	uart6_rx_buf[Size] = uart6_get_mux();

	uart6_dyn_buf.busy = TRUE;
	dynamic_buf_push(&uart6_dyn_buf, uart6_rx_buf, Size + 1);
	uart6_dyn_buf.busy = FALSE;
}



void uart6_transmit(uart6_mux_t mux, uint8_t * packet, uint16_t packet_len)
{
	uint32_t diff;



	if (!packet || !packet_len) return;


	diff = HAL_DiffTick(uart6_send_tick);
	if (diff < 50) {
		HAL_Delay(50 - diff);
	}
	HAL_UART_AbortReceive_IT(&huart6);
	//HAL_UART_AbortTransmit_IT(&huart6);


	uart6_set_mux(mux);

	memcpy(uart6_tx_buf, packet, packet_len);

	PIN_SET(UART_SAD_EN); //DE(tx) - active high, RE(rx) - active low
	HAL_UART_Transmit(&huart6, uart6_tx_buf, packet_len, HAL_MAX_DELAY);
	//HAL_UART_Transmit_IT(&huart6, uart6_tx_buf, packet_len);
	PIN_CLR(UART_SAD_EN);

	uart6_send_tick = HAL_GetTick();
	HAL_UARTEx_ReceiveToIdle_IT(&huart6, uart6_rx_buf, sizeof(uart6_rx_buf));



#ifdef DEBUG_UART6

#ifdef DEBUG_UART6_CMD
	if (packet[4] == DEBUG_UART6_CMD) {
#endif

		ulog_fmt(CONM"t6(%d %d) ", mux, packet_len);
		ulogmem(packet, packet_len, FALSE);
		ulog(CRLF);

#ifdef DEBUG_UART6_CMD
	}
#endif

#endif

}



void uart6_transmit_piv(uart6_mux_t mux, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uint16_t len;
	uint8_t buf[MAX_PACKET_SIZE];

	len = create_piv_packet_cmd(buf, daddr, saddr, cmd, cmd_data, cmd_data_len);
	uart6_transmit(mux, buf, len);
}


//время когда пришли данные с ПД
static uint32_t mux_pd_rcv_tick[4][3]= {0};
static uint8_t pd_send_addr = 1;
static uart6_mux_t mux_pd_snd_query = UART6_MUX_P1C;


void uart6_rcv(uint8_t mux, uint8_t * packet, uint16_t packet_len)
{
	//uint16_t scrc, rcrc;


#ifdef DEBUG_UART6

#ifdef DEBUG_UART6_CMD
	if (packet[4] == DEBUG_UART6_CMD) {
#endif

		ulog_fmt(CONM"r6(%d %d) " , mux,  packet_len);
		ulogmem(packet, packet_len, FALSE);
		ulog(CRLF);

#ifdef DEBUG_UART6_CMD
	}
#endif
#endif
	//scrc = Crc16(data, data_len - 2);
	//rcrc = (data[data_len - 2] << 8) | data[data_len - 1] ;
	//if (scrc != rcrc) return;

	//if (packet_len < 7) return;
	if (!is_correct_crc16(packet, packet_len)) return;

	switch (mux) {
		case UART6_MUX_P1C:
			//pd_rcv(UART6_MUX_P1C, packet, packet_len);
			cm4core_pd_rcv(packet, packet_len);
			mux_pd_rcv_tick[UART6_MUX_P1C][packet[1]] = HAL_GetTick();
		break;
		case UART6_MUX_EXT_AMP:
			um_rcv(packet, packet_len);
		break;
		case UART6_MUX_P2C:
			//pd_rcv(UART6_MUX_P2C, packet, packet_len);
			cm4core_pd_rcv(packet, packet_len);
			mux_pd_rcv_tick[UART6_MUX_P2C][packet[1]] = HAL_GetTick();
		break;
		case UART6_MUX_TMP:
		break;
		default: break;
	}


}

void uart6_transmit_to_pd_nomux(uint8_t * packet, uint16_t packet_len)
{
	uint32_t diff1, diff2;
	uart6_mux_t mux;


	diff1 = HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P1C][packet[0]]);
	diff2 = HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P2C][packet[0]]);


	mux = (diff2 < diff1 ? UART6_MUX_P2C : UART6_MUX_P1C);
	uart6_transmit(mux, packet, packet_len);
}

void uart6_task_data(void)
{
	uint8_t buf[MAX_UART6_PACKET_TX_SIZE];
	size_t len;

	if (dynamic_buf_pop(&uart6_dyn_buf, buf, sizeof(buf), &len)) {
		uart6_rcv(buf[len - 1], buf, len - 1);
	}
}


void uart6_task_rx(void)
{
	if (DYNAMIC_BUF_IS_EMPTY(uart6_dyn_buf)) return;
	if (DYNAMIC_BUF_GET_BUSY(uart6_dyn_buf)) return;
	uart6_task_data();
}


void pd_asnd_query(void)
{


	switch (setting.pd_count) {
		case 0: return;
		case 1: //ПД1
			pd_send_addr = 1;
			mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
		break;
		case 2: //ПД2
			pd_send_addr = 2;
			mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
		break;
		case 3: //ПД1 и ПД2
			if (pd_send_addr == 2) {
				pd_send_addr = 1;
				mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
			}
			else {
				pd_send_addr = 2;
				//mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
			}

		break;
	}


	BOOL t1, t2;

	t1 = (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P1C][pd_send_addr]) > 80000);
	t2 = (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P2C][pd_send_addr]) > 80000);


	if (t1 && t2) {
		fault_set(FAULT_PD_CON_1 + (pd_send_addr - 1));
		fault_set(FAULT_PD_RING);
	}
	else if (t1 && !t2) {
		fault_set(FAULT_PD_RING);
	}
	else if (!t1 && t2) {
		fault_set(FAULT_PD_RING);
	}
	else {
		fault_unset(FAULT_PD_RING);
	}

	uart6_transmit_piv(mux_pd_snd_query, pd_send_addr, setting.uart_local_addr, PD_CMD_QUERY, NULL, 0);
}

//раз в 50 мс
void uart6_task_tx(void)
{


	if (uart6_tick < 80) uart6_tick++;
	else {
		uart6_tick = 0;
		return;
	}

	if (uart6_tick == 80) { //4 сек
		um_snd_query();
	}
	else if ((uart6_tick & 0x07) == 0x07) { //400 млс
		pd_asnd_query();
	}


}
