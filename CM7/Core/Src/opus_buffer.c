/*
 * opus_buffer.c
 *
 *  Created on: 21 июл. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "opus_buffer.h"
#include "ringbuff_util.h"
#include "buf_io.h"
#include "stm_util.h"
#include "oggopus_util.h"
#include "pu.h"
#include "pcm1753.h"
#include "opus_util.h"
#include "dynbuf_util.h"

//uint8_t opus_buffer_strategy = 0;


//буффер c PCM данными
static dynamic_buf_t OpusBuffer = DYNAMIC_BUF_STATIC_INIT(64 * 1024);

#define OPUS_BUF_CLEAR							dynamic_buf_clear(&OpusBuffer);
#define OPUS_BUF_FREE_SIZE						dynamic_buf_get_free_size(&OpusBuffer)
#define OPUS_BUF_PUT_PACKET(data, data_len)		dynamic_buf_push_io(&OpusBuffer, data, data_len)
#define OPUS_BUF_POP_PACKET(buf, buf_size)		dynamic_buf_pop_io(&OpusBuffer,  buf, buf_size)
#define OPUS_BUF_IS_EMPTY						DYNAMIC_BUF_IS_EMPTY(OpusBuffer)
#define OPUS_BUF_FULL_SIZE						dynamic_buf_get_data_size(&OpusBuffer)
#define OPUS_BUF_GET_MAX_SIZE					dynamic_buf_get_max_alloc_size(&OpusBuffer)


void opus_buffer_init(void)
{
}



void opus_buffer_clear(void)
{
	OPUS_BUF_CLEAR;
}




BOOL opus_buffer_put(uint8_t * opus, uint16_t opus_len)
{
	if (OPUS_BUF_PUT_PACKET(opus, opus_len) < opus_len) {
		fulog_fmt(CONM"pu: OpusBuffer FULL(%d)"CRLF, opus_len);
		return FALSE;
	}
	else return TRUE;
}

/*
BOOL opus_buffer_put(uint8_t * opus, uint16_t opus_len)
{
	int cnt;
	BOOL res;

	cnt = BUFIO_FREE_SIZE(&OpusBuffer);


	if (cnt < opus_len + 100) {
		fulog_fmt(CONM"pu: OpusBuffer FULL(%d,%d)"CRLF, opus_len, cnt);
		return FALSE;
	}

	cnt = BUFIO_PUT_PACKET(&OpusBuffer, opus, opus_len);

	res = (cnt == opus_len);

	if (!res) {
		fulog_fmt(CONM"pu: error OpusBuffer AFULL (writeln:%d opus_len:%d alloc:%lu max:%lu t:%lu)"CRLF, cnt, opus_len, BUFIO_FULL_SIZE(&OpusBuffer), BUFIO_GET_MAX_SIZE(&OpusBuffer), HAL_GetTick());
		return FALSE;
	}

	return res;
}

*/



void opus_buffer_task_data(void)
{
	uint8_t opus[OPUS_MAX_ENCODE_SIZE];
	size_t opus_len;

	opus_len = OPUS_BUF_POP_PACKET(opus, OPUS_MAX_ENCODE_SIZE);
	if (!opus_len) return;

	pu_put_opus_in_pcm1753_now(opus, opus_len);
}



void opus_buffer_task(void)
{

	if (OPUS_BUF_IS_EMPTY) return;


    if (pcm1753_is_DMA_started()) {

		if (pcm1753_AudioBufferGetFree() > OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE) { //если в pcm1753 свободного места хватает
			opus_buffer_task_data();
		}

    } else {

    	if (opus_buffer_procent_full() > 50) {
    		opus_buffer_task_data();
    	}

		//if (OPUS_BUF_FREE_SIZE < OPUS_MAX_ENCODE_SIZE) {
		//	opus_buffer_task_data();
		//}

    }


}

/*
void opus_buffer_task(void)
{

	if (BUFIO_IS_EMPTY(&OpusBuffer)) return;


	if (opus_buffer_strategy == 0) {
		if (BUFIO_FULL_SIZE(&OpusBuffer) > BUFIO_FREE_SIZE(&OpusBuffer)) { //OpusBuffer заполнен больше чем на половину
			opus_buffer_strategy = 1;
		}
	}
	else if (opus_buffer_strategy == 1) {

		//if (BUFIO_FULL_SIZE(&OpusBuffer) > BUFIO_FREE_SIZE(&OpusBuffer)) { //OpusBuffer заполнен больше чем на половину

			if (pcm1753_AudioBufferGetFree() > OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE) { //если в pcm1753 свободного места хватает
				opus_buffer_task_data();
			}

		//} else {

			//if (pcm1753_AudioBufferGetFree() > pcm1753_AudioBufferGetFull()) { //если в pcm1753 свободного места больше чем занятого
			//	pu_from_OpusBuffer_in_pcm1753_task_data();
			//}


		//}

	}
	else {
		opus_buffer_strategy = 0;
	}


}
*/

int opus_buffer_procent_full(void)
{
	size_t max = OPUS_BUF_GET_MAX_SIZE;

	return max ? (OPUS_BUF_FULL_SIZE * 100 / max) : 0;
}


/*
BOOL pu_put_opus_to_choose(uint8_t * opus, uint16_t opus_len)
{

	if ( (BUFIO_FULL_SIZE(&OpusBuffer) == 0) && (pcm1753_AudioBufferGetFree() > OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE) ) { //если в pcm1753 свободного места хватает
		return pu_put_opus_in_pcm1753_now(opus, opus_len);
	} else {
		return pu_put_opus_in_OpusBuffer(opus, opus_len);
	}
}
*/
