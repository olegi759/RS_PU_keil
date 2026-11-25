/*
 * mkk.h
 *
 *  Created on: 19 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MKK_H_
#define CORE_INC_MKK_H_

#include "main.h"
#include "uart2.h"

void mkk_send(uart2_mux_t mux, uint8_t cmd, uint8_t * data, uint8_t data_len);

void mkk_send_reboot(uart2_mux_t mux);
//void mkk_send_cfg(uart2_mux_t mux, uint8_t value);
void mkk_send_insig_on(uart2_mux_t mux, uint16_t value);
void mkk_send_lin_on(uart2_mux_t mux, uint16_t value);
//void mkk_send_start_on(uart2_mux_t mux);
void mkk_send_insig_lin(uart2_mux_t mux);
//void mkk_send_start(uart2_mux_t mux);
void mkk_send_lin(uart2_mux_t mux, uint16_t um1, uint16_t um2);
void mkk_send_lin_met(uart2_mux_t mux, uint8_t value);


void mkk_send_version(uart2_mux_t mux);

void mkk_send_relay(uart2_mux_t mux, uint8_t state);


void mkk_send_relay_mux(uart2_mux_t mux, uint8_t relay_number, BOOL on);
BOOL mkk_get_relay_mux(uart2_mux_t mux, uint8_t relay_number);


void mkk_rcv(uart2_mux_t mux, uint16_t * data, uint16_t data_len);

void mkk_send_insig_on_from_setting(uart2_mux_t mux);
void mkk_send_lin_on_from_setting(uart2_mux_t mux);
void mkk_send_lin_from_setting(uart2_mux_t mux);

void mkk_change(void);

//uint8_t mkk_fire_lines_cnt(void);

//extern uint8_t mkk_rcv_sound[24];
//extern uint8_t mkk_rcv_insig[24];
//extern uint8_t mkk_rcv_start[2];


//const char * mkk_get_lin_state_str(uint8_t line_number);
//const char * mkk_get_insig_state_str(uint8_t line_number);
//const char * mkk_get_start_state_str(uint8_t line_number);

//uint32_t mkk_get_fire_lines(void);


#endif /* CORE_INC_MKK_H_ */
