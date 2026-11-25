/*
 * mchs.c
 *
 *  Created on: 17 СЏРЅРІ. 2025 Рі.
 *      Author: Rafael Boltachev
 */

#include "mchs_mic.h"

#include "mux_sound.h"
#include "stm_util.h"
#include "adcplay_util.h"
#include "pcm_util.h"
#include "pcm1753.h"
#include "pu.h"
//#include "mki.h"
#include "uarts.h"
#include "opus_util.h"
#include "cm7core.h"
#include "mki_data.h"
#include "common.h"
#include "math_util.h"

/*
РћС‚ РїРѕСЂС‚Р° РњР§РЎ РµСЃС‚СЊ СЃРёРіРЅР°Р» IN_MCHS РЅР° РїРѕСЂС‚
PD4
Р­С‚Рѕ СЃРёРіРЅР°Р» Р·Р°РїСѓСЃРєР°, РїРѕСЃР»Рµ РєРѕС‚РѕСЂРѕРіРѕ РЅР°С‡РёРЅР°РµС‚СЃСЏ С‚СЂР°РЅСЃР»СЏС†РёСЏ РѕРїРѕРІРµС‰РµРЅРёСЏ РѕС‚ РњР§РЎ
Р·Р°РґРµСЂР¶РєР° СЃРёРіРЅР°Р»Р° РјРµР¶РґСѓ РїРѕСЏРІР»РµРЅРёСЏ СЃРёРіРЅР°Р»Р° Рё РЅР°С‡Р°Р»Р° С‚СЂР°РЅСЃР»СЏС†РёРё РїРѕСЂСЏРґРєР° 300РјСЃ - 500 РјСЃ
РџРѕРєР° СЃРёРіРЅР°Р» IN_MCHS Р°РєС‚РёРІРµРЅ РёРґРµС‚ С‚СЂР°РЅСЃР»СЏС†РёСЏ, РєР°Рє РѕРЅ РІС‹РєР»СЋС‡Р°РµС‚СЃСЏ - С‚СЂР°РЅСЃР»СЏС†РёСЏ РІС‹РєР»СЋС‡Р°РµС‚СЃСЏ.
РџРѕ РјРёРєСЂРѕС„РѕРЅСѓ. РќР° РјРѕРґСѓР»Рµ РєР»Р°РІРёР°С‚СѓСЂС‹ Рё РёРЅРґРёРєР°С†РёРё РµСЃС‚СЊ РєРЅРѕРїРєР° РІРєР»СЋС‡РµРЅРёСЏ РјРёРєСЂРѕС„РѕРЅР°.
СЃРѕРјРЅРµРІР°СЋСЃСЊ, С‡С‚Рѕ РњР§РЎ Рё РјРёРєСЂРѕС„РѕРЅ РѕРґРЅРѕРІСЂРµРјРµРЅРЅРѕ Р±СѓРґСѓС‚ СЂР°Р±РѕС‚Р°С‚СЊ. РЈ РІС…РѕРґР° РњР§РЎ (РЅР°СЃРєРѕР»СЊРєРѕ СЏ РїРѕРјРЅСЋ) СЃР°РјС‹Р№ РІС‹СЃРѕРєРёР№ РїСЂРёРѕСЂРёС‚РµС‚. РЎСЂРµРґРё РІС…РѕРґРѕРІ СЃРёРіРЅР°Р»РёР·Р°С†РёРё.
Р•С‰Рµ РїРѕ РїРѕСЂС‚Сѓ РњР§РЎ. РџСЂРё РїРѕР»СѓС‡РµРЅРёРё СЃРёРіРЅР°Р»Р° IN_MCHS РІ РѕС‚РІРµС‚ РЅСѓР¶РЅРѕ РІС‹СЃС‚Р°РІРёС‚СЊ СЃРёРіРЅР°Р» OUT_MCHS РїРѕСЂС‚ PD3. Р­С‚Рѕ РїРѕРґС‚РІРµСЂР¶РґРµРЅРёРµ, С‡С‚Рѕ СЃРёРіРЅР°Р» Р·Р°РїСѓСЃРєР° РїСЂРёРЅСЏС‚. РћС‚РєР»СЋС‡Р°РµС‚СЃСЏ РѕРЅ РїРѕСЃР»Рµ С‚РѕРіРѕ РєР°Рє РїСЂРёРЅСЏС‚Рѕ РѕС‚РєР»СЋС‡РµРЅРёРµ СЃРёРіРЅР°Р»Р° IN_MCHS Рё РІС‹РєР»СЋС‡РµРЅР° С‚СЂР°РЅСЃР»СЏС†РёСЏ.
Р•С‰Рµ РїРѕ РІРєР»СЋС‡РµРЅРёСЋ.
РџСЂРё РїРѕР»СѓС‡РµРЅРёРё IN_MCHS:
1. РІРєР»СЋС‡Р°РµРј РєР°РЅР°Р»С‹ РЅР° MUX_SOUND_1 (MUX_SOUND_2) СЌС‚Рѕ РІС‹С…РѕРґС‹ РЅР° РЈРњ. Р›РёР±Рѕ РЅР° РѕРґРёРЅ РЈРњ Р»РёР±Рѕ РЅР° РґРІР°.
2. РћРў РјРѕРґСѓР»СЏ Р¦Рџ РїРµСЂРµРґР°РµС‚ РєРѕРјР°РЅРґСѓ РЅР° РјРѕРґСѓР»СЊ РєРѕРјРјСѓС‚Р°С†РёРё Рё РєРѕРЅС‚СЂРѕР»СЏ Р»РёРЅРёРё - РІРєР»СЋС‡РёС‚СЊ Р·РѕРЅСѓ С‚СЂР°РЅСЃР»СЏС†РёРё РёР»Рё Р·РѕРЅС‹. РљР°РєРёРµ Р·РѕРЅС‹ РЅСѓР¶РЅС‹ СЃРјРѕС‚СЂРёРј РїРѕ С‚Р°Р±Р»РёС†Рµ РЈСЂРѕРІРЅРµР№ РѕРїРѕРІРµС‰РµРЅРёСЏ.
3. РџРѕС‚Рѕ СѓР¶Рµ РЅР°С‡РёРЅР°РµРј РїРµСЂРµРґР°С‡Сѓ РѕС‚ РђР¦Рџ Рє Р¦РђРџ
С‚.Рµ. СЃРЅР°С‡Р°Р»Р° РІСЃРµ РєРѕРјРјСѓС‚РёСЂСѓРµРј, Р° РїРѕС‚РѕРј СѓР¶Рµ РІРєР»СЋС‡Р°РµРј Р·РІСѓРє РѕС‚ РњР§РЎ
*/

