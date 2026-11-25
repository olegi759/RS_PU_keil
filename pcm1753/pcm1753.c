/*
 * pcm1753.c
 *
 *  Created on: 15 РЅРѕСЏР±. 2023 Рі.
 *      Author: Rafael Boltachev
 */
#include "pcm1753.h"
#include "fatfs_util.h"
#include "str_util.h"
#include "wav_util.h"
#include "stm_util.h"
#include "mux_sound.h"
#include "pcm_util.h"
#include "pcm1753.h"
#include "ringbuff_util.h"

//ALIGN_32BYTES
//Р±СѓС„С„РµСЂ РїРѕРґ DMA
static __attribute__((aligned(32))) int16_t SaiAudioBufferDma[2][SAI_DMA_BUFFER_SIZE];
//С‚РµРєСѓС‰РёР№ РёРЅРґРµРєСЃ Р±СѓС„С„РµСЂР° РїРѕРґ DMA
volatile static uint8_t SaiAudioBuffeIndex = 0xFF;

static uint8_t SaiAudioBufferData[64 * 1024];
//Р±СѓС„С„РµСЂ c PCM РґР°РЅРЅС‹РјРё
static ringbuff_t SaiAudioBuffer = RINGBUFF_STATIC_INIT(SaiAudioBufferData, sizeof(SaiAudioBufferData));

#define PCM1753_BUF_CLEAR						ringbuff_reset(&SaiAudioBuffer)
#define PCM1753_BUF_FREE_SIZE					ringbuff_get_free(&SaiAudioBuffer)
#define PCM1753_BUF_FULL_SIZE					ringbuff_get_full(&SaiAudioBuffer)
#define PCM1753_BUF_PUT(data, data_len)			ringbuff_write(&SaiAudioBuffer, data, data_len)
#define PCM1753_BUF_POP(buf, buf_size)			ringbuff_read(&SaiAudioBuffer,  buf, buf_size)
#define PCM1753_BUF_GET_MAX_SIZE				ringbuff_get_max_size(&SaiAudioBuffer)


//СЃС‚РІСЂС‚РѕРІР°Р»Рѕ Р»Рё DMA
BOOL sai_is_DMA_started = FALSE;
//С‚Р°Р№РјРµСЂ РґРѕР±Р°РІР»РµРЅРёСЏ РґР°РЅРЅС‹С… РІ SaiAudioBuffer
static  uint32_t pcm1753_AudioBufferLwrbWrite_tick = 0;
//РіСЂРѕРјРєРѕСЃС‚СЊ SAI
static uint16_t sai_volume = 100;
//РќРµРѕР±С…РѕРґРёРјРѕ Р·Р°РїРѕР»РЅРёС‚СЊ РґРјР° РЎРђР
static BOOL SaiDmaNeedFill[2]	=   { FALSE };
//Сѓ РЎРђР СЃРјРµРЅРёР»СЃСЏ РёРЅРґРµРєСЃ
static BOOL SaiChangeIndex = FALSE;
//РґР°РЅРЅС‹Рµ РґР»СЏ СЃР°Рё Р·Р°РєРѕРЅС‡РёР»РёСЃСЊ
static BOOL SaiDataFinish = FALSE;



void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
	ulog_fmt(CONM"pcm1753: SAI error: 0x%08lx\n", (unsigned long)hsai->ErrorCode);
}



//СѓСЃС‚Р°РЅРѕРІРєР° РІ 0 Р·РЅРІС‡РµРЅРёР№ РІ Р±СѓС„РµСЂРµ DMA
void SaiAudioBufferSetZero(uint8_t index, uint16_t start)
{
	uint16_t i;

	for (i = start; i < SAI_DMA_BUFFER_SIZE; i++) SaiAudioBufferDma[index][i] = 0;
}

//РЎРѕР±С‹С‚РёРµ РїРѕ РїРѕР»РѕРІРёРЅРµ DMA
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef * hsai)
{
	//if (SaiAudioBuffeIndex < 2) {
	//	if (HAL_DiffTick(pcm1753_AudioBufferLwrbWrite_tick) < 100)	ulog(CONM"pcm1753: SAI empty"CRLF);
	//}
	SaiAudioBuffeIndex = 0;
	SaiDmaNeedFill[0] = TRUE;
	SaiChangeIndex = TRUE;
	//ulog(" s0 ");
	//speed_test_tick++;
}

