/*
 * uart6.h
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_UART6_H_
#define CORE_INC_UART6_H_

#include "main.h"


typedef enum {
	UART6_MUX_EXT_AMP = 0,
	UART6_MUX_P1C,
	UART6_MUX_P2C,
	UART6_MUX_TMP
} uart6_mux_t;


#define MAX_UART6_PACKET_TX_SIZE 	400
#define MAX_UART6_PACKET_RX_SIZE 	200

void uart6_set_mux(uart6_mux_t mux);
uart6_mux_t uart6_get_mux(void);
//void uart6_transmit(uart6_mux_t mux, uint8_t * data, uint16_t data_len);
void uart6_transmit_piv(uart6_mux_t mux, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);
void uart6_task_rx(void);
void uart6_task_tx(void);

void HAL_UART6_RxEventCallback(uint16_t Size);
void HAL_UART6_TxCpltCallback(void);
//uint8_t * uart6_get_tx_buf(void);
void uart6_transmit_to_pd_nomux(uint8_t * packet, uint16_t packet_len);


#endif /* CORE_INC_UART6_H_ */
