/*
 * playopus.c
 *
 *  Created on: 30 СЃРµРЅС‚. 2024 Рі.
 *      Author: Rafael Boltachev
 */



#include "playopus.h"
#include <string.h>
#include "ogg_util.h"
#include "fatfs_util.h"
#include "stm_util.h"
#include "opus_util.h"
#include "dynbuf_util.h"
#include "uarts.h"
#include "str_util.h"
#include "pcm1753.h"
#include "mux_sound.h"
#include "playopus.h"
#include "oggopus_util.h"
#include "pu.h"
#include "cm7core.h"



//#define PLAYOPUS_FILE 			f_global_file
//static FIL playopus_file = {0};

#define PLAYOPUS_OGG_DECODER 	global_ogg_decoder
//static ogg_decoder_t ogg_decoder = {0};


static BOOL playopus_execute = FALSE;
uint8_t playopus_channels = 0;
uint32_t playopus_sample_rate = 0;


BOOL playopus_is_execute(void)
{
	return playopus_execute;
}

void playopus_stop(void)
{

	if (playopus_execute) {
		playopus_execute = FALSE;
		cm7core_close_file();
		decoder_opus_destroy();
		pcm1753_DataFinish();
		playopus_sample_rate = 0;
		playopus_channels = 0;
		ulog_fmt(CONM"playopus: stop tick:%lu"CRLF, (unsigned long)HAL_GetTick());
	}

}



void playopus_task_data(void)
{
	uint8_t segment[OPUS_MAX_ENCODE_SIZE];
	int rc;



	rc = oggopus_read_segment(&PLAYOPUS_OGG_DECODER, segment);
    if (!rc) {
		playopus_stop();
		return;
    }

    pu_put_opus_in_pcm1753_now(segment, rc);
}


void playopus_task(void)
{
	if (playopus_execute) {
		if (pcm1753_AudioBufferGetFree() > pcm1753_AudioBufferGetFull()) {
			playopus_task_data();
		}
	}

}


unsigned int playopus_read_file(void * afile, void * data, unsigned int data_len)
{
	return cm7core_read_file(data, data_len);
}

BOOL playopus_start(const char * file_name)
{

	//ulog_fmt("playopus_start(%s, %d, %s)"CRLF, file_name, volume, bool2str(external_flash));


	if (!cm7core_open_file(file_name)) {
		ulog_fmt(CONM"playopus: start error open file: %s"CRLF, file_name);
		return FALSE;
	}

	ulog_fmt(CONM"playopus: open ogg: %s OK"CRLF, file_name);
    ogg_decoder_init(&PLAYOPUS_OGG_DECODER, NULL,  playopus_read_file);


    if (!oggopus_open(&PLAYOPUS_OGG_DECODER, &playopus_sample_rate, &playopus_channels)) {
		ulog_fmt(CONM"playopus: start error open ogg: %s"CRLF, file_name);
		cm7core_close_file();
		return FALSE;
    }

    //ulog("playopus: call to find header"CRLF);



    if (!pu_prepare_audio_play(playopus_sample_rate, playopus_channels == 2)) {
		cm7core_close_file();
		return FALSE;
    }


	ulog_fmt(CONM"playopus: file:%s, frequency:%lu, channels:%d tick:%lu"CRLF, file_name, (unsigned long)playopus_sample_rate, playopus_channels, (unsigned long)HAL_GetTick());

    playopus_execute = TRUE;

    //set_mux_sound_left_right_enable(TRUE);

	return TRUE;

}

