/*
 * playmem.h
 *
 *  Created on: 3 сент. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_PLAYMEM_H_
#define CM7_CORE_INC_PLAYMEM_H_

#include "main.h"

BOOL playmem_is_execute(void);
void playmem_stop(void);
void playmem_task(void);
BOOL playmem_start(void * mem, size_t mem_size);

extern uint8_t playmem_channels;
extern uint32_t playmem_sample_rate;


#endif /* CM7_CORE_INC_PLAYMEM_H_ */