//РЎРѕР±С‹С‚РёРµ РїРѕ С†РµР»РѕР№ DMA
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef * hsai)
{
	//if (SaiAudioBuffeIndex < 2) {
		//if (HAL_DiffTick(pcm1753_AudioBufferLwrbWrite_tick) < 100) ulog(CONM"pcm1753: SAI empty"CRLF);
	//}
	SaiAudioBuffeIndex = 1;
	SaiDmaNeedFill[1] = TRUE;
	SaiChangeIndex = TRUE;
	//ulog(" s1 ");
	//speed_test_tick++;
}


//РёР· С‡Р°СЃС‚РѕС‚С‹ РІ С‡Р°СЃС‚РѕС‚Сѓ SAI
uint32_t get_sai_audio_frequency(uint32_t value)
{

	if (value < 10000) return SAI_AUDIO_FREQUENCY_8K;
	else if (value < 13500) return SAI_AUDIO_FREQUENCY_11K;
	else if (value < 19000) return SAI_AUDIO_FREQUENCY_16K;
	else if (value < 27000) return SAI_AUDIO_FREQUENCY_22K;
	else if (value < 38000) return SAI_AUDIO_FREQUENCY_32K;
	else if (value < 46000) return SAI_AUDIO_FREQUENCY_44K;
	else if (value < 50000) return SAI_AUDIO_FREQUENCY_48K;
	else if (value < 100000) return SAI_AUDIO_FREQUENCY_96K;
	else if (value < 200000) return SAI_AUDIO_FREQUENCY_192K;
	else return SAI_AUDIO_FREQUENCY_MCKDIV;
}



//СЃС‡РёС‚Р°РµС‚ РјРЅРѕР¶РёС‚РµР»Рё Рё РґРµР»РёС‚РµР»Рё SAI РЅР°РёР±РѕР»РµРµ РїРѕРґС…РѕРґСЏС‰РёРµ РґР»СЏ С‡Р°СЃС‚РѕС‚
void calc_sai_coeff(uint32_t input_freq, uint32_t output_freq, uint32_t * DIVN3, uint32_t * DIVP3)
{
	//DIVN3 - 13 ... 78
	//DIVP3 - 1 ... 128

	uint32_t n, p, val, max_diff, diff;

	max_diff = 0xFFFFFFFF;
	for (n = 13; n <= 78; n++) {
		for (p = 1; p <= 128; p++) {
			val = input_freq * n / p;
			if (val > output_freq) diff = val - output_freq;
			else diff =  output_freq - val;

			if  (diff < max_diff) {
				*DIVN3 = n;
				*DIVP3 = p;
				max_diff = diff;
			}

		}
	}

}

void pcm1753_SetPll(uint32_t DIVN3, uint32_t DIVP3)
{
	  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);

	  PeriphClkInitStruct.PLL3.PLL3N = DIVN3;
	  PeriphClkInitStruct.PLL3.PLL3P = DIVP3;


	  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);


	  ulog_fmt(CONM"pcm1753: set clock To SAI2.3 (DIVN3:%lu, DIVP3:%lu), %lu"CRLF, (unsigned long)DIVN3, (unsigned long)DIVP3, (unsigned long)(12288000 * DIVN3 / DIVP3));

}
//СѓСЃС‚Р°РЅР°РІР»РёРІР°РµС‚ РєРѕСЌС„С„РёС†РµРЅС‚С‹ РЅР° SAI
void pcm1753_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SaiAudioFreq, BOOL Stereo)
{
	  uint32_t DIVN3, DIVP3;

	  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);

	  //12288000 / 48000 = 256;
	  //12288000 / 44100 = 278.63945;
	  //12288000 / 32000 = 384;
	  //12288000 / 22050 = 557.27891;
	  //12288000 / 16000 = 768;
	  //12288000 / 11025 = 1114.55792;
	  //12288000 / 8000 = 1536;



	  DIVN3 = 13;
	  DIVP3 = 13;

	  calc_sai_coeff(12288000, SaiAudioFreq * 256 * (Stereo ? 2 : 1), &DIVN3, &DIVP3);

	  PeriphClkInitStruct.PLL3.PLL3N = DIVN3;
	  PeriphClkInitStruct.PLL3.PLL3P = DIVP3;

	  //ulog_fmt(CONM"pcm1753: set clock pll3 - DIVN3:%lu, DIVP3:%lu, ((12288000 * DIVN3 / DIVP3) / 256) = %lu"CRLF, DIVN3, DIVP3, (12288000 * DIVN3 / DIVP3) / 256);

	  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

