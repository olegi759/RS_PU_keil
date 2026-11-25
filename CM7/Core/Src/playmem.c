/*
 * playmem.c
 *
 *  Created on: 3 СЃРµРЅС‚. 2026 Рі.
 *      Author: Rafael Boltachev
 */



#include "playmem.h"
#include <string.h>
#include "stm_util.h"
#include "str_util.h"
#include "pcm1753.h"
#include "pu.h"
#include "cm7core.h"
#include "mad_util.h"





static BOOL playmem_execute = FALSE;
uint8_t playmem_channels = 0;
uint32_t playmem_sample_rate = 0;
static uint8_t * playmem_mem = NULL;
static unsigned int playmem_size = 0;


BOOL playmem_is_execute(void)
{
	return playmem_execute;
}

void playmem_stop(void)
{

	if (playmem_execute) {
		playmem_execute = FALSE;
		mad_data_clear(&global_mad_data);
		pcm1753_DataFinish();
		playmem_channels = 0;
		playmem_sample_rate = 0;
		ulog_fmt(CONM"playmem: stop tick:%lu"CRLF, (unsigned long)HAL_GetTick());
	}

}



void playmem_task_data(void)
{
	int rc;
	int16_t pcm16[MAX_MAD_PCM_SIZE];


	rc = mad_read_next_frame(&global_mad_data);
	if (rc == -1) {
		playmem_stop();
		return;
	}
	if (rc != 1)  return;

	mad_synth_frame(&global_mad_data.synth, &global_mad_data.frame);

	if (global_mad_data.frame.header.samplerate && !playmem_sample_rate) {
		playmem_sample_rate = global_mad_data.frame.header.samplerate;
		playmem_channels = (global_mad_data.frame.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);

		pu_prepare_pcm1753(playmem_sample_rate, playmem_channels == 2);

		ulog_fmt(CONM"playmem: frequency:%lu, channels:%d"CRLF, (unsigned long)playmem_sample_rate, playmem_channels);


	}

	rc = mad_output_pcm16(&global_mad_data.synth.pcm, pcm16, MAX_MAD_PCM_SIZE);
	if (rc > 0) pcm1753_AudioBufferLwrbWrite(pcm16, rc);

}


void playmem_task(void)
{
	if (playmem_execute) {
		if (pcm1753_AudioBufferGetFree() > pcm1753_AudioBufferGetFull()) {
			playmem_task_data();
		}
	}

}


unsigned int playmem_read_file(void * afile, void * data, unsigned int data_len)
{

	size_t read;

	if (!playmem_size) return 0;

	read = MIN(playmem_size,  data_len);  //РјРёРЅРёРјСѓРј РёР· РґРѕСЃС‚СѓРїРЅРѕР№ Рё РЅРµРѕР±С…РѕРґРёРјРѕР№

	memcpy(data, playmem_mem, read); //С‡РёС‚Р°РµРј РёР· РїР°РјСЏС‚Рё
	playmem_mem += read;
	playmem_size -= read;

	return read;
}



BOOL playmem_start(void * mem, unsigned int mem_size)
{


	if (!mem || !mem_size) {
		ulog(CONM"playmem: start errors"CRLF);
		return FALSE;
	}

	mad_data_clear(&global_mad_data);

	global_mad_data.file = NULL;
	global_mad_data.mad_input_read = playmem_read_file;
	mad_stream_init(&global_mad_data.stream);
	mad_frame_init(&global_mad_data.frame);
	global_mad_data.frame.header.samplerate = 0;
	mad_synth_init(&global_mad_data.synth);
	global_mad_data.synth.pcm.length = 0;
	mad_stream_options(&global_mad_data.stream, 0); // TODO - add options support


    playmem_execute = TRUE;
    playmem_sample_rate = 0;
    playmem_channels = 0;

    playmem_mem = (uint8_t *)mem;
    playmem_size = mem_size;


	ulog_fmt(CONM"playmem: tick:%lu"CRLF, (unsigned long)HAL_GetTick());

	return TRUE;

}
