/*
 * sendopus.c
 *
 *  Created on: 30 СЃРµРЅС‚. 2024 Рі.
 *      Author: Rafael Boltachev
 */


#include "sendopus.h"
#include <string.h>
#include "ogg_util.h"
#include "fatfs_util.h"
#include "stm_util.h"
#include "opus_util.h"
#include "dynbuf_util.h"
#include "uarts.h"
#include "str_util.h"
#include "oggopus_util.h"
#include "pcm1753.h"
#include "pu.h"
#include "cm7core.h"
#include "opus_buffer.h"
#include "pd_data.h"
#include "pd.h"



//#define SENDOPUS_FILE 			f_global_file
//static FIL sendopus_file = {0};

#define SENDOPUS_OGG_DECODER	 global_ogg_decoder
//static ogg_decoder_t ogg_decoder = {0};

uint32_t sendopus_samplerate = 0;
uint8_t sendopus_channels = 1;

static uint32_t sendopus_period = 0, sendopus_tick = 0;
int sendopus_frame_size = 0;
BOOL sendopus_execute = FALSE;
static uint8_t sendopus_daddr = 0xFF;
static uint8_t sendopus_cmd = PD_CMD_OPUS48mono;

uint32_t sendopus_cnt = 0;

static uint32_t sendopus_task_it_cnt = 0;
static dynamic_buf_t SendOpusBuffer = DYNAMIC_BUF_STATIC_INIT(64 * 1024);
extern TIM_HandleTypeDef htim1;

void sendopus_stop(void)
{

    if (sendopus_execute) {
    	sendopus_execute = FALSE;
    	cm7core_close_file();
        decoder_opus_destroy();
        pcm1753_DataFinish();
        sendopus_cnt = 0;
        dynamic_buf_clear(&SendOpusBuffer);
    	ulog(CONM"sendopus: stop"CRLF);
    }


}

/*
uint64_t sendopus_calc_period(int frame_size, uint32_t samplerate, uint8_t channels)
{
	uint64_t period;

	period =  ((uint64_t)frame_size * (uint64_t)1000000 / (uint64_t)samplerate);
   	ulog_fmt(CONM"sendopus: (frame_size:%d, samplerate:%lu, channels:%d)->period:%s "CRLF, frame_size, samplerate, channels, ull2str(period));
	return period;
}

void sendopus_task_data(uint64_t diff)
{
	uint8_t segment[OPUS_MAX_ENCODE_SIZE];
	int16_t pcm[OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE];
	int opus_len;
	int pcm_len;
	//int pcm1753_proc;
	//uint64_t raz;



	if (pcm1753_AudioBufferGetFree() < OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE) {
		return;
	}

	fulog_fmt("df:%s"CRLF, ull2str(diff));

	opus_len = oggopus_read_segment(&SENDOPUS_OGG_DECODER, segment);
    if (!opus_len) {
    	sendopus_stop();
		return;
    }

	uarts_send_opus_data(segment, opus_len, sendopus_daddr, sendopus_samplerate, sendopus_channels == 2);
    //ulog_fmt("sendopus: send:%d, diff:%s cnt:%lu"CRLF, rc, ull2str(diff), sendopus_cnt);


    //uart8_uart5_transmit_opus(sendopus_daddr, setting.uart_local_addr, sendopus_cmd, segment, opus_len);

	sendopus_cnt++;



	pcm_len = decode_opus(segment, opus_len, pcm, OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE);
    if (pcm_len < 1) return;

    if (sendopus_frame_size) {

    	if (sendopus_frame_size != pcm_len) {
           	sendopus_frame_size = pcm_len;
           	sendopus_period = sendopus_calc_period(sendopus_frame_size, sendopus_samplerate, sendopus_channels);
    	}

    } else {
    	sendopus_frame_size = pcm_len;
       	sendopus_period = sendopus_calc_period(sendopus_frame_size, sendopus_samplerate, sendopus_channels);
    }





    //fulog_fmt("playopus: pcm:%d"CRLF, rc * playopus_channels);
    pcm1753_AudioBufferLwrbWrite(pcm, pcm_len * sendopus_channels);

   // if (diff > sendopus_period + 1000) {
   // 	sendopus_period = 5000;
   // }


    //opus_buffer_put(pcm, pcm_len * sendopus_channels);


	//if (pcm1753_is_DMA_started()) {
	//	pcm1753_proc = pcm1753_GetAudioBufferProcentFull();
	//	if (pcm1753_proc > 75) {
	//		if (sendopus_period > 100) sendopus_period -= 100;
	//	}
	//	else if (pcm1753_proc < 25) {
	//		sendopus_period += 100;
	//	}

	//}


}
*/




