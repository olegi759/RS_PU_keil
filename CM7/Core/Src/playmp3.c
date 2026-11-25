/*
 * playmp3.c
 *
 *  Created on: 14 РёСЋР». 2026 Рі.
 *      Author: Rafael Boltachev
 */


#include "playmp3.h"
#include <string.h>
#include "stm_util.h"
#include "str_util.h"
#include "pcm1753.h"
#include "pu.h"
#include "cm7core.h"
#include "mad_util.h"





static BOOL playmp3_execute = FALSE;
uint8_t playmp3_channels = 0;
uint32_t playmp3_sample_rate = 0;


BOOL playmp3_is_execute(void)
{
	return playmp3_execute;
}

void playmp3_stop(void)
{

	if (playmp3_execute) {
		playmp3_execute = FALSE;
		cm7core_close_file();
		mad_data_clear(&global_mad_data);
		pcm1753_DataFinish();
		playmp3_channels = 0;
		playmp3_sample_rate = 0;
		ulog_fmt(CONM"playmp3: stop tick:%lu"CRLF, (unsigned long)HAL_GetTick());
	}

}



void playmp3_task_data(void)
{
	int rc;
	int16_t pcm16[MAX_MAD_PCM_SIZE];


	rc = mad_read_next_frame(&global_mad_data);
	if (rc == -1) {
		playmp3_stop();
		return;
	}
	if (rc != 1)  return;

	mad_synth_frame(&global_mad_data.synth, &global_mad_data.frame);

	if (global_mad_data.frame.header.samplerate && !playmp3_sample_rate) {
		playmp3_sample_rate = global_mad_data.frame.header.samplerate;
		playmp3_channels = (global_mad_data.frame.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);

		pu_prepare_pcm1753(playmp3_sample_rate, playmp3_channels == 2);

		ulog_fmt(CONM"playmp3: frequency:%lu, channels:%d"CRLF, (unsigned long)playmp3_sample_rate, playmp3_channels);


	}

	rc = mad_output_pcm16(&global_mad_data.synth.pcm, pcm16, MAX_MAD_PCM_SIZE);
	if (rc > 0) pcm1753_AudioBufferLwrbWrite(pcm16, rc);

}


void playmp3_task(void)
{
	if (playmp3_execute) {
		if (pcm1753_AudioBufferGetFree() > pcm1753_AudioBufferGetFull()) {
			playmp3_task_data();
		}
	}

}


unsigned int playmp3_read_file(void * afile, void * data, unsigned int data_len)
{
	return cm7core_read_file(data, data_len);
}

BOOL playmp3_start(const char * file_name)
{


	if (!cm7core_open_file(file_name)) {
		ulog_fmt(CONM"playmp3: start error open file: %s"CRLF, file_name);
		return FALSE;
	}

	mad_data_clear(&global_mad_data);

	global_mad_data.file = NULL;
	global_mad_data.mad_input_read = playmp3_read_file;
	mad_stream_init(&global_mad_data.stream);
	mad_frame_init(&global_mad_data.frame);
	global_mad_data.frame.header.samplerate = 0;
	mad_synth_init(&global_mad_data.synth);
	global_mad_data.synth.pcm.length = 0;
	mad_stream_options(&global_mad_data.stream, 0); // TODO - add options support


    playmp3_execute = TRUE;
    playmp3_sample_rate = 0;
    playmp3_channels = 0;

	ulog_fmt(CONM"playmp3: file:%s tick:%lu"CRLF, file_name, (unsigned long)HAL_GetTick());

	return TRUE;

}

