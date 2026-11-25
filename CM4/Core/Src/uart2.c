/*
 * uart2.c
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "uart2.h"
#include "stm32h7xx_hal_uart.h"
#include <string.h>
#include <stdio.h>
#include "mki.h"
#include "math_util.h"
#include "stm_util.h"
#include "dynbuf_util.h"
#include "str_util.h"
#include "console.h"
#include "common.h"
#include "mup.h"
#include "mkk.h"
#include "setting.h"
#include "str_util.h"
#include "mkk_data.h"



//#define DEBUG_UART2	1
//#define DEBUG_UART2_CMD_ONLY		  MKI_CMD_SMALL_LEDS

static dynamic_buf_t uart2_dyn_buf = DYNAMIC_BUF_STATIC_INIT(20 * 1024) ;

static uint16_t uart2_rx_buf[MAX_UART2_PACKET_SIZE];
static uint16_t uart2_tx_buf[MAX_UART2_PACKET_SIZE];
//static uint8_t uart2_tx_buf_index = 0;

static uint32_t uart2_tick = 10;

static uint32_t uart2_send_tick = 0;


/*
uint16_t * uart2_get_tx_buf(void)
{
	uart2_tx_buf_index = !uart2_tx_buf_index;
	return uart2_tx_buf[uart2_tx_buf_index];
}
*/

void uart2_set_mux(uart2_mux_t mux)
{
	PIN_CLR(UART_S_A0);
	PIN_CLR(UART_S_A1);
	PIN_CLR(UART_S_A2);
	switch (mux) {
		case UART2_MUX_EXT1:
			break;
		case UART2_MUX_EXT2:
			PIN_SET(UART_S_A0);
			break;
		case UART2_MUX_EXT3:
			PIN_SET(UART_S_A1);
			break;
		case UART2_MUX_EXT4:
			PIN_SET(UART_S_A0);
			PIN_SET(UART_S_A1);
			break;
	}
}

uart2_mux_t uart2_get_mux(void)
{
	uint8_t a0, a1;

	a0 = PIN_ISSET(UART_S_A0);
	a1 = PIN_ISSET(UART_S_A1);
	if (a0 & a1) return UART2_MUX_EXT4;
	else if (a1) return UART2_MUX_EXT3;
	else if (a0) return UART2_MUX_EXT2;
	else return UART2_MUX_EXT1;
}


void HAL_UART2_RxEventCallback(uint16_t Size)
{

	uart2_rx_buf[Size] = uart2_get_mux();

	uart2_dyn_buf.busy = TRUE;
	dynamic_buf_push(&uart2_dyn_buf, uart2_rx_buf, Size * 2 + 2);
	uart2_dyn_buf.busy = FALSE;

}





void uart2_transmit(uart2_mux_t mux, uint16_t * packet, uint16_t packet_len)
{
	uint32_t diff;


	if (!packet || !packet_len) return;

	diff = HAL_DiffTick(uart2_send_tick);
	if (diff < 50) {
		HAL_Delay(50 - diff);
	}
	HAL_UART_AbortReceive_IT(&huart2);
	HAL_UART_AbortTransmit_IT(&huart2);

	uart2_set_mux(mux);


	memcpy(uart2_tx_buf, packet, packet_len * 2);
	//HAL_UART_Transmit(&huart2, (uint8_t *)uart2_tx_buf, packet_len, HAL_MAX_DELAY);
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)uart2_tx_buf, packet_len);

	uart2_send_tick = HAL_GetTick();

	HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t *)uart2_rx_buf, MAX_UART2_PACKET_SIZE);


#ifdef DEBUG_UART2


#ifdef DEBUG_UART2_CMD_ONLY
	if (packet[0] == (0x100 | DEBUG_UART2_CMD_ONLY)) {
#endif
		ulog_fmt(CONM"t2(%d %d) ", mux, packet_len);
		ulogmem(packet, packet_len, TRUE);
		ulog(CRLF);


#ifdef DEBUG_UART2_CMD_ONLY
	}
