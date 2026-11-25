/*
 * dacplaybuf_util.c
 *
 *  Created on: 19 февр. 2024 г.
 *      Author: Rafael Boltachev
 */

#include "dacplaybuf_util.h"
#include "stm_util.h"


BOOL dacplaybuf_push(dacplaybuf_data_t * dacplaybuf_data, uint16_t * dac_data, uint32_t dac_data_len)
{

	dacplaybuf_data->tick = HAL_GetTick();

	if (!dynamic_buf_push(dacplaybuf_data->dynamic_buf, dac_data, dac_data_len * 2)) {
		ulog("dacplaybuf: FULL"CRLF);
		return FALSE;
	}


	if (dacplaybuf_data->dacplay_data->dma_started == FALSE) { //дма не запущено


		if (dynamic_buf_get_data_size(dacplaybuf_data->dynamic_buf) >  dynamic_buf_get_free_size(dacplaybuf_data->dynamic_buf)) {
			set_tim_frequency2_and_start(dacplaybuf_data->htim, dacplaybuf_data->frequency, BASE_FREQUENCY);
			dacplay_start_dma(dacplaybuf_data->dacplay_data);
		}

	}

	return TRUE;
}

//вызвать из dacplay_read_data(dacplay_data_t * dacplay_data, uint16_t * dac_data); или из task
void dacplaybuf_on_read_data(dacplaybuf_data_t * dacplaybuf_data, uint16_t * dac_data)
{
	size_t data_len, i;

	dynamic_buf_pop_line(dacplaybuf_data->dynamic_buf, dac_data, DACPLAY_DMA_BUF_SIZE * 2, &data_len);


	if (!data_len) {
		ulog("dacplaybuf: EMPTY"CRLF);

		if (HAL_DiffTick(dacplaybuf_data->tick) > DACPLAYBUF_TIMEOUT) {
			dacplaybuf_stop_dma(dacplaybuf_data);
			dacplaybuf_on_stop_dma(dacplaybuf_data);
		}
	} else	{
		if (data_len < DACPLAY_DMA_BUF_SIZE * 2) {
			ulog("dacplaybuf: SMALL DATA"CRLF);
			for (i = data_len / 2; i < DACPLAY_DMA_BUF_SIZE; i++) dac_data[i] =  DACPLAY_ZERO_VALUE;
		}
		dacplaybuf_data->tick = HAL_GetTick();
	}

}


//задача
void dacplaybuf_task(dacplaybuf_data_t * dacplaybuf_data)
{
	if (dacplaybuf_data->dacplay_data->dma_started == FALSE) return;

	if (dacplaybuf_data->dacplay_data->index < 2) {
		dacplaybuf_on_read_data(dacplaybuf_data, dacplaybuf_data->dacplay_data->dma_buf[dacplaybuf_data->dacplay_data->index]);
		dacplaybuf_data->dacplay_data->index = 0xFF;
	}
}

void dacplaybuf_stop_dma(dacplaybuf_data_t * dacplaybuf_data)
{
	dacplay_stop_dma(dacplaybuf_data->dacplay_data);
	dynamic_buf_clear(dacplaybuf_data->dynamic_buf);
	ulog("dacplaybuf: STOP"CRLF);
}




//остальные ф-ции можно не использовать
/*
void dacplaybuf_set_htim(dacplaybuf_data_t * dacplaybuf_data, TIM_HandleTypeDef * htim)
{
	dacplaybuf_data->htim = htim;
}

void dacplaybuf_set_dacplay_data(dacplaybuf_data_t * dacplaybuf_data, dacplay_data_t * dacplay_data)
{
	dacplaybuf_data->dacplay_data = dacplay_data;
}


void dacplaybuf_set_dynamic_buf_size(dacplaybuf_data_t * dacplaybuf_data, uint32_t buf_size)
{
	dacplaybuf_data->dynamic_buf->max_alloc_size =  buf_size;
}

void dacplaybuf_set_dynamic_buf(dacplaybuf_data_t * dacplaybuf_data, dynamic_buf_t * dynamic_buf)
{
	dacplaybuf_data->dynamic_buf =  dynamic_buf;
}

//Событие по половине DMA
void dacplaybuf_HalfCpltCallback(dacplaybuf_data_t * dacplaybuf_data)
{

	dacplay_HalfCpltCallback(dacplaybuf_data->dacplay_data);
}

//Событие по целой DMA
void dacplaybuf_FullCpltCallback(dacplaybuf_data_t * dacplaybuf_data)
{
	dacplay_FullCpltCallback(dacplaybuf_data->dacplay_data);
}




BOOL dacplaybuf_is_dma_started(dacplaybuf_data_t * dacplaybuf_data)
{
	return dacplay_is_dma_started(dacplaybuf_data->dacplay_data);
}

//Установка частоты на DAC
void dacplaybuf_set_frequency(dacplaybuf_data_t * dacplaybuf_data, uint32_t Frequency)
{
	dacplaybuf_data->frequency = Frequency;
}

uint32_t dacplaybuf_get_frequency(dacplaybuf_data_t * dacplaybuf_data)
{
	return dacplaybuf_data->frequency;
}



size_t dacplaybuf_get_free(dacplaybuf_data_t * dacplaybuf_data)
{
	return dynamic_buf_get_free_size(dacplaybuf_data->dynamic_buf);
}

size_t dacplaybuf_get_full(dacplaybuf_data_t * dacplaybuf_data)
{
	return dynamic_buf_get_data_size(dacplaybuf_data->dynamic_buf);
}

void dacplaybuf_reset(dacplaybuf_data_t * dacplaybuf_data)
{
	 dynamic_buf_clear(dacplaybuf_data->dynamic_buf);
}
*/