//РЈСЃС‚Р°РЅРѕРІРєР° С‡Р°СЃС‚РѕС‚С‹ РЅР° SAI
void pcm1753_SetFrequency(uint32_t Frequency, BOOL Stereo)
{
	uint32_t SaiAudioFrequency;


	SaiAudioFrequency = get_sai_audio_frequency(Frequency) ;

  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
	//pcm1753_ClockConfig(&hsai_BlockB2, SaiAudioFrequency, Stereo);

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&hsai_BlockB2);

  /* Update the SAI audio frequency configuration */
  hsai_BlockB2.Init.AudioFrequency = SaiAudioFrequency;
  HAL_SAI_Init(&hsai_BlockB2);






  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&hsai_BlockB2);

  //ulog_fmt("pcm1753: set frequency %lu"CRLF, SaiAudioFrequency);


  //pcm1753_SetPll(13 * 6, 13);
}

uint32_t pcm1753_GetFrequency(void)
{
	return hsai_BlockB2.Init.AudioFrequency;
}


//РЈСЃС‚Р°РЅРѕРІРєР° РјРѕРЅРѕ/СЃС‚РµСЂРµРѕ РЅР° SAI
void pcm1753_SetStereo(BOOL value)
{
	  __HAL_SAI_DISABLE(&hsai_BlockB2);
	  hsai_BlockB2.Init.MonoStereoMode = value ? SAI_STEREOMODE : SAI_MONOMODE;
	  __HAL_SAI_ENABLE(&hsai_BlockB2);
	  //ulog_fmt("pcm1753: set MonoStereoMode %s"CRLF, value ? "stereo" : "mono");
}

BOOL pcm1753_GetStereo(void)
{
	  return hsai_BlockB2.Init.MonoStereoMode == SAI_STEREOMODE;
}


//СЃР±СЂРѕСЃ AudioBufferLwrb
void pcm1753_AudioBufferLwrbReset(void)
{
	PCM1753_BUF_CLEAR;
}


size_t pcm1753_AudioBufferGetFree(void)
{
	return PCM1753_BUF_FREE_SIZE;
}



size_t pcm1753_AudioBufferGetFull(void)
{
	return PCM1753_BUF_FULL_SIZE;
}


BOOL pcm1753_AudioBufferIsEmpty(void)
{
	return (PCM1753_BUF_FULL_SIZE == 0);
}

//РіСЂРѕРјРєРѕСЃС‚СЊ
void pcm1753_SetVolume(uint16_t volume)
{
	sai_volume = volume;
	//ulog_fmt("pcm1753: set Volume %d"CRLF, volume);
}

//void pcm1753_AudioBufferLwrbFreePop(void)
//{
//	int16_t buf[SAI_DMA_BUFFER_SIZE];
//	size_t data_len;

//	fulog("1"CRLF); //РїРµСЂРµРїРѕР»РЅРµРЅРёРµ Р±СѓС„РµСЂР°
//	dynamic_buf_pop(&SaiAudioBuffer, buf, sizeof(buf), &data_len);
//	fulog("2"CRLF); //РїРµСЂРµРїРѕР»РЅРµРЅРёРµ Р±СѓС„РµСЂР°
//}



