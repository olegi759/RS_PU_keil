/*
 * dac_play_util.h
 *
 *  Created on: 19 февр. 2024 г.
 *      Author: Rafael Boltachev
 *
 *      Библотека для воспроизведения звука на DAC через DMA (устарела)
 */

#ifndef DACPLAYBUF_UTIL_H_
#define DACPLAYBUF_UTIL_H_


//#define USE_DACPLAY_READ_DATA	1

#include "main.h"
#include "def_util.h"
#include "dynbuf_util.h"
#include "dacplay_util.h"


#define DACPLAYBUF_TIMEOUT	100

typedef struct _dacplaybuf_data {

	TIM_HandleTypeDef * htim;
	uint32_t frequency;


	dacplay_data_t * dacplay_data;
	dynamic_buf_t * dynamic_buf;

	uint32_t tick;

} dacplaybuf_data_t;


//Нужно заполнить  htim, frequency, dacplay_data, dynamic_buf. Для dynamic_buf указать max_alloc_size.
//И вызывать dacplay_HalfCpltCallback(dacplaybuf_data->dacplay_data) и 	dacplay_FullCpltCallback(dacplaybuf_data->dacplay_data);
//И вызывать dacplaybuf_task(dacplaybuf_data);
#define DACPLAYBUF_STATIC_INIT(htim, frequency, dacplay_data, dynamic_buf) 	{htim, frequency, dacplay_data, dynamic_buf, 0}

void dacplaybuf_push(dacplaybuf_data_t * dacplaybuf_data, uint16_t * dac_data, uint32_t dac_data_len);
void dacplaybuf_task(dacplaybuf_data_t * dacplaybuf_data);
void dacplaybuf_stop_dma(dacplaybuf_data_t * dacplaybuf_data);
//NEED
void dacplaybuf_on_stop_dma(dacplaybuf_data_t * dacplaybuf_data);




/*
void dacplaybuf_HalfCpltCallback(dacplaybuf_data_t * dacplaybuf_data);
void dacplaybuf_FullCpltCallback(dacplaybuf_data_t * dacplaybuf_data);

uint32_t dacplaybuf_get_frequency(dacplaybuf_data_t * dacplaybuf_data);
void dacplaybuf_set_frequency(dacplaybuf_data_t * dacplaybuf_data, uint32_t Frequency);

void dacplaybuf_set_htim(dacplaybuf_data_t * dacplaybuf_data, TIM_HandleTypeDef * htim);

void dacplaybuf_set_dacplay_data(dacplaybuf_data_t * dacplaybuf_data, dacplay_data_t * dacplay_data);

void dacplaybuf_set_dynamic_buf(dacplaybuf_data_t * dacplaybuf_data, dynamic_buf_t * dynamic_buf);
void dacplaybuf_set_dynamic_buf_size(dacplaybuf_data_t * dacplaybuf_data, uint32_t buf_size);




void dacplaybuf_on_read_data(dacplaybuf_data_t * dacplaybuf_data, uint16_t * dac_data);

size_t dacplaybuf_get_free(dacplaybuf_data_t * dacplaybuf_data); //*
size_t dacplaybuf_get_full(dacplaybuf_data_t * dacplaybuf_data); //*
void dacplaybuf_reset(dacplaybuf_data_t * dacplaybuf_data); //*


BOOL dacplaybuf_is_dma_started(dacplaybuf_data_t * dacplaybuf_data);

*/

#endif /* UTIL_dacplaybuf_UTIL_H_ */