//#define MCHS_MIC_AUDIO_FREQUENCY_TIM_16K	16830
//#define MCHS_MIC_AUDIO_FREQUENCY_TIM_22K	22748
//#define MCHS_MIC_AUDIO_FREQUENCY_TIM_32K	33669
//#define MCHS_MIC_AUDIO_FREQUENCY_TIM_48K	50621




static uint8_t in_mchs_old = 0; //СЃРѕРѕСЃС‚РѕСЏРЅРёРµ IN_MCHS
static uint8_t in_mchs_now = 0; //СЃРѕРѕСЃС‚РѕСЏРЅРёРµ IN_MCHS

static uint16_t mchs_mic_dma_buf[2][MCHS_MIC_DMA_BUF_SIZE];
static uint8_t mchs_mic_dma_buf_index = 0xFF;


static BOOL mchs_mic_dma_started = FALSE;
static BOOL mchs_mic_is_mic = FALSE;
static uint8_t mchs_mic_daddr = 0xFF;


void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
	ulog_fmt(CONM"HAL_ADC_GetError=0x%08lx"CRLF, (unsigned long)HAL_ADC_GetError(hadc));

//#define HAL_ADC_ERROR_INTERNAL          (0x01U)   /*!< ADC peripheral internal error (problem of clocking, enable/disable, erroneous state, ...)       */
//#define HAL_ADC_ERROR_OVR               (0x02U)   /*!< Overrun error                               */
//#define HAL_ADC_ERROR_DMA               (0x04U)   /*!< DMA transfer error
}