//Р—Р°РїРёСЃСЊ РґР°РЅРЅС‹С… РІ AudioBufferLwrb
void pcm1753_AudioBufferLwrbWrite(int16_t *pcmdata, uint16_t pcmdata_len)
{
	//size_t rc;
	uint16_t pcmdata_len_in_bytes;

	if (!pcmdata_len) return;

	pcmdata_len_in_bytes = pcmdata_len * 2;

	//rc = pcm1753_AudioBufferGetFree();
	//if (rc < pcmdata_len_in_bytes) {
	//	fulog_fmt(CONM"pcm1753: SAI full(%d, %d)"CRLF, pcmdata_len, rc); //РїРµСЂРµРїРѕР»РЅРµРЅРёРµ Р±СѓС„РµСЂР°
	//	pcm1753_on_sai_full();
	//	return; //?
	//}


	if (sai_volume != 100) {
		pcm_set_volume(pcmdata, pcmdata, pcmdata_len, sai_volume);
	}

	//ulog("pcm1753: put"CRLF);

	//BUFIO_SET_BUSY(&SaiAudioBuffer, TRUE);
	if (PCM1753_BUF_PUT(pcmdata, pcmdata_len_in_bytes) != pcmdata_len_in_bytes) {
		fulog(CONM"pcm1753: SAI full"CRLF);
		//fulog_fmt(CONM"pcm1753: SAI dynamic_buf_push error (a:%lu ma:%lu)"CRLF, BUFIO_FULL_SIZE(&SaiAudioBuffer), BUFIO_GET_MAX_SIZE(&SaiAudioBuffer));
	}
	//BUFIO_SET_BUSY(&SaiAudioBuffer, FALSE);

	//ulog_fmt(CONM"ABW %d"CRLF, pcmdata_len * 2);

	//ulog_fmt("pcm1753: put %d(%lu)"CRLF, pcmdata_len *2, dynamic_buf_get_data_size(&SaiAudioBuffer));

	if (!pcm1753_is_DMA_started()) { //РµСЃР»Рё dma РЅРµ Р·Р°РїСѓС‰РµРЅРѕ
		if (pcm1753_AudioBufferGetFull() > pcm1753_AudioBufferGetFree()) { //Р·Р°РїСѓСЃРє DMA РµСЃР»Рё РґР°РЅРЅС‹С… РІ Р±СѓС„РµСЂРµ SaiAudioBuffer РґРѕСЃС‚Р°С‚РѕС‡РЅРѕ
			pcm1753_start_DMA();
		}
	}

	pcm1753_AudioBufferLwrbWrite_tick = HAL_GetTick();
}


//static uint8_t last_SaiAudioBuffeIndex = 0xFF;




//РїРµСЂРµРјРµСЃС‚РёС‚СЊ РґР°РЅРЅС‹Рµ РёР· SaiAudioBuffer РІ SaiAudioBuffer[0 РёР»Рё 1]
void pcm1753_SaiAudioBufferFill(void)
{
	size_t rc;
	//BOOL Timeout;

	//Timeout = (HAL_DiffTick(pcm1753_AudioBufferLwrbWrite_tick) > 500);

	if (HAL_DiffTick(pcm1753_AudioBufferLwrbWrite_tick) > 1000) { //РµСЃР»Рё РґР»РёС‚РµР»СЊРЅРѕРµ РІСЂРµРјСЏ РЅРµ РїРёСЃР°Р»Рё РІ SaiAudioBufferLwr

		if (pcm1753_AudioBufferIsEmpty()) {		//Р¶РґРµРј РѕСЃРІРѕР±РѕР¶РґРµРЅРёСЏ Р±СѓС„РµСЂР° SaiAudioBuffer
			pcm1753_stop_DMA();
			return;
		}
	}

	if (SaiAudioBuffeIndex > 1) return; //РЅРµС‚ С‚РµРєСѓС‰РµРіРѕ РёРЅРґРµРєСЃР° РЎРђР
	//if (BUFIO_IS_BUSY(&SaiAudioBuffer)) return; //РµСЃР»Рё Р·Р°РЅСЏС‚ Р±СѓС„РµСЂ


	rc = PCM1753_BUF_POP(SaiAudioBufferDma[SaiAudioBuffeIndex], sizeof(SaiAudioBufferDma[SaiAudioBuffeIndex]));

	if (rc < sizeof(SaiAudioBufferDma[SaiAudioBuffeIndex])) {

		if (rc) {
			SaiAudioBufferSetZero(SaiAudioBuffeIndex, rc / 2); //РµСЃР»Рё РІ Р±СѓС„РµСЂРµ SaiAudioBuffer РЅРµ С…РІР°С‚Р°РµС‚ РґР°РЅРЅС‹С… -> 0
			if (!SaiDataFinish) {
				fulog_fmt(CONM"pcm1753: SAI empty (need %lu bytes)"CRLF, (unsigned long)(sizeof(SaiAudioBufferDma[SaiAudioBuffeIndex]) - rc));
				pcm1753_on_sai_empty();
			}
		} else {
			if (SaiChangeIndex && SaiDmaNeedFill[!SaiAudioBuffeIndex]) { //РµСЃР»Рё СЃРјРµРЅРёР»СЃСЏ РёРЅРґРµРєСЃ Р° Р”РњРђ РїСЂРµРґС‹РґСѓС‰РµРіРѕ РЅРµ Р·Р°РїРѕР»РЅРµРЅРѕ
				SaiDmaNeedFill[!SaiAudioBuffeIndex] = FALSE;
				if (!SaiDataFinish) {
					fulog(CONM"pcm1753: SAI empty"CRLF);
					pcm1753_on_sai_empty();
				}
			}

			if (SaiChangeIndex) {
				SaiDmaNeedFill[!SaiAudioBuffeIndex] = FALSE;
			}
			SaiChangeIndex = FALSE;
			return;
		}
	}

	if (SaiChangeIndex) {
		SaiDmaNeedFill[!SaiAudioBuffeIndex] = FALSE;
	}
	SaiChangeIndex = FALSE;

	SaiAudioBuffeIndex = 0xFF;
	SaiDmaNeedFill[SaiAudioBuffeIndex] = FALSE; //Р”РњРђ Р·Р°РїРѕР»РЅРµРЅРѕ

}