void sendopus_task_it_data(void)
{
	uint8_t segment[OPUS_MAX_ENCODE_SIZE];
	size_t data_len;

	if (!dynamic_buf_pop(&SendOpusBuffer, segment, sizeof(segment), &data_len)) return;
	if (data_len) {
		uart8_uart5_send_opus_data(segment, data_len, sendopus_daddr, sendopus_samplerate, sendopus_channels == 2);
	}

}


//1 СЂР°Р· РІ РјРёР»РёСЃРµРє
void sendopus_task_it(void)
{
	if (sendopus_task_it_cnt < sendopus_period - 1) {
		sendopus_task_it_cnt++;
		return;
	}

	sendopus_task_it_cnt = 0;
	sendopus_task_it_data();

}



void sendopus_task_data(void)
{
	uint8_t segment[OPUS_MAX_ENCODE_SIZE];
	int rc;
	int nb_samples;
	//uint32_t diff_tick;



	rc = oggopus_read_segment(&SENDOPUS_OGG_DECODER, segment);
    if (!rc) {
    	sendopus_stop();
		return;
    }

    //pu_put_opus_in_pcm1753_now(segment, rc);


    dynamic_buf_push(&SendOpusBuffer, segment, rc);

    //int nb_frames = opus_packet_get_nb_frames(segment, rc);
    nb_samples = opus_packet_get_nb_samples(segment, rc, sendopus_samplerate); //РІС‹РґР°РµС‚ 960

    sendopus_period =  nb_samples * 1000 / sendopus_samplerate;
    if (sendopus_period < 10) sendopus_period = 10;

    //fulog_fmt(CONM"nb_frames:%d nb_samples:%d"CRLF, nb_frames, nb_samples);
}


void sendopus_task(void)
{
	if (sendopus_execute) {
		//if (pcm1753_AudioBufferGetFree() > pcm1753_AudioBufferGetFull()) {
			//sendopus_task_data();
		//}
		if (dynamic_buf_get_free_size(&SendOpusBuffer) > dynamic_buf_get_data_size(&SendOpusBuffer)) {
			sendopus_task_data();
		}
	}

}


unsigned int sendopus_read_file(void * afile, void * data, unsigned int data_len)
{
	return cm7core_read_file(data, data_len);
}

BOOL sendopus_start(const char * file_name, uint8_t daddr)
{

	if (!cm7core_open_file(file_name)) {
		ulog_fmt(CONM"sendopus: start error open file: %s"CRLF, file_name);
		return FALSE;
	}

	ogg_decoder_init(&SENDOPUS_OGG_DECODER, NULL, sendopus_read_file);



    if (!oggopus_open(&SENDOPUS_OGG_DECODER, &sendopus_samplerate, &sendopus_channels)) {
		ulog_fmt(CONM"sendopus: start error open ogg: %s"CRLF, file_name);
		cm7core_close_file();
		return FALSE;
    }



    if (!pu_prepare_audio_play(sendopus_samplerate, sendopus_channels == 2)) {
		cm7core_close_file();
		return FALSE;
    }



	sendopus_period = 10;
	sendopus_tick = HAL_GetTickUs(&htim1);
	sendopus_frame_size = 0;
	sendopus_execute = TRUE;
    sendopus_daddr = daddr;
    sendopus_cnt = 0;
    sendopus_cmd = pd_get_opus_cmd_from_frequency_stereo(sendopus_samplerate, sendopus_channels == 2);
    //send_opus_tick = HAL_GetTick();
    dynamic_buf_clear(&SendOpusBuffer);


	ulog_fmt(CONM"sendopus: opus head read, frequency:%lu, channels:%d, daddr:%d"CRLF, (unsigned long)sendopus_samplerate, sendopus_channels, sendopus_daddr);


	return TRUE;

}


/*
void sendopus_task_execute(void)
{
	uint64_t tnew, diff;
	uint8_t segment[OPUS_MAX_ENCODE_SIZE];
	int opus_len;

	opus_len = oggopus_read_segment(&SENDOPUS_OGG_DECODER, segment);
    if (!opus_len) {
    	sendopus_stop();
		return;
    }



	if (pcm1753_AudioBufferGetFree() < OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE) {
		return;
	}

	if (pcm1753_is_DMA_started()) {
		sendopus_task_data(0);
		return;
	}


	tnew = HAL_GetTickUs(&htim1);
	diff = DIFF_TICKU64(sendopus_tick, tnew);

	if (diff > sendopus_period) {
		sendopus_tick = tnew;
		sendopus_task_data(diff);
	}

}
*/