// ----------------- РџСЂРµСЂС‹РІР°РЅРёСЏ СЃ ADC --------------------------
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
	mchs_mic_dma_buf_index = 0;

	//ulog("h ");

	//pcm_adc12mono2pcm(mchs_mic_dma_buf[0], pcm, MCHS_MIC_DMA_BUF_SIZE);
	//pcm1753_AudioBufferLwrbWrite(pcm, MCHS_MIC_DMA_BUF_SIZE);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	mchs_mic_dma_buf_index = 1;

	//ulog("c ");

	//fulog_fmt("c:%lu"CRLF, HAL_GetTick());
	//pcm_adc12mono2pcm(mchs_mic_dma_buf[1], pcm, MCHS_MIC_DMA_BUF_SIZE);
	//pcm1753_AudioBufferLwrbWrite(pcm, MCHS_MIC_DMA_BUF_SIZE);
}

//Р—Р°РїСѓС‰РµРЅРѕ Р»Рё DMA РњР§РЎ РёР»Рё РјРёРєСЂРѕС„РѕРЅР°
BOOL mchs_mic_is_dma_started(void)
{
	return mchs_mic_dma_started;
}

extern DMA_HandleTypeDef hdma_adc1;

//РЎС‚Р°СЂС‚ С‚СЂР°РЅСЃР»СЏС†РёРё РњР§РЎ РёР»Рё РјРёРєСЂРѕС„РѕРЅР°
void mchs_mic_start(BOOL is_mic, uint8_t daddr)
{
	  ADC_ChannelConfTypeDef sConfig = {0};
	  HAL_StatusTypeDef hs;
	  uint16_t volume;


  	  //ulog(CONM"mchs_mic:  mchs_mic_start"CRLF);

	  mchs_mic_daddr = daddr;
	  mchs_mic_is_mic = is_mic;

	  mchs_mic_stop();


	  if (is_mic) {
		  	cm7core_send_one_led_color(MKI_LED_MICROPHONE, LED_COLOR_GREEN);
	  }
	  else {
		  PIN_SET(OUT_MCHS);
	  }

	  //set_mux_sound_left_right_enable(mki_get_led(MKI_LED_MUTE) == LED_COLOR_BLACK);
	  //set_mux_sound_left_right_enable(TRUE);
	  set_mux_sound_right_left(SLR_UM1_UM2);

	  pu_prepare_pcm1753(MCHS_MIC_AUDIO_FREQUENCY, FALSE);
	  //pcm1753_SetAudioBufferSize(16 * 1024);



	  if (is_mic) volume = setting.mic_volume;
	  else volume = setting.mchs_volume;
	  pcm1753_SetVolume(volume);



	  sConfig.Channel = (is_mic ? ADC_CHANNEL_17 : ADC_CHANNEL_14);
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	  sConfig.SingleDiff = ADC_SINGLE_ENDED;
	  sConfig.OffsetNumber = ADC_OFFSET_NONE;
	  sConfig.Offset = 0;
	  sConfig.OffsetSignedSaturation = DISABLE;
	  HAL_ADC_ConfigChannel(&hadc1, &sConfig);




	  set_tim_frequency2(&htim15, MCHS_MIC_AUDIO_FREQUENCY, CM7_BASE_FREQUENCY / 2);
	  hs = HAL_TIM_Base_Start(&htim15);
	  if (hs !=  HAL_OK) {
	  	  ulog_fmt(CONM"mchs_mic:  start tim fault %d"CRLF, hs);
	  	  return;
	  }


	  HAL_ADC_Stop_DMA(&hadc1);
	  mchs_mic_dma_buf_index = 0xFF;




	  hs = HAL_ADC_Start_DMA(&hadc1, (uint32_t *)mchs_mic_dma_buf, MCHS_MIC_DMA_BUF_SIZE * 2);
	  mchs_mic_dma_started = (hs == HAL_OK);
	  if (hs !=  HAL_OK) {
	  	  ulog_fmt("mchs_mic:  start dma fault %d"CRLF, hs);
	  	  return;
	  }

  	  ulog_fmt("mchs_mic: start dma OK, volume:%d"CRLF, volume);


	  if ((pu_mode == PU_MODE_VESHANIE_MCHS) || (pu_mode == PU_MODE_UPRAVLENIE_DRUGIMI_PU) ) {
		  decoder_opus_destroy();
		  if (encoder_opus_create(MCHS_MIC_AUDIO_FREQUENCY, 1) != OPUS_OK) {
			  ulog("mchs_mic:  fault encoder opus create"CRLF);
		  }
	 }


}

