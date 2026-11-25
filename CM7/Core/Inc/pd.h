/*
 * pd.h
 *
 *  Created on: 20 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_PD_H_
#define CORE_INC_PD_H_

#include "main.h"

void pd_rcv(uint8_t * packet, uint16_t packet_len, uint8_t huart_number);

//void pd_parse_rcv(uart6_mux_t mux, uint8_t * data, uint16_t data_len, uint8_t huart_number);
//void pd_rcv(uart6_mux_t mux, uint8_t * data, uint16_t data_len);
//void pd_snd_query(void);

BOOL pd_start_translation(uint32_t freq, BOOL stereo);


//void pd_on_sai_full(void);
//void pd_on_sai_empty(void);


//void pd_transmit_piv_nomux(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);

extern uint32_t pu_need_lines;
extern uint8_t pd_translation_addr;

//void pd_init(void);
extern uint32_t pd_id;

BOOL pd_get_audio_param_from_command(uint8_t cmd, uint32_t * freq, BOOL * stereo);
uint8_t pd_get_opus_cmd_from_frequency_stereo(uint32_t frequency, BOOL stereo);


#endif /* CORE_INC_PD_H_ */
