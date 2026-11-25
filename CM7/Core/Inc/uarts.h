/*
 * uarts.h
 *
 *  Created on: 6 мар. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_UARTS_H_
#define CORE_INC_UARTS_H_

#include "main.h"
#include "dynbuf_util.h"

#define MAX_UART_PACKET_SIZE	124


void uarts_init(void);
void uarts_task(void);
void uarts_init_huart(UART_HandleTypeDef * huart);


void uarts_transmit_it(UART_HandleTypeDef *huart, uint8_t *packet, uint16_t packet_len);
void uarts_transmit_it_by_num(uint8_t huart_number, uint8_t * packet, uint16_t packet_len);
void uarts_transmit_piv(uint8_t huart_number, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);
//void uart4_uart7_snd(uint8_t * packet, uint16_t packet_len);

void uart8_uart5_send_opus_data(uint8_t * opus_data, int opus_data_len, uint8_t daddr, uint32_t frequency, BOOL stereo);


void HAL_UARTS_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

//extern uint32_t test_cmd_cnt[2];
//extern uint32_t test_snd_cnt[2];


void uarts_transmit_dejurniy(uint8_t daddr);
void uarts_transmit_veshanie_pd(uint8_t daddr, uint32_t frequency, BOOL stereo, uint32_t sound_lines);
void uarts_transmit_change_mode(void);

void uarts_task_data_rx(void);
extern dynamic_buf_t uarts_rx_buf;

void uarts_send_ring(void);
void uarts_ring_init(void);

//extern uint32_t uarts_transmit_it_tick;

extern uint32_t recv_opus_cnt;


//void uart8_uart5_transmit_opus(uint8_t daddr, uint8_t saddr, uint8_t opus_cmd, uint8_t * opus, uint16_t opus_len);


extern uint16_t count_tx_id;
void uart4_uart7_snd(uint8_t * packet, uint16_t packet_len);
void uart4_uart7_snd_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);

/* Third ring line number: UART4 + UART7 + I2C2.
   TX over I2C2 goes via CM4 (CORE_CMD_PD_I2C_SEND), RX -
   via CORE_CMD_PD_I2C_RECV into uart4_uart7_rcv(). */
#define UART_LINE_I2C2		9

void uart4_uart7_rcv(uint8_t huart_number, uint8_t * packet, uint16_t packet_len);

void uart8_uart5_ring_check(void);
void uart4_uart7_ring_check(void);

#endif /* CORE_INC_UARTS_H_ */