//РџРѕРґСЃС‚СЂРѕР№РєР° С‡Р°СЃС‚РѕС‚С‹  -
void mchs_mic_on_sai_full(void)
{
	uint32_t freq;

	if (!mchs_mic_dma_started) return;

	//if (htim15.State == HAL_TIM_STATE_READY ) return;

	freq = get_tim_frequency2(&htim15,  CM7_BASE_FREQUENCY);
	set_tim_frequency2(&htim15, freq - 10, CM7_BASE_FREQUENCY);
	ulog_fmt("freq - : %d"CRLF, freq - 10);
}

//РџРѕРґСЃС‚СЂРѕР№РєР° С‡Р°СЃС‚РѕС‚С‹ +
void mchs_mic_on_sai_empty(void)
{
	uint32_t freq;

	if (!mchs_mic_dma_started) return;

	//if (htim15.State == HAL_TIM_STATE_READY ) return;

	freq = get_tim_frequency2(&htim15,  CM7_BASE_FREQUENCY);
	set_tim_frequency2(&htim15, freq + 10, CM7_BASE_FREQUENCY);
	ulog_fmt("freq + : %d"CRLF, freq + 10);
}

//РћСЃС‚Р°РЅРѕРІ С‚СЂР°РЅСЃР»СЏС†РёРё РњР§РЎ Рё РјРёРєСЂРѕС„РѕРЅР°
void mchs_mic_stop(void)
{
	if (mchs_mic_dma_started) {
		ulog("mchs_mic: stop dma"CRLF);
		HAL_ADC_Stop_DMA(&hadc1);
		mchs_mic_dma_buf_index = 0xFF;
		//set_mux_sound_left_right_enable(FALSE);
		set_mux_sound_right_left(SLR_OFF);
		PIN_CLR(OUT_MCHS);
		HAL_TIM_Base_Stop(&htim15);
		pcm1753_AudioBufferLwrbReset();
		mchs_mic_dma_started = FALSE;
		cm7core_send_one_led_color(MKI_LED_MICROPHONE, LED_COLOR_BLACK);
		HAL_Delay(50);
	}
	encoder_opus_destroy();
}


//РљРѕРЅС‚СЂРѕР»СЊ Р·Р° СЃРёРіРЅР°Р»РѕРј РњР§РЎ
void mchs_mic_task_in_mchs(void)
{
	in_mchs_now = !PIN_GET(IN_MCHS);

	if (!in_mchs_old && in_mchs_now) { //СЃРёРіРЅР°Р» РњР§РЎ СѓСЃС‚Р°РЅРѕРІР»РµРЅ
		pu_set_mode(PU_MODE_VESHANIE_MCHS);
	}
	else if (in_mchs_old && !in_mchs_now) { //СЃРёРіРЅР°Р» РњР§РЎ СЃРЅСЏС‚
		pu_set_mode(PU_MODE_DEJURNIY);
	} else {

		if (in_mchs_now) { //СЃРёРіРЅР°Р» РњР§РЎ СѓСЃС‚Р°РЅРѕРІР»РµРЅ

			if (pu_mode < PU_MODE_VESHANIE_MCHS) {
				pu_set_mode(PU_MODE_VESHANIE_MCHS);
			}
		} else { //СЃРёРіРЅР°Р» РњР§РЎ СЃРЅСЏС‚

			if (pu_mode == PU_MODE_VESHANIE_MCHS) {
				pu_set_mode(PU_MODE_DEJURNIY);
			}

		}

	}

	in_mchs_old = in_mchs_now;
}


unsigned short mic_min = 0;
unsigned short mic_max = 0;

