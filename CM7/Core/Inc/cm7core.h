/*
 * cm7core.h
 *
 *  Created on: 21 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_CM7CORE_H_
#define CM7_CORE_INC_CM7CORE_H_

#include "main.h"
#include "setting_data.h"

void cm7core_init(void);
void cm7core_task(void);

BOOL cm7core_send_to_CM4(uint8_t * packet, uint16_t packet_len);
BOOL cm7core_send_to_CM4_cmd(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);
BOOL cm7core_send_to_CM4_cmd_only(uint8_t cmd);


char * cm7core_get_date_time_str(void);
char * cm7core_get_time_str(void);



BOOL cm7core_open_file(const char * file_name);
uint16_t cm7core_read_file(uint8_t * buf, uint16_t buf_len);
void cm7core_close_file(void);
BOOL cm7core_file_exist(const char * file_name);
void cm7core_mki_set_display(char * str);

void cm7core_send_one_led_color(uint8_t number, uint8_t color);
void cm7core_set_led_color(uint8_t number, uint8_t color);
void cm7core_send_leds(void);
//void cm7core_um_send_mode_all(uint8_t mode_ch1, uint8_t mode_ch2);
void cm7core_mkk_send_lin(uint8_t cmd, uint16_t um1, uint16_t um2);
void cm7core_mkk_send_relay_mux(uint8_t cmd, uint8_t relay, BOOL on);
void cm7core_pd_send(uint8_t * packet, uint16_t packet_len);
void cm7core_i2c_send(uint8_t * packet, uint16_t packet_len);
//void cm7core_add_log(char * str);
uint16_t cm7core_get_opus_list(char * buf, uint16_t buf_len);
uint16_t cm7core_get_mp3_and_opus_list(char * buf, uint16_t buf_len);


void cm7core_fault_set(uint8_t number);
void cm7core_fault_unset(uint8_t number);

void cm7core_mki_test(uint8_t enable);

const char * cm7_mkk_get_insig_state_str(uint8_t line_number);
const char * cm7_mkk_get_lin_state_str(uint8_t line_number);
const char * cm7_mkk_get_start_state_str(uint8_t line_number);

BOOL cm7core_wait_cmd_from_CM4(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_size, uint32_t timeout, uint16_t * cmd_data_len);

void cm7core_option_change(void);



#endif /* CM7_CORE_INC_CM7CORE_H_ */
