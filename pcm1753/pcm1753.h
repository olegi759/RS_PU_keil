/*
 * pcm1753.h
 *
 *  Created on: 15 нояб. 2023 г.
 *      Author: Rafael Boltachev
 */

#ifndef PCM1753_PCM1753_H_
#define PCM1753_PCM1753_H_

#include "main.h"

#ifndef SAI_DMA_BUFFER_SIZE
#define SAI_DMA_BUFFER_SIZE 	(960 * 2)
#endif

void pcm1753_task(void);

void pcm1753_start_DMA(void);

void pcm1753_stop_DMA(void);

//void pcm1753_init(void);

BOOL pcm1753_is_DMA_started(void);

//Установка моно/стерео на SAI
void pcm1753_SetStereo(BOOL value);
BOOL pcm1753_GetStereo(void);

//задать частоту
void pcm1753_SetFrequency(uint32_t Frequency, BOOL Stereo);
uint32_t pcm1753_GetFrequency(void);

//очистить буфер
void pcm1753_AudioBufferLwrbReset(void);
//и писать данные
void pcm1753_AudioBufferLwrbWrite(int16_t *pcmdata, uint16_t pcmdata_len);
//своюожное место в буфере
size_t pcm1753_AudioBufferGetFree(void);
//громкость
void pcm1753_SetVolume(uint16_t volume);

size_t pcm1753_AudioBufferGetFull(void);


//OTHER

//void pcm1753_play_sunus(uint32_t freq);


void pcm1753_on_sai_full(void);
void pcm1753_on_sai_empty(void);

//void pcm1753_SetAudioBufferSize(size_t size);

void pcm1753_OnStopDMA(void);

void pcm1753_SaiAudioBufferFill(void);
extern BOOL sai_is_DMA_started;

BOOL pcm1753_set_SaiAudioBufferDma(uint8_t ASaiAudioBuffeIndex, uint16_t num, int16_t value);

void pcm1753_DataFinish(void);

int pcm1753_GetAudioBufferProcentFull(void);

#endif /* PCM1753_PCM1753_H_ */