#endif

#endif
}




void uart2_rcv(uint16_t mux, uint16_t * packet, uint16_t packet_len)
{
#ifdef DEBUG_UART2

#ifdef DEBUG_UART2_CMD_ONLY
	if (packet[0] == (0x100 | DEBUG_UART2_CMD_ONLY)) {
#endif

		ulog_fmt(CONM"r2(%d %d) ", mux, packet_len);
		ulogmem(packet, packet_len, TRUE);
		ulog(CRLF);

#ifdef DEBUG_UART2_CMD_ONLY
	}
#endif

#endif

	switch (mux) {
		case UART2_MUX_MKI:
			mki_rcv(packet, packet_len);
		break;
		case UART2_MUX_MKKL_1:
			mkk_rcv(UART2_MUX_MKKL_1, packet, packet_len);
		break;
		case UART2_MUX_MKKL_2:
			mkk_rcv(UART2_MUX_MKKL_2, packet, packet_len);
		break;
		case UART2_MUX_MUP:
			 mup_rcv(packet, packet_len);
		break;
		default: break;
	}
}

void uart2_task_data(void)
{
	uint16_t buf[MAX_UART2_PACKET_SIZE];
	size_t byte_len, len;

	if (dynamic_buf_pop(&uart2_dyn_buf, buf, sizeof(buf), &byte_len)) {
		len  = byte_len / 2;
		uart2_rcv(buf[len - 1], buf, len - 1);
	}
}


void uart2_task_rx(void)
{
	if (DYNAMIC_BUF_IS_EMPTY(uart2_dyn_buf)) return;
	if (DYNAMIC_BUF_GET_BUSY(uart2_dyn_buf)) return;
	uart2_task_data();
}

/*
static uint32_t uart2_mup_tick = 30;
static uint32_t uart2_mkk_tick = 60;
static BOOL uart2_mkk_send2 = FALSE;

//раз в 50 мс
void uart2_task_tx(void)
{

	if (uart2_tick < 20) {
		uart2_tick++;
		if (mki_int_p) {
			mki_send_key();
		}
		return;
	}
	uart2_tick = 0;

	//1 сек

	if (uart2_mup_tick < 60) { //60 сек
		uart2_mup_tick++;
	} else {
		uart2_mup_tick = 0;
		mup_send_query();
		return;
	}


	if ( uart2_mkk_tick < setting.control_time_sec) {
		 uart2_mkk_tick++;
	} else {
		uart2_mkk_tick = 0;
		if (uart2_mkk_send2) {
			if (setting.lines_count > 12) mkk_send_insig_lin(UART2_MUX_MKKL_2);
			else mkk_send_insig_lin(UART2_MUX_MKKL_1);
		} else {
			mkk_send_insig_lin(UART2_MUX_MKKL_1);
		}
		uart2_mkk_send2 = !uart2_mkk_send2;
		return;
	}
}

*/


//раз в 50 мс
void uart2_task_tx(void)
{


	if (uart2_tick < 198) uart2_tick++;
	else {
		uart2_tick = 0;
		return;
	}

	if (mki_int_p) {

		if (uart2_tick & 0x01) {
			mki_send_key(); //раз в 100 мс
			return;
		}
	}



	if (uart2_tick == 198) { //10 сек МУП
		//ulog("Send MUP"CRLF);
		mup_send_query();
	}
	else if ((uart2_tick == 40) ||  (uart2_tick == 120)) { //МКК 1
	//else if (uart2_tick == 60) { //МКК 1
		mkk_send_insig_lin(UART2_MUX_MKKL_1);
	}
	else if ((uart2_tick == 80) || (uart2_tick  == 160)) { //МКК 2
	//else if (uart2_tick == 130) { //МКК 2
		if (setting.lines_count > 12) mkk_send_insig_lin(UART2_MUX_MKKL_2);
	}

}

