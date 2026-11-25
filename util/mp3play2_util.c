/*
 * mp3read.c
 *
 *  Created on: 6 мая 2024 г.
 *      Author: Rafael Boltachev
 */

#include "mp3play2_util.h"
#include "mp3read_util.h"
#include "pcm_util.h"
#include "dac_util.h"


static dacplaybuf_data_t * mp3play2_dacplaybuf_data = NULL;
static uint16_t mp3play2_volume = 100;
static BOOL mp3play2_enable = FALSE;
static uint32_t mp3play2_mem_size = 0;
static void * mp3play2_mem = NULL;



void mp3play2_stop(void)
{
	if (f_is_open(&f_global_file)) {
		f_close(&f_global_file);
		ulog("mp3play2: close MP3 file"CRLF);
	}
	mad_data_clear(&global_mad_data);
	if (dacplay_is_dma_started(mp3play2_dacplaybuf_data->dacplay_data)) dacplay_stop_dma(mp3play2_dacplaybuf_data->dacplay_data);
	dynamic_buf_clear(mp3play2_dacplaybuf_data->dynamic_buf);
	mp3play2_enable = FALSE;
	//mp3play_on_stop();
}


void mp3play2_task_data(void)
{
	int16_t dac12[MAX_MAD_PCM_SIZE];
	int rc;




	rc = mp3read_read_dac12_mono(&f_global_file, &global_mad_data, dac12);
	if (rc < 0) {
		mp3play2_stop();
		return;
	}

	if (!mp3play2_dacplaybuf_data->frequency && global_mad_data->frame.header.samplerate) {
		mp3play2_dacplaybuf_data->frequency = global_mad_data->frame.header.samplerate
	}


	if (rc == 0) return;

   	if (mp3play2_volume != 100) {
   		dac12_set_volume(dac12, dac12, rc, mp3play2_volume);
   	}

	dacplaybuf_push(mp3play2_dacplaybuf_data, dac12, rc);
}

void mp3play2_task(void)
{
	if (!mp3play2_enable) return;

	if (dynamic_buf_get_free_size(mp3play2_dacplaybuf_data->dynamic_buf) < MAX_MAD_PCM_SIZE * 2) return;

	mp3play2_task_data();
}




BOOL mp3play2_from_file(const char * file_name, dacplaybuf_data_t * dacplaybuf_data, uint16_t volume)
{


	if (!strlen(file_name)) return FALSE;

	if (f_is_open(&f_global_file)) f_close(&f_global_file);
	mad_data_clear(&global_mad_data);
	if (dacplay_is_dma_started(dacplaybuf_data->dacplay_data)) dacplay_stop_dma(dacplaybuf_data->dacplay_data);
	dynamic_buf_clear(dacplaybuf_data->dynamic_buf);


	if (!mp3read_open(&f_global_file, &global_mad_data, file_name)) {
	     ulog_fmt("mp3play2: error open file: %s"CRLF, file_name);
	     return FALSE;
	}

	mp3play2_volume = volume;
    dacplaybuf_data->frequency = 0;
    mp3play2_dacplaybuf_data = dacplaybuf_data;
    mp3play2_enable = TRUE;

    ulog_fmt("mp3play2: play file:%s volume:%d play_on:%d"CRLF, file_name, volume, dacplaybuf_data->dacplay_data->id);
	return TRUE;
}


unsigned int memplay_input_read(void * file, void * read_start, unsigned int read_size)
{

	size_t read;

	if (!mp3play2_mem_size) return 0;

	read = MIN(mp3play2_mem_size, read_size);  //минимум из доступной и необходимой

	memcpy(read_start, mp3play2_mem, read); //читаем из памяти
	mp3play2_mem += read;
	mp3play2_mem_size -= read;

	return read;
}


BOOL mp3play2_from_mem(void * mem, uint32_t mem_size, dacplaybuf_data_t * dacplaybuf_data, uint16_t volume)
{
	if (!mem || !mem_size || !dacplaybuf_data || !volume) return FALSE;

	mad_data_clear(&global_mad_data);
	if (dacplay_is_dma_started(dacplaybuf_data->dacplay_data)) dacplay_stop_dma(dacplaybuf_data->dacplay_data);
	dynamic_buf_clear(dacplaybuf_data->dynamic_buf);


	global_mad_data->file = NULL;
	global_mad_data->mad_input_read = memplay_input_read;
	mad_stream_init(&global_mad_data->stream);
	mad_frame_init(&global_mad_data->frame);
	global_mad_data->frame.header.samplerate = 0;
	mad_synth_init(&global_mad_data->synth);
	global_mad_data->synth.pcm.length = 0;
	mad_stream_options(&global_mad_data->stream, 0); // TODO - add options support




	mp3play2_volume = volume;
    dacplaybuf_data->frequency = 0;
    mp3play2_dacplaybuf_data = dacplaybuf_data;
    mp3play2_mem_size = mem_size;
    mp3play2_mem = mem;
    mp3play2_enable = TRUE;

    ulog_fmt("mp3play2: play mem volume:%d play_on:%d"CRLF, volume, dacplaybuf_data->dacplay_data->id);
	return TRUE;
}


