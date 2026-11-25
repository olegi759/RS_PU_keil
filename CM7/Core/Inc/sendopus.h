/*
 * sendopus.h
 *
 *  Created on: 30 сент. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SENDOPUS_H_
#define CORE_INC_SENDOPUS_H_

#include "main.h"

BOOL sendopus_start(const char * file_name,  uint8_t daddr);
void sendopus_task_execute(void);
void sendopus_task(void);
void sendopus_stop(void);

extern uint32_t sendopus_samplerate;
extern uint8_t sendopus_channels;

extern BOOL sendopus_execute;
extern uint32_t sendopus_cnt;

void sendopus_task_it(void);

//int sendopus_frame_size;
#endif /* CORE_INC_SENDOPUS_H_ */
