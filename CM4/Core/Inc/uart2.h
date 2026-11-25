
/*
 * uart2.h
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_UART2_H_
#define CORE_INC_UART2_H_

#include "main.h"

#define MAX_UART2_PACKET_SIZE	100

typedef enum {
	UART2_MUX_EXT1 = 0,
	UART2_MUX_EXT2,
	UART2_MUX_EXT3,
	UART2_MUX_EXT4
} uart2_mux_t;

#define UART2_MUX_MKI		UART2_MUX_EXT1
#define UART2_MUX_MKKL_1	UART2_MUX_EXT2
#define UART2_MUX_MKKL_2	UART2_MUX_EXT3
#define UART2_MUX_MUP		UART2_MUX_EXT4



void uart2_set_mux(uart2_mux_t mux);
uart2_mux_t uart2_get_mux(void);
void uart2_transmit(uart2_mux_t mux, uint16_t * packet, uint16_t packet_len);
void uart2_task_rx(void);
void uart2_task_tx(void);
void HAL_UART2_RxEventCallback(uint16_t Size);
//uint16_t * uart2_get_tx_buf(void);

#endif /* CORE_INC_UART2_H_ */
