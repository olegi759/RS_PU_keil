/*
 * playmp3.h
 *
 *  Created on: 14 июл. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_PLAYMP3_H_
#define CM7_CORE_INC_PLAYMP3_H_

#include "main.h"

BOOL playmp3_is_execute(void);
void playmp3_stop(void);
void playmp3_task(void);
BOOL playmp3_start(const char * file_name);

extern uint8_t playmp3_channels;
extern uint32_t playmp3_sample_rate;

#endif /* CM7_CORE_INC_PLAYMP3_H_ */