//Р·Р°РґР°С‡Р° pcm1753
void pcm1753_task(void)
{

	if (sai_is_DMA_started) {
		pcm1753_SaiAudioBufferFill();
	}

}

//DMA СЃС‚Р°СЂС‚РѕРІР°Р»Р°?
BOOL pcm1753_is_DMA_started(void)
{
	return  sai_is_DMA_started;
}

//РћСЃС‚Р°РЅРѕРІРєР° DMA
void pcm1753_stop_DMA(void)
{
	if (sai_is_DMA_started) {
		HAL_SAI_DMAStop(&hsai_BlockB2);
		//set_mux_sound_left_right_enable(FALSE);
		set_mux_sound_right_left(SLR_OFF);
		sai_is_DMA_started = FALSE;
		SaiAudioBuffeIndex = 0xFF;
		//PIN_CLR(SP_ON);
		pcm1753_AudioBufferLwrbReset();
		ulog(CONM"pcm1753: stop DMA"CRLF);
		pcm1753_OnStopDMA();
	}
}


//Р—Р°РїСѓСЃРє DMA
void pcm1753_start_DMA(void)
{
	HAL_StatusTypeDef status;
	if (pcm1753_is_DMA_started()) pcm1753_stop_DMA();
	set_mux_sound_right_left(SLR_UM1_UM2);
	SaiAudioBuffeIndex = 0xFF;
	status = HAL_SAI_DMAStop(&hsai_BlockB2);
	//if (status != HAL_OK) {
	//	ulog_fmt("pcm1753: stop DMA error:%d"CRLF, status);
	//}


	//SCB_CleanDCache_by_Addr()
	//__DSB();
	//SCB_CleanDCache_by_Addr((uint32_t *)SaiAudioBufferDma, sizeof(SaiAudioBufferDma));

	//ulog(CONM"do HAL_SAI_Transmit_DMA"CRLF);
	status = HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t *)SaiAudioBufferDma, SAI_DMA_BUFFER_SIZE * 2);
	//ulog(CONM"posle HAL_SAI_Transmit_DMA"CRLF);

	if (status != HAL_OK) {
		//HAL_SAI_ERROR_TIMEOUT
		//HAL_SAI_ERROR_DMA | HAL_SAI_ERROR_TIMEOUT | HAL_SAI_ERROR_UDR
		ulog_fmt(CONM"pcm1753: start DMA error:%d(0x%08lx)"CRLF, status, (unsigned long)hsai_BlockB2.ErrorCode);
       // if (hsai_BlockB2.hdmarx != NULL)
            ulog_fmt(CONM" -> DMA RX error: 0x%08lx"CRLF, (unsigned long)hsai_BlockB2.hdmarx->ErrorCode);
       // if (hsai_BlockB2.hdmatx != NULL)
        	ulog_fmt(CONM" -> DMA TX error: 0x%08lx"CRLF, (unsigned long)hsai_BlockB2.hdmatx->ErrorCode);
		return;
	}

	pcm1753_AudioBufferLwrbWrite_tick = HAL_GetTick();
	SaiDmaNeedFill[0] = FALSE;
	SaiDmaNeedFill[1] = FALSE;
	SaiChangeIndex = FALSE;
	sai_is_DMA_started = TRUE;
	SaiDataFinish = FALSE;
	ulog_fmt(CONM"pcm1753: start DMA channels:%s frequency:%lu volume:%d "CRLF, (hsai_BlockB2.Init.MonoStereoMode == SAI_STEREOMODE ? "stereo": "mono"), (unsigned long)hsai_BlockB2.Init.AudioFrequency, sai_volume);
}


