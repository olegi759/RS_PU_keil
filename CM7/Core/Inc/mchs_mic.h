/*
 * mchs.h
 *
 *  Created on: 17 янв. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MCHS_MIC_H_
#define CORE_INC_MCHS_MIC_H_

#include "main.h"

#define MCHS_MIC_AUDIO_FREQUENCY	SAI_AUDIO_FREQUENCY_16K
#define MCHS_MIC_DMA_BUF_SIZE		960

void mchs_mic_start(BOOL is_mic, uint8_t daddr);
void mchs_mic_stop(void);



void mchs_mic_task(void);
void mchs_mic_init(void);
void mchs_mic_task_in_mchs(void);

void mchs_mic_on_sai_full(void);
void mchs_mic_on_sai_empty(void);

BOOL mchs_mic_is_dma_started(void);


extern unsigned short mic_min;
extern unsigned short mic_max;

#endif /* CORE_INC_MCHS_MIC_H_ */
