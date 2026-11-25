/*
 * opus_util.c
 *
 *  Created on: 9 янв. 2024 г.
 *      Author: Rafael Boltachev
 */

#include "opus_util.h"
#include "opus.h"

OpusEncoder *enc = NULL;
OpusDecoder *dec = NULL;

//�������  8000, 12000, 16000, 24000, 48000
//opus_int32 sampling_rate =  48000;

//���-�� �������  1 ��� 2
//int channels = 1;

//OPUS_APPLICATION_AUDIO, OPUS_APPLICATION_VOIP, OPUS_APPLICATION_RESTRICTED_LOWDELAY
const int application = OPUS_APPLICATION_AUDIO;

//< 1000
const opus_int32 bitrate_bps = 0;

//OPUS_AUTO, OPUS_BANDWIDTH_NARROWBAND, OPUS_BANDWIDTH_MEDIUMBAND, OPUS_BANDWIDTH_WIDEBAND, OPUS_BANDWIDTH_SUPERWIDEBAND, OPUS_BANDWIDTH_FULLBAND
const int bandwidth = OPUS_BANDWIDTH_FULLBAND; //OPUS_AUTO;

//use_vbr == 0 - > static encode size
const int use_vbr = 1;

const int cvbr = 0;

//int  complexity = 10;
const int  complexity = 0;

const int use_inbandfec = 0;

const int  forcechannels = OPUS_AUTO;

const int use_dtx = 0;

const int packet_loss_perc = 0;

const opus_int32 skip = 0;

const int variable_duration = OPUS_FRAMESIZE_ARG;


opus_int32 calc_opus_sampling_rate(int sampling_rate)
{
  if (sampling_rate < 10000) return 8000;
  else if (sampling_rate < 14000) return 12000;
  else if (sampling_rate < 20000) return 16000;
  else if (sampling_rate < 36000) return 24000;
  else return 48000;
}

OpusEncoder * get_encoder_opus(void)
{
	return enc;
}


//#include "stm_util.h"
int encoder_opus_create(opus_int32 sampling_rate, int channels)
{
     int err;


     if (enc) opus_encoder_destroy(enc);

     enc = opus_encoder_create(sampling_rate, channels, application, &err);
     if (err != OPUS_OK) {
       enc = NULL;
       return err;
     }
     //ulog("10"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate_bps));
     //ulog("20"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(bandwidth));
     //ulog("30"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_VBR(use_vbr));
     //ulog("40"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(cvbr));
     //ulog("50"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity));
     //ulog("60"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(use_inbandfec));
     //ulog("70"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(forcechannels));
     //ulog("80"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_DTX(use_dtx));
     //ulog("90"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(packet_loss_perc));
     //ulog("100"CRLF);

     //opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));
     //ulog("110"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));
     //ulog("120"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(variable_duration));
     //ulog("130"CRLF);
     opus_encoder_ctl(enc, OPUS_SET_FORCE_MODE(MODE_CELT_ONLY));
     //ulog("140"CRLF);

     return OPUS_OK;
}



void encoder_opus_destroy(void)
{
    if (enc) opus_encoder_destroy(enc);
    enc = NULL;
}


void encoder_opus_set_bitrate(int abitrate)
{
     opus_encoder_ctl(enc, OPUS_SET_BITRATE(abitrate));
}

void encoder_opus_set_bandwidth(int abandwidth)
{
     opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(abandwidth));
}



OpusDecoder * get_decoder_opus(void)
{
	return dec;
}


void decoder_opus_destroy(void)
{
    if (dec) opus_decoder_destroy(dec);
	dec = NULL;
}


int decoder_opus_create(opus_int32 sampling_rate, int channels)
{
    int err;

    if (dec) opus_decoder_destroy(dec);

    dec = opus_decoder_create(sampling_rate, channels, &err);
    if (err != OPUS_OK) {
      dec = NULL;
    }

    //opus_decoder_ctl(dec, OPUS_SET_FORCE_MODE(MODE_CELT_ONLY));

    return err;
}


opus_int32 encode_opus(const opus_int16 *pcm, int frame_size, unsigned char *data, opus_int32 max_data_bytes)
{
  return opus_encode(enc, pcm, frame_size, data, max_data_bytes);
}

int decode_opus(const unsigned char *data, opus_int32 len, opus_int16 *pcm, int frame_size)
{
    return opus_decode(dec, data, len, pcm, frame_size, 0);
}


int opus2pcm16(uint8_t * opus, opus_int32 opus_len, opus_int16 * pcm16_buf, int pcm16_buf_len, opus_int32 sampling_rate, int channels)
{
	int pcm16_len;

	if (!get_decoder_opus()) {
		if (!decoder_opus_create(sampling_rate, channels)) return 0;
	}

	pcm16_len = decode_opus(opus, opus_len, pcm16_buf, pcm16_buf_len);

	return pcm16_len * channels;
}
