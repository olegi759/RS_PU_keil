/*
 * opus_buffer.h
 *
 *  Created on: 21 июл. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_OPUS_BUFFER_H_
#define CM7_CORE_INC_OPUS_BUFFER_H_

#include "main.h"

//extern uint8_t opus_buffer_strategy;

void opus_buffer_init(void);
void opus_buffer_clear(void);

BOOL opus_buffer_put(uint8_t * opus, uint16_t opus_len);
int opus_buffer_procent_full(void);
void opus_buffer_task(void);


//BOOL pu_put_opus_to_choose(uint8_t * opus, uint16_t opus_len);

#endif /* CM7_CORE_INC_OPUS_BUFFER_H_ */
