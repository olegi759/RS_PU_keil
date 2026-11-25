/*
 * twocore.h
 *
 *  Created on: 20 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM4_CORE_INC_CM4CORE_H_
#define CM4_CORE_INC_CM4CORE_H_

#include "main.h"
#include "shared_memory.h"


void cm4core_init(void);
void cm4core_task(void);

void cm4core_send_to_CM7(uint8_t * packet, uint16_t packet_len);
void cm4core_send_to_CM7_cmd(uint8_t cmd, void * cmd_data, uint16_t cmd_data_len);
void cm4core_send_to_CM7_cmd_only(uint8_t cmd);


BOOL cm4core_wait_cmd_from_CM7(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_size, uint32_t timeout, uint16_t * cmd_data_len);

void cm4core_pd_rcv(uint8_t * packet, uint16_t packet_len);
void cm4core_pd_i2c_rcv(uint8_t * packet, uint16_t packet_len);
void cm4core_button_press(uint8_t key);
void cm4core_button_unpress(uint8_t key);
void cm4core_change_setting(void);


BOOL cm4core_get_cm7_version(char * buf, uint16_t buf_size);

extern uint32_t cm4_core_ping_tick;

#endif /* CM4_CORE_INC_CM4CORE_H_ */
