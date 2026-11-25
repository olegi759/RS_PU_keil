/*
 * adcplay_util.c
 *
 *  Created on: 20 мая 2024 г.
 *      Author: Rafael Boltachev
 */


#include "adcplay_util.h"
#include "stm_util.h"


void adcplay_HalfCpltCallback(adcplay_data_t * adcplay_data)
{
	adcplay_data->index = 0;
#ifdef USE_ADCPLAY_WRITE_DATA
	adcplay_write_data(adcplay_data, adcplay_data->dma_buf[0]);
#endif
}

void adcplay_FullCpltCallback(adcplay_data_t * adcplay_data)
{
	adcplay_data->index = 1;
#ifdef USE_ADCPLAY_WRITE_DATA
	adcplay_write_data(adcplay_data, adcplay_data->dma_buf[1]);
#endif
}

//#ifndef ADCPLAY_VERSION_DYNAMIC

void adcplay_stop_dma(adcplay_data_t * adcplay_data)
{
	if (!adcplay_data) return;
	HAL_ADC_Stop_DMA(adcplay_data->hadc);
	adcplay_data->dma_start = FALSE;
	adcplay_data->index = 0xFF;
	ulog_fmt("adcplay: stop dma (id:%d)"CRLF, adcplay_data->id);
}

BOOL adcplay_is_dma_started(adcplay_data_t * adcplay_data)
{
	if (!adcplay_data) return FALSE;
	return adcplay_data->dma_start;
}

BOOL adcplay_start_dma(adcplay_data_t * adcplay_data)
{
	HAL_StatusTypeDef hs;

	if (!adcplay_data) return FALSE;
	if (adcplay_data->dma_start) adcplay_stop_dma(adcplay_data);
	adcplay_data->index = 0xFF;
	hs = HAL_ADC_Start_DMA(adcplay_data->hadc, (uint32_t *)adcplay_data->dma_buf, ADCPLAY_DMA_BUF_SIZE * 2);
	if (hs != HAL_OK) {
		ulog_fmt("adcplay: error start dma(id:%d)=%d"CRLF, adcplay_data->id, hs);
		return FALSE;
	}
	adcplay_data->dma_start = TRUE;
	ulog_fmt("adcplay: start dma(id:%d)=%d"CRLF, adcplay_data->id, hs);
	return TRUE;
}




void adcplay_init(adcplay_data_t * adcplay_data, uint8_t id, ADC_HandleTypeDef * hadc)
{
	if (!adcplay_data) return;
	adcplay_data->id = id;
	adcplay_data->dma_start = FALSE;
	adcplay_data->hadc = hadc;
	adcplay_data->index = 0xFF;
}

//#endif

/*
#ifdef ADCPLAY_VERSION_DYNAMIC


void adcplay_stop_dma(adcplay_data_t * adcplay_data)
{
	if (!adcplay_data) return;
	HAL_ADC_Stop_DMA(adcplay_data->hadc);
	adcplay_data->index = 0xFF;
	if (adcplay_data->buf) {
		free(adcplay_data->buf);
		adcplay_data->buf = NULL;
		adcplay_data->dma_buf[0] = NULL;
		adcplay_data->dma_buf[1] = NULL;
	}
	ulog_fmt("adcplay: stop dma (id:%d)"CRLF, adcplay_data->id);
}


BOOL adcplay_is_dma_started(adcplay_data_t * adcplay_data)
{
	return adcplay_data->buf ? TRUE : FALSE;
}

BOOL adcplay_start_dma(adcplay_data_t * adcplay_data)
{
	if (!adcplay_data) return FALSE;
	if (adcplay_data->buf) adcplay_stop_dma(adcplay_data);
	adcplay_data->buf  = (uint8_t *)malloc(ADCPLAY_DMA_BUF_SIZE * 4);
	if (!adcplay_data->buf) return FALSE;
	adcplay_data->dma_buf[0] = (uint16_t *)adcplay_data->buf;
	adcplay_data->dma_buf[1] = (uint16_t *)(adcplay_data->buf + DACPLAY_DMA_BUF_SIZE * 2);
	adcplay_data->index = 0xFF;
	HAL_ADC_Start_DMA(adcplay_data->hadc, (uint32_t *)adcplay_data->buf, ADCPLAY_DMA_BUF_SIZE * 2);
	ulog_fmt("adcplay: start dma(id:%d)"CRLF, adcplay_data->id);
	return TRUE;
}

void adcplay_init(adcplay_data_t * adcplay_data, uint8_t id, ADC_HandleTypeDef * hadc)
{
	adcplay_data->id = id;
	adcplay_data->hadc = hadc;
	adcplay_data->index = 0xFF;
	adcplay_data->buf = NULL;
	adcplay_data->dma_buf[0] = NULL;
	adcplay_data->dma_buf[1] = NULL;
}

#endif
*/