BOOL pcm1753_set_SaiAudioBufferDma(uint8_t ASaiAudioBuffeIndex, uint16_t num, int16_t value)
{
	if (ASaiAudioBuffeIndex > 1) return FALSE;
	if (num >= SAI_DMA_BUFFER_SIZE) return FALSE;

	SaiAudioBufferDma[ASaiAudioBuffeIndex][num] = value;
	return TRUE;
}


//void pcm1753_SetAudioBufferSize(size_t size)
//{
//}



uint32_t pcm1753_GetAudioBufferMaxSize(void)
{
	return PCM1753_BUF_GET_MAX_SIZE;
}


int pcm1753_GetAudioBufferProcentFull(void)
{
	return  pcm1753_AudioBufferGetFull() * 100 / pcm1753_GetAudioBufferMaxSize();
}

void pcm1753_DataFinish(void)
{
	SaiDataFinish = TRUE;
}

//РёРіРёС†РёР°Р»РёР·Р°С†РёСЏ
//void pcm1753_init(void)
//{
//	pcm1753_SetAudioBufferSize(64 * 1024);
//}



// ----------------------------------------------------------------------
//------------------------------------- OTHER ---------------------------
// ----------------------------------------------------------------------


/*


#define MAX_BUF_SIN_SZ 100
static int16_t buf_sin[MAX_BUF_SIN_SZ];


void pcm1753_play_sunus(uint32_t freq)
{
	uint16_t i;

	pcm1753_stop_DMA();

	if (!freq) {
		ulog("pcm1753: play sinus stop"CRLF);
		return;
	}


	pcm1753_SetFrequency(freq);

	for (i = 0; i < MAX_BUF_SIN_SZ; ++i) buf_sin[i] = sin(2.0 * M_PI * i / MAX_BUF_SIN_SZ) * 32767;

	fulog_fmt("pcm1753: play sinus freq:%lu"CRLF, freq);


	HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t *)buf_sin,  MAX_BUF_SIN_SZ);
	set_mux_sound_left_right_enable(TRUE);
	sai_is_DMA_started = TRUE;
}




	// РџСЂРµСЂС‹РІР°РµРј С‚РµРєСѓС‰СѓСЋ РїРµСЂРµРґР°С‡Сѓ
	status = HAL_SAI_Abort(&hsai_BlockB2);
	if (status != HAL_OK) {
	    // РћР±СЂР°Р±РѕС‚РєР° РѕС€РёР±РєРё
	}

	// Р–РґС‘Рј РіРѕС‚РѕРІРЅРѕСЃС‚Рё SAI
	while (hsai_BlockB2.State != HAL_SAI_STATE_READY) {
	    HAL_Delay(1);
	}


	//SAI_FillFifo(&hsai_BlockB2);

	 //HAL_SAI_Transmit(&hsai_BlockB2, (uint8_t *)&status, 1, 100);

	//SCB_CleanDCache_by_Addr()
	//SCB_InvalidateDCache_by_Addr()


	//ulog(CONM"do SCB_CleanDCache_by_Addr"CRLF);
	//SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)SaiAudioBufferDma) & ~(uint32_t)0x1F), sizeof(SaiAudioBufferDma) + 32);
	//ulog(CONM"posle SCB_CleanDCache_by_Addr"CRLF);

	//ulog(CONM"do HAL_SAI_Transmit_DMA"CRLF);
	//status = HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t *)SaiAudioBufferDma, SAI_DMA_BUFFER_SIZE * 2);
	//ulog(CONM"posle HAL_SAI_Transmit_DMA"CRLF);


*/
