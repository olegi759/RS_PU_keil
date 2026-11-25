/*
 * um.h
 *
 *  Created on: 20 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_UM_H_
#define CORE_INC_UM_H_

#include "main.h"
#include "um_data.h"

void um_rcv(uint8_t * data, uint16_t data_len);
void um_snd_query(void);

void um_snd_modes(uint8_t addr, uint8_t mode_ch1, uint8_t mode_ch2);
void um_snd_modes_all(uint8_t mode_ch1, uint8_t mode_ch2);
void um_snd_set_addr(uint8_t um_addr, uint8_t set_addr);

void um_transmit_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);

#endif /* CORE_INC_UM_H_ */