//Р—Р°РіСЂСѓР·РєР° DMA РњР§РЎ Рё РјРёРєСЂРѕС„РѕРЅР°
void mchs_mic_task_data(void)
{
	int16_t pcm[MCHS_MIC_DMA_BUF_SIZE];
	opus_int32 rc;
	uint8_t opus_data[MCHS_MIC_DMA_BUF_SIZE];

	pcm_adc12mono2pcm(mchs_mic_dma_buf[mchs_mic_dma_buf_index], pcm, MCHS_MIC_DMA_BUF_SIZE);

	ushort_array_get_min_max_value(mchs_mic_dma_buf[mchs_mic_dma_buf_index],  MCHS_MIC_DMA_BUF_SIZE, &mic_min, &mic_max);

	pcm1753_AudioBufferLwrbWrite(pcm, MCHS_MIC_DMA_BUF_SIZE);


	if ((pu_mode == PU_MODE_VESHANIE_MCHS) || (pu_mode == PU_MODE_UPRAVLENIE_DRUGIMI_PU) ) {
		if (/*!mchs_mic_is_mic  &&*/ get_encoder_opus()) {
			rc = encode_opus(pcm, MCHS_MIC_DMA_BUF_SIZE, opus_data, sizeof(opus_data));
			if (rc > 0) {
				//fulog_fmt(CONM"mchs_mic opus send %d"CRLF, rc);
				uart8_uart5_send_opus_data(opus_data, rc,  mchs_mic_daddr, MCHS_MIC_AUDIO_FREQUENCY, FALSE);
			}
		}
	}

	//ulog_fmt("a %lu", pcm1753_AudioBufferGetFull());
}

//РџСЂРѕС†РµСЃСЃ Р·Р°РіСЂСѓР·РєРё DMA РњР§РЎ Рё РјРёРєСЂРѕС„РѕРЅР°
void mchs_mic_task(void)
{
	if (mchs_mic_dma_started) {
		if (mchs_mic_dma_buf_index < 2) {
			mchs_mic_task_data();
			mchs_mic_dma_buf_index = 0xFF;
		}
	}

}

void mchs_mic_init(void)
{
	in_mchs_old = PIN_GET(IN_MCHS);
	mchs_mic_dma_buf_index = 0xFF;
	mchs_mic_dma_started = FALSE;
}


/*
#if defined( __ICCARM__ )
  #define DMA_BUFFER \
      _Pragma("location=\".dma_buffer\"")
#else
  #define DMA_BUFFER \
      __attribute__((section(".dma_buffer")))
#endif

//static __attribute__((aligned(32))) uint16_t mchs_mic_dma_buf[2][MCHS_MIC_DMA_BUF_SIZE];

//uint16_t mchs_mic_dma_buf[2][MCHS_MIC_DMA_BUF_SIZE]

//static __attribute__((section(".bss"))) __attribute__((aligned(32))) uint16_t mchs_mic_dma_buf[2][MCHS_MIC_DMA_BUF_SIZE];

	  //HAL_TIM_Base_Start_IT(&htim15);

	  //hs = HAL_DMA_Start_IT(&hdma_adc1, *(uint32_t *)mchs_mic_dma_buf, hadc1.Instance->DR, MCHS_MIC_DMA_BUF_SIZE * 2);
  	  //ulog_fmt("HAL_DMA_Start_IT %d"CRLF, hs);

	  //SCB_InvalidateDCache_by_Addr(mchs_mic_dma_buf, sizeof(mchs_mic_dma_buf));
	  //SCB_CleanDCache_by_Addr(~(uint32_t)0x1F(uint32_t *)mchs_mic_dma_buf)& ~(uint32_t)0x1F, sizeof(mchs_mic_dma_buf) + 32);

	  //SCB_CleanDCache_by_Addr((uint32_t*)(((uint32_t)mchs_mic_dma_buf) & ~(uint32_t)0x1F), sizeof(mchs_mic_dma_buf) + 32);

//#include "stm32h7xx_hal_adc.h"

	  //HAL_ADC_RegisterCallback(&hadc1,  HAL_ADC_CONVERSION_COMPLETE_CB_ID , HAL_ADC_ConvCpltCallback);
	  //HAL_ADC_RegisterCallback(&hadc1,   HAL_ADC_CONVERSION_HALF_CB_ID , HAL_ADC_ConvHalfCpltCallback);



	  //hdma_adc1

	  //ADC_DMAHalfConvCplt(&hdma_adc1);

	  //__HAL_RCC_TIM15_CLK_ENABLE();
	  //__HAL_RCC_DMA1_CLK_ENABLE();
	  //__HAL_RCC_DMA2_CLK_ENABLE();

	  //HAL_ADC_GetState()
*/
