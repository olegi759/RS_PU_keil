/*
 * playopus.h
 *
 *  Created on: 30 сент. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_PLAYOPUS_H_
#define CORE_INC_PLAYOPUS_H_

#include "main.h"

void playopus_task(void);
BOOL playopus_start(const char * file_name);
void playopus_stop(void);
BOOL playopus_is_execute(void);

void on_play_opus_stop(void);

extern uint8_t playopus_channels;
extern uint32_t playopus_sample_rate;


#endif /* CORE_INC_PLAYOPUS_H_ */
