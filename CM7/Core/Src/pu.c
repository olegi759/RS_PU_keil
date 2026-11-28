/*
 * pu.c
 *
 *  Created on: 26 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "pu.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "mki_data.h"
#include "screens.h"
//#include "mki.h"
//#include "mkk.h"
//#include "setting.h"
#include "stm_util.h"
#include "mchs_mic.h"
#include "net_util.h"
#include "str_util.h"
//#include "external_flash.h"
#include "fatfs_util.h"
#include "playopus.h"
#include "mux_sound.h"
#include "pcm1753.h"
#include "logfile_util.h"
//#include "rtc.h"
#include "opus_util.h"
#include "math_util.h"
//#include "um.h"
//#include "mkk.h"
#include "screen0.h"
#include "timer_util.h"
#include "pd.h"
//#include "fault.h"
#include "pd_data.h"
#include "uarts.h"
#include "screen5.h"
#include "pd_data.h"
#include "screen6.h"
#include "sendopus.h"
#include "net_util.h"
#include "cm7core.h"
#include "pcm1753.h"
#include "um_data.h"
#include "shared_memory.h"
#include "mkk_data.h"
#include "display.h"
#include "cm7core.h"
#include "beep.h"
#include "buf_io.h"
#include "common.h"
#include "pu.h"
#include "screen7.h"
#include "playmp3.h"
#include "opus_buffer.h"
#include "playmem.h"
#include "mp3files.h"


uint32_t pu_test_enable_tick = 0;

//Текущий уровень оповещения для режима Пожар
uint8_t pu_uo_fire = 1;
//Текущий уровень оповещения для режима Ручной пуск
//uint8_t pu_uo_manual = 1;
//Запуск транляции в режиме Ручной пуск
//BOOL pu_mode_manual_is_start_translation = FALSE;
////пожар на линиях
//uint32_t fire_lines = 0;
//Имитация пожара на линиях
uint32_t fire_lines_imit = 0;
//Отключение детектирования пожара на линиях
uint32_t fire_lines_insig = 0;
uint32_t fire_lines_manual = 0;   /* manual (panel) fire activation, not a blocking source */


//Опция автоматика линии
uint32_t pu_option_avtomatika_otkluchena_lines = 0;
//Опция отключение линии
uint32_t pu_option_otkluchenie_lines = 0;

uint32_t pu_ruchnoi_pusk_lines = 0;
//uint32_t pu_upravlenie_drugimi_pu_lines = 0;

//выборр групп или линий
BOOL pu_select_group = FALSE;
//Оставшееся время
uint8_t uo_fire_time_remaining = 0;
uint8_t uo_manual_time_remaining = 0;



static uint8_t pu_start_translation_pojar_repeat = 0;
/* set while pu_stop_translation() runs: the pcm1753 stop callback must not restart the track */
BOOL pu_translation_stopping = FALSE;
uint8_t 	pu_last_active_fire_uo = 1;


uint8_t pu_start_translation_manual_repeat = 0;

//необходимые линии для включения
uint32_t pu_need_lines = 0;



uint8_t faults_small = 0;


uint32_t pu_small_infos_tick[26] = {0};
pu_small_info_t pu_small_infos[26] = {0};

uint8_t pu_netinfo[3] = {0};
extern int sendopus_frame_size ;

static char tilibom_file_name[64] = { '\0' };

void tilibom_play(char * file_name)
{
	strcpy_s(tilibom_file_name, sizeof(tilibom_file_name), file_name);
	playmem_start((void *)tilibom, sizeof(tilibom));
}

uint16_t pu_get_data_from_small_infos(uint8_t * data)
{
	int i;
	uint16_t j;

	j = 0;
	for (i = 1; i < 25; i++) {
		if (!pu_small_infos_tick[i]) continue;
		if (HAL_DiffTick(pu_small_infos_tick[i]) > 90000) {
			pu_small_infos_tick[i] = 0;
			continue;
		}
		memcpy(&data[j], &pu_small_infos[i], sizeof(pu_small_info_t));
		j += sizeof(pu_small_info_t);
	}
	return j;
}


void pu_get_local_small_info(pu_small_info_t  * small_info)
{
	small_info->addr = setting.uart_local_addr;
	small_info->mode = pu_mode;
	small_info->lines_count = setting.lines_count;
	small_info->lines = pu_get_fire_lines();
}

void pu_fill_local_small_infos(void)
{
	uint8_t index;

	index = setting.uart_local_addr / 10;
	pu_get_local_small_info(&pu_small_infos[index]);
	pu_small_infos_tick[index] = HAL_GetTickNoZero();
}

BOOL pu_play_audio_file(const char * filename)
{

	if (is_file_name_mp3(filename)) return playmp3_start(filename);
	return playopus_start(filename);
}

//Неверный код доступа
void pu_invalid_access_code(char * ulog_msg, uint8_t need_level)
{
	char buf[MAX_STR_SIZE];
	uint8_t level_now;


	level_now = safety_get_level_from_pin_code();

	ulog(ulog_msg);

	//if (pu_mode != PU_MODE_DEJURNIY) {
	//	pu_set_mode(PU_MODE_DEJURNIY);
	//}

	snprintf(buf, sizeof(buf), "Неверный уровень доступа-%d(необходим уровень %d и больше)", level_now, need_level);

	screen7_init(buf, 0);
	screens_goto(7);

	SMALL_BEEP;
}

//сработавшие входов линий пожарной сигнализации
uint32_t pu_get_fire_lines(void)
{
	/* Inputs are combined with logical OR: MKK fire inputs (fire_lines_insig) and
	   the duplicated inputs arriving through fire_lines_imit. Bits 0..23 = inputs 1..24. */
	return (uint32_t)((fire_lines_insig | fire_lines_imit | fire_lines_manual) & 0x00FFFFFFUL);
}



/* fire inputs that BLOCK leaving the fire mode: MKK inputs and the duplicated fire_lines_imit bits.
   The manual (panel) activation is not a blocking source. */
uint32_t pu_get_fire_lines_hw(void)
{
	return (uint32_t)((fire_lines_insig | fire_lines_imit) & 0x00FFFFFFUL);
}


uint8_t pu_get_fire_lines_cnt(void)
{
	uint32_t fire_lines;

	fire_lines = pu_get_fire_lines();
	return bitis1cnt(&fire_lines, 24);
}

//Количество включенных линий
uint8_t pu_enable_lines_cnt(void)
{
	return bitis1cnt(&pu_enable_lines, 24);
}


//Из pu_soundsrc_t в стринг
const char * soundsrc2str(pu_soundsrc_t soundsrc)
{
	switch (soundsrc) {
		case PU_SOUNDSRC_MICROPHONE: return "МИКРОФОН";
		case PU_SOUNDSRC_FILE: return "ПЛЕЕР ПУ";
		case PU_SOUNDSRC_LINEINIY_VHOD: return "ЛИН.ВХОД";
		default: return "НЕТ";
	}
}


uint32_t last_frequency = 16000;

//подготовить pcm1753 к воспроизведению звука
void pu_prepare_pcm1753(uint32_t frequency, BOOL stereo)
{
	last_frequency = frequency;

	pcm1753_stop_DMA();

	opus_buffer_clear();
	pcm1753_AudioBufferLwrbReset();
	//pcm1753_SetAudioBufferSize(32 * 1024);
	pcm1753_SetStereo(stereo);
	pcm1753_SetFrequency(frequency, stereo);
	pcm1753_SetVolume(setting.sai_volume);
	recv_opus_cnt = 0;
}


//подготовить decoder_opus к декодироывнию звука
BOOL pu_prepare_decoder_opus(uint32_t frequency, BOOL stereo)
{
	uint32_t opus_freq;

	opus_freq = calc_opus_sampling_rate(frequency);
	if (decoder_opus_create(opus_freq, stereo ? 2: 1) == OPUS_OK) {
		return TRUE;
	} else {
		ulog(CONM"pu: error create opus decoder"CRLF);
		return FALSE;
	}
}

//подготовить audio к воспроизведению звука
BOOL pu_prepare_audio_play(uint32_t frequency, BOOL stereo)
{
	pu_prepare_pcm1753(frequency, stereo);
	return pu_prepare_decoder_opus(frequency, stereo);
}


//const short um_snd_test_const_20_2047[20] =  { 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047 };
const short um_snd_test_const_20_2047[20] =  { 0, 6320, 12030, 16560, 19460, 20470, 19460, 16560, 12030, 6320, 0, -6320, -12030, -16560, -19460, -20470, -19460, -16560, -12030, -6320 };


void pu_um_snd_sinus(void)
{

    pu_prepare_pcm1753(SAI_AUDIO_FREQUENCY_44K, FALSE);

	set_mux_sound_right_left(SLR_UM1_UM2);

	while (pcm1753_AudioBufferGetFree() >= 2 * 1024) {
		pcm1753_AudioBufferLwrbWrite((int16_t *)um_snd_test_const_20_2047, 20);
	}

	cm7core_send_to_CM4_cmd_only(CORE_CMD_UM_TEST);

	//um_transmit_piv(0xFF, setting.uart_local_addr , UM_CMD_TEST_LINE, NULL, 0);
}



BOOL pu_put_opus_in_pcm1753_now(uint8_t * opus, uint16_t opus_len)
{
	int16_t pcm16[OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE];
	int rc;
	BOOL res;


	//fulog_fmt("100 %d "CRLF, opus_len);

	if (!get_decoder_opus()) {
		if (!pu_prepare_decoder_opus(pcm1753_GetFrequency(), pcm1753_GetStereo())) 	return FALSE;
	}

	//fulog_fmt("o:%d->", opus_len);


	rc = decode_opus(opus, opus_len, pcm16, OPUS_MAX_ENCODE_PCM_SAMPLES_SIZE);


	//fulog_fmt("p:%d"CRLF, rc);

	res = (rc > 0);
    if (res) {
	
		sendopus_frame_size = rc;
		if (pcm1753_GetStereo()) rc += rc;
		pcm1753_AudioBufferLwrbWrite(pcm16, rc);
	}

	//fulog_fmt("400 %d "CRLF, rc);

	//if (rc < 980)
		//fulog_fmt("%d->%d(%lu)"CRLF, opus_len, rc, HAL_GetTick());

	return res;
}




const uint8_t * fire_lines_insig_ptr = (uint8_t *)&fire_lines_insig;
const uint8_t * fire_lines_imit_ptr = (uint8_t *)&fire_lines_imit;
const uint8_t * pu_enable_lines_ptr = (uint8_t *)&pu_enable_lines;
const uint8_t * pu_option_otkluchenie_lines_ptr = (uint8_t *)&pu_option_otkluchenie_lines;
const uint8_t * pu_option_avtomatika_otkluchena_lines_ptr = (uint8_t *)&pu_option_avtomatika_otkluchena_lines;

void pu_get_info(uint8_t info[PU_INFO_SIZE])
{

	info[0] = pu_mode;

	info[1] = pd_translation_addr;


	info[2] = faults_small;

	if (fire_lines_imit) {
		info[3] = fire_lines_imit_ptr[0];
		info[4] = fire_lines_imit_ptr[1];
		info[5] = fire_lines_imit_ptr[2];
	} else {
		info[3] = fire_lines_insig_ptr[0];
		info[4] = fire_lines_insig_ptr[1];
		info[5] = fire_lines_insig_ptr[2];
	}

	info[6] = pu_enable_lines_ptr[0];
	info[7] = pu_enable_lines_ptr[1];
	info[8] = pu_enable_lines_ptr[2];


	info[9] = pu_option_otkluchenie_lines_ptr[0];
	info[10] = pu_option_otkluchenie_lines_ptr[1];
	info[11] = pu_option_otkluchenie_lines_ptr[2];


	info[12] = pu_option_avtomatika_otkluchena_lines_ptr[0];
	info[13] = pu_option_avtomatika_otkluchena_lines_ptr[1];
	info[14] = pu_option_avtomatika_otkluchena_lines_ptr[2];

	info[15] = date_time[0];
	info[16] = date_time[1];
	info[17] = date_time[2];
	info[18] = date_time[3];
	info[19] = date_time[4];

	switch (pu_mode) {
		case PU_MODE_VESHANIE_POJAR:
			info[20] = pu_uo_fire;
			info[21] = uo_fire_time_remaining;
		break;
		case PU_MODE_RUCHNOI_PUSK:
			info[20] = 0; //pu_uo_manual;
			info[21] = 0; //uo_manual_time_remaining;
		break;
		default:
			info[20] = 0;
			info[21] = 0;
	}
	info[22] = setting.lines_count;

	info[23] = pu_netinfo[0];
	info[24] = pu_netinfo[1];
	info[25] = pu_netinfo[2];
	info[26] = 1; //исполнение

}


//Из горящих светодиодов 1..24 получить линии (РУЧНОЙ ПУСК)
uint32_t pu_get_lines_from_leds(void)
{
	return  pu_ruchnoi_pusk_lines;
}

//Из  горящих светодиодов (РУЧНОЙ ПУСК)
uint32_t pu_get_group_lines_from_leds(void)
{
	uint32_t lines;
	int i;

	lines = 0;
	for (i = 0; i < 24; i++) {
		if (GETBIT(pu_ruchnoi_pusk_lines, i)) {
			lines |= setting.groups[i + 1];
		}
	}
	return lines;
}


uint32_t pu_get_manual_lines_from_leds(void)
{
	return pu_select_group ? pu_get_group_lines_from_leds() : pu_get_lines_from_leds();
}


//Из количество горящих светодиодов (РУЧНОЙ ПУСК)
uint8_t pu_get_lines_cnt_from_leds(void)
{
	return bitis1cnt(&pu_ruchnoi_pusk_lines, 24);
}


//Зажечь светодиоды цветом color из линий
void pu_paint_led_lines(uint32_t lines, uint8_t color)
{
	uint8_t buf[5];

	memcpy(buf, &lines, sizeof(lines));
	buf[4] = color;

	cm7core_send_to_CM4_cmd(CORE_CMD_MKI_PAINT_LED_FROM_LINES, buf, sizeof(buf));

}


//Установка транслирующих линий на звука МКК
BOOL pu_enable_sound_lines(uint32_t lines)
{
	//BOOL wait;
	//uint16_t cmd_data_len;

	pu_need_lines = lines;

	if (pu_option_avtomatika_otkluchena_lines) lines = (lines & (~pu_option_avtomatika_otkluchena_lines)) & 0x00FFFFFF;

	cm7core_send_to_CM4_cmd(CORE_CMD_SET_ENABLE_LINES, (uint8_t * )&lines, sizeof(lines));




	//wait = cm7core_wait_cmd_from_CM4(CORE_CMD_SET_ENABLE_LINES_RESULT, (uint8_t *)&pu_enable_lines, sizeof(pu_enable_lines), 10000, &cmd_data_len);
	//return wait ? TRUE : FALSE;
	return TRUE;
}



// ------------------------- ПОЖАР ---------------------------------


uint32_t get_pojar_enable_lines(uint32_t fire_lines, uint8_t uo_fire)
{
	uint32_t lines;
	int i;


	lines = 0;
	for (i = 0; i < 24; i++) {
		if (GETBIT(fire_lines, i)) {
				lines |= setting.mode_fire[uo_fire].input_lines[i + 1];
		}
	}
	return lines;
}


void pu_start_translation_pojar_after_delay(void)
{
	uint32_t pojar_enable_lines;


	if (uo_fire_time_remaining) {
		uo_fire_time_remaining--;
		screen0_update();
	}
	else {
		timers_delete(pu_start_translation_pojar_after_delay);
		pojar_enable_lines = get_pojar_enable_lines(pu_get_fire_lines(), pu_uo_fire);
		pu_enable_sound_lines(pojar_enable_lines);
		ulog_fmt(CONM"pu: start translation pojar uo:%d file:%s"CRLF, pu_uo_fire, setting.mode_fire[pu_uo_fire].file);

		if (setting.mode_fire[pu_uo_fire].tilibom) {
			tilibom_play(setting.mode_fire[pu_uo_fire].file);
		} else {
			pu_play_audio_file(setting.mode_fire[pu_uo_fire].file);
		}
	}
}


//запустить трансляцию с текущим УО
BOOL pu_start_translation_pojar_now_uo(void)
{
	uint32_t pojar_enable_lines; //подключаемые линии

	//ulog_fmt("pu: start translation pojar uo:%d fire_lines:%s"CRLF, pu_uo_fire, ul2bitstr(fire_lines));

	if (pu_mode != PU_MODE_VESHANIE_POJAR) return FALSE;



	if (!pu_get_fire_lines()) {
		ulog_fmt(CONM"pu: start translation pojar uo:%d fault fire_lines=0"CRLF, pu_uo_fire);
		pu_set_mode(PU_MODE_DEJURNIY);
		return FALSE;
	}

	if (!setting.mode_fire[pu_uo_fire].enable) return FALSE;



	if (!strlen(setting.mode_fire[pu_uo_fire].file) || !cm7core_file_exist(setting.mode_fire[pu_uo_fire].file)) {
		ulog_fmt("pu: start translation pojar uo:%d fault src file invalid"CRLF, pu_uo_fire);
		return FALSE;

	}


	pojar_enable_lines = get_pojar_enable_lines(pu_get_fire_lines(), pu_uo_fire);

	if (!pojar_enable_lines) {
		ulog_fmt(CONM"pu: start translation pojar uo:%d fault enable_lines=0"CRLF, pu_uo_fire);
		return FALSE;
	}


	ulog_fmt(CONM"pu: delay start translation pojar uo:%d file:%s enable_lines:%s"CRLF, pu_uo_fire, setting.mode_fire[pu_uo_fire].file, u24l2bitstr(pojar_enable_lines));


	uo_fire_time_remaining = setting.mode_fire[pu_uo_fire].delay;
	if (uo_fire_time_remaining)	{
		timers_add(pu_start_translation_pojar_after_delay, 1000);
	}
	else pu_start_translation_pojar_after_delay();

	screen0_update();

	return TRUE;
}

//запустить трансляцию с следующим УО
BOOL pu_start_translation_pojar_next_uo(void)
{
	uint8_t cnt;

	cnt = 9;
	while (cnt) {
          pu_uo_fire++;
          if (pu_uo_fire > 10) { //прошли все 10 УО

        	  if (setting.fire_cycle) {
        		  pu_uo_fire = pu_last_active_fire_uo; //Сработавший УО
        	  }
        	  else pu_uo_fire = 1;

          }
      	  if (pu_start_translation_pojar_now_uo())  {
      		  pu_last_active_fire_uo = pu_uo_fire; //Сработавший УО
      		  return TRUE;
      	  }
      	  cnt--;
	}

	return FALSE;
}


// ------------------------- РУЧНОЙ ---------------------------------

/*
void pu_start_translation_manual_after_delay(void)
{
	if (uo_manual_time_remaining) {
		uo_manual_time_remaining--;
		if (screens_now() == 0) screen0_paint();
	}
	else {
		timers_delete(pu_start_translation_manual_after_delay);
		ulog_fmt(CONM"pu: start translation manual uo:%d file:%s"CRLF, pu_uo_manual, setting.mode_manual[pu_uo_manual].file);
		pu_play_audio_file(setting.mode_manual[pu_uo_manual].file);
	}

}

//запустить трансляцию с текущим УО
BOOL pu_start_translation_manual_now_uo(void)
{
	uint32_t lines;
	//uint8_t src;

	ulog_fmt(CONM"pu: start translation manual uo:%d"CRLF, pu_uo_manual);


	if (pu_mode != PU_MODE_RUCHNOI_PUSK) return FALSE;

	if (!setting.mode_manual[pu_uo_manual].enable) return FALSE;
	//if (setting.mode_manual[pu_uo_manual].soundsrc == PU_SOUNDSRC_NONE) return FALSE;



	lines = pu_get_lines_from_leds();

	if (pu_select_group) {
		lines = pu_get_group_lines_from_leds();
	} else  {
		lines = pu_get_lines_from_leds();
	}

	if (!lines) {
		ulog_fmt(CONM"pu: start translation manual uo:%d fault lines=0"CRLF, pu_uo_manual);
		pu_set_mode(PU_MODE_DEJURNIY);
		return FALSE;
	}


	//src = get_file_src(setting.mode_manual[pu_uo_manual].file);

	//if (!src) {
	//	ulog_fmt("pu: start translation manual uo:%d fault src file invalid"CRLF, pu_uo_manual);
	//	return FALSE;
	//}

	pu_enable_sound_lines(lines);



	screens_update();


	ulog_fmt(CONM"pu: delay start translation manual uo:%d file:%s"CRLF, pu_uo_manual, setting.mode_manual[pu_uo_manual].file);


	uo_manual_time_remaining = setting.mode_manual[pu_uo_manual].delay;
	if (uo_manual_time_remaining) {
		if (screens_now() == 0) screen0_paint();
		timers_add(pu_start_translation_manual_after_delay, 1000);
	}
	else pu_start_translation_manual_after_delay();


	return TRUE;
}


static uint8_t pu_last_active_manual_uo = 1;

//запустить трансляцию с следующим УО
BOOL pu_start_translation_manual_next_uo(void)
{
	uint8_t cnt;

	cnt = 9;
	while (cnt--) {
          pu_uo_manual++;
          if (pu_uo_manual > 10) {
        	  if (setting.manual_cycle) {
        		  pu_uo_manual = pu_last_active_manual_uo;
        	  }
        	  else pu_uo_manual = 1;
          }
      	  if (pu_start_translation_manual_now_uo())	{
      		  pu_last_active_manual_uo = pu_uo_manual;
      		  return TRUE;
      	  }
	}

	return FALSE;
}

*/
//static char pu_add_log_fmt_buf[MAX_STR_SIZE] = {0};







//раз в 1 сек
void pu_task(void)
{
	/* Periodic (1 Hz) polling of the fire inputs: MKK inputs and the duplicated
	   inputs from fire_lines_imit; any active bit -> VESHANIE_POJAR. */
	if ((pu_mode < PU_MODE_VESHANIE_POJAR) && pu_get_fire_lines()) {
		ulog_fmt(CONM"pu: periodic fire check: lines:%s, goto POJAR"CRLF, u24l2bitstr(pu_get_fire_lines()));
		pu_set_mode(PU_MODE_VESHANIE_POJAR);
		return;
	}


	switch (pu_mode) {

		case PU_MODE_DEJURNIY:
			//if ((setting.uart_local_addr == 20) && (HAL_DiffTick(uarts_transmit_it_tick) > 100) ){
			//	uarts_send_ring();
			//}
			//else uarts_ring_init();
			//uarts_send_ring();
		break;

		case PU_MODE_VESHANIE_MCHS:
		case PU_MODE_VESHANIE_POJAR:
			//pu_send_change_mode();
		break;

		default: break;
	}

}

BOOL mode_rucnoi_pusk_to_dejurniy_blocked = FALSE;

//Окончание проигрывания трека
void pcm1753_OnStopDMA(void)
{
	recv_opus_cnt = 0;

	/* if the DMA was stopped by pu_stop_translation() (mode change / reset) - do not restart the track */
	if (pu_translation_stopping) {
		set_mux_sound_right_left(SLR_OFF);
		pu_enable_sound_lines(0);
		return;
	}


	if (tilibom_file_name[0] != '\0') {

		if (strequals(tilibom_file_name, "mic")) {
			mchs_mic_start(TRUE, 0xFF);
		} else {
			pu_play_audio_file(tilibom_file_name);
		}

		tilibom_file_name[0] = '\0';

		return;
	}

	switch (pu_mode) {

		case PU_MODE_DEJURNIY:
			set_mux_sound_right_left(SLR_OFF);
			pu_enable_sound_lines(0);
		break;
		case PU_MODE_ZVUKOVOE_VESHANIE:
			pu_set_mode(PU_MODE_DEJURNIY);
		break;
		case PU_MODE_VESHANIE_PD:
		break;
		case PU_MODE_VESHANIE_POJAR:


			if (pu_start_translation_pojar_repeat) { //кол-во повторов уменьшаем
				pu_start_translation_pojar_repeat--;
			}

			if (pu_start_translation_pojar_repeat) { //еще есть повторы?
				pu_play_audio_file(setting.mode_fire[pu_uo_fire].file);
				ulog_fmt(CONM"pu: pu_start_translation_pojar_now_uo() repeat:%d"CRLF, pu_start_translation_pojar_repeat);
				break;
			}


			//т.к. повторов нет едем на следующее УО
			if (pu_start_translation_pojar_next_uo()) {
				pu_start_translation_pojar_repeat = setting.mode_fire[pu_uo_fire].repeat; //запоминаем кол-во повторов
			} else {
				ulog(CONM"pu: pu_start_translation_pojar_next_uo (no first) FALSE, goto DEJURNIY"CRLF);
				pu_set_mode(PU_MODE_DEJURNIY);
			}

		break;
		case PU_MODE_VESHANIE_MCHS:
		break;
		case PU_MODE_RUCHNOI_PUSK_PD:
		break;
		case PU_MODE_RUCHNOI_PUSK:

			if (mode_rucnoi_pusk_to_dejurniy_blocked){

			} else {
				set_mux_sound_right_left(SLR_OFF);
				pu_enable_sound_lines(0);
				pu_set_mode(PU_MODE_DEJURNIY);
			}
			mode_rucnoi_pusk_to_dejurniy_blocked = FALSE;

			//if (pu_mode_manual_is_start_translation) {

			//	if (pu_start_translation_manual_repeat) {
			//		pu_start_translation_manual_repeat--;
			//		//pu_start_translation_manual_now_uo();
			//		pu_play_audio_file(setting.mode_manual[pu_uo_manual].file);
			//		ulog_fmt(CONM"pu: pu_start_translation_manual_now_uo repeat:%d"CRLF, pu_start_translation_manual_repeat);
			//		break;
			//	}

			//	if (pu_start_translation_manual_next_uo()) {
			//		pu_start_translation_manual_repeat = setting.mode_manual[pu_uo_manual].repeat;
			//	} else {
			//		ulog(CONM"pu: pu_start_translation_manual_next_uo (no first) FALSE, goto DEJURNIY"CRLF);
			//		pu_set_mode(PU_MODE_DEJURNIY);
			//	}

			//}

		break;
		case PU_MODE_UPRAVLENIE_DRUGIMI_PU:
			pu_set_mode(PU_MODE_DEJURNIY);
		break;
		default:
		break;
	}



}


void pu_dma_stop(void)
{
	HAL_ADC_Stop_DMA(&hadc1);
	HAL_SAI_DMAStop(&hsai_BlockB2);
	pcm1753_AudioBufferLwrbReset();
}


//const uint8_t screen6_pu_addrs[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240};

//Останов трансляции
void pu_stop_translation(void)
{
	pu_translation_stopping = TRUE;

	if (playopus_is_execute() || pcm1753_is_DMA_started() || mchs_mic_is_dma_started()) {
		ulog(CONM"pu: stop translation"CRLF);
	}
	pcm1753_stop_DMA();

	pu_dma_stop();

    pu_enable_sound_lines(0);
	mchs_mic_stop();
	playopus_stop();
	sendopus_stop();




	//pu_mode_manual_is_start_translation = FALSE;
	//pu_uo_fire = 1;
	//pu_uo_manual = 1;


	//set_mux_sound_left_mux(0);
	//set_mux_sound_right_mux(1);
	//set_mux_sound_left_right_enable(FALSE);
	set_mux_sound_right_left(SLR_OFF);

	decoder_opus_destroy();
	pu_translation_stopping = FALSE;
}

/*

///Длбавить в лог строку с форматированием
void pu_add_log_fmt(const char * fmt, ... )
{
	va_list arg;
	char buf[MAX_STR_SIZE];

	snprintf(buf, sizeof(buf), "20%d-%d-%d %d:%d", date_time[0], date_time[1], date_time[2], date_time[3], date_time[4]);

	strcat_s(buf, sizeof(buf), " Режим: ");
	strcat_s(buf, sizeof(buf), pu_mode2str(pu_mode));
	strcat_s(buf, sizeof(buf), ";");


	va_start(arg, fmt);

	vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, arg);

	va_end(arg);

	strcat_s(buf, sizeof(buf), CRLF);

	buf[sizeof(buf) - 1] = 0;

	cm7core_add_log(buf);
}
*/

//Установить режим
BOOL pu_set_mode(pu_mode_t new_pu_mode)
{

	if (new_pu_mode < PU_MODE_DEJURNIY) new_pu_mode = PU_MODE_DEJURNIY;

	if (new_pu_mode == pu_mode) return FALSE;

	/* VESHANIE_POJAR can be left only when no fire input is active:
	   fire_lines_insig (MKK) and fire_lines_imit (duplicates) are equal sources (OR). */
	if ((pu_mode == PU_MODE_VESHANIE_POJAR) && (new_pu_mode != PU_MODE_VESHANIE_POJAR) && pu_get_fire_lines_hw()) {
		ulog_fmt(CONM"pu: leave POJAR blocked, fire lines active:%s"CRLF, u24l2bitstr(pu_get_fire_lines_hw()));
		SMALL_BEEP;
		return FALSE;
	}

	if (new_pu_mode < PU_MODE_VESHANIE_MCHS) {

	}


	if (pu_mode == PU_MODE_UPRAVLENIE_DRUGIMI_PU) {
		for (int i = 1; i <= 25; i++) {
			if (!screen6_lines[i]) continue;
			uarts_transmit_dejurniy(i * 10);
			HAL_Delay(50);
		}

	}

	if (new_pu_mode  != PU_MODE_DEJURNIY) {
		if (pu_mode > new_pu_mode) {
			ulog_fmt("pu: abort change mode %s->%s by priority"CRLF, pu_mode2str(pu_mode), pu_mode2str(new_pu_mode));
			SMALL_BEEP;
			return FALSE; //приоритеты если текущий режим меньше пр приоритету чем новый режим
		}
	}


	timers_delete(pu_start_translation_pojar_after_delay);
	//timers_delete(pu_start_translation_manual_after_delay);

	pu_stop_translation();


	if (pu_mode != new_pu_mode) {
		ulog_fmt(CONM"pu: change mode from %s -> %s"CRLF, pu_mode2str(pu_mode), pu_mode2str(new_pu_mode));
	}


	pu_mode = new_pu_mode;

	cm7core_send_to_CM4_cmd(CORE_CMD_CHANGE_MODE, (uint8_t *)&pu_mode, 1);



	cm7core_set_led_color(MKI_LED_MANUALSTART,  pu_mode == PU_MODE_RUCHNOI_PUSK ? pu_mode2ledcolor(PU_MODE_RUCHNOI_PUSK) : LED_COLOR_BLACK);
	cm7core_set_led_color(MKI_LED_FIRE, pu_mode == PU_MODE_VESHANIE_POJAR ? pu_mode2ledcolor(PU_MODE_VESHANIE_POJAR) : LED_COLOR_BLACK);
	cm7core_send_leds();


	uarts_transmit_change_mode();


	//uarts_transmit_veshanie_pd(4, 0xFF, 16000, FALSE, 1);
	//uarts_transmit_veshanie_pd(7, 0xFF, 16000, FALSE, 1);


	switch (pu_mode) {

		case PU_MODE_DEJURNIY:
			set_mux_sound_right_left(SLR_OFF);
			/* fire_lines_imit = 0; -- duplicated fire inputs are an independent source */
			/* fire_lines_insig = 0; -- MKK inputs are refreshed by mkk_change_insig_lin() */
			fire_lines_manual = 0;   /* manual activation is cancelled on reset */
			pu_select_group = FALSE;
			//filelog_fmt("");
			screens_goto(0);
			//pu_last_active_manual_uo = 1;
			pu_last_active_fire_uo = 1;
			//um_snd_modes_all(CH_MODE_STANDBY, CH_MODE_STANDBY);
			uo_manual_time_remaining = 0;
			uo_fire_time_remaining = 0;
			uarts_ring_init();
			pd_translation_addr = 0;
			pu_ruchnoi_pusk_lines = 0;
			//pu_upravlenie_drugimi_pu_lines = 0;
			tilibom_file_name[0] = '\0';
		break;
		case PU_MODE_VESHANIE_PD:
			set_mux_sound_right_left(SLR_UM1_UM2);
			//um_snd_modes_all(CH_MODE_TRANSLATION, CH_MODE_TRANSLATION);
			screens_goto(0);

			filelog_fmt("ID польз.:%lu", pd_id);
		break;
		case PU_MODE_ZVUKOVOE_VESHANIE:
			//um_snd_modes_all(CH_MODE_TRANSLATION, CH_MODE_TRANSLATION);

			switch (setting.mode_sound.soundsrc) {
				case PU_SOUNDSRC_MICROPHONE:
					set_mux_sound_right_left(SLR_UM1_UM2);
					if (setting.mode_sound.tilibom) {
						tilibom_play("mic");
					} else {
//						mchs_mic_start(TRUE, 0xFF);
					}
				break;
				case PU_SOUNDSRC_FILE:
					set_mux_sound_right_left(SLR_UM1_UM2);

					if (setting.mode_sound.tilibom) {
						tilibom_play(setting.mode_sound.file);
					} else {

						if (!pu_play_audio_file(setting.mode_sound.file)) {
							ulog(CONM"pu: start translation ZVUKOVOE_VESHANIE FALSE, goto DEJURNIY"CRLF);
							pu_set_mode(PU_MODE_DEJURNIY);
							SMALL_BEEP;
							break;
						}
					}

				break;
				case PU_SOUNDSRC_LINEINIY_VHOD:
					//set_mux_sound_left_mux(3);
					//set_mux_sound_right_mux(3);
					//set_mux_sound_left_right_enable(mki_get_led(MKI_LED_MUTE) == LED_COLOR_BLACK);
					set_mux_sound_right_left(SLR_LIN);
				break;
				default:
					ulog(CONM"pu: start translation ZVUKOVOE_VESHANIE FALSE, goto DEJURNIY"CRLF);
					pu_set_mode(PU_MODE_DEJURNIY);
					SMALL_BEEP;
				break;
			}
			pu_enable_sound_lines(setting.mode_sound.sound_lines);


			screens_goto(0);

			filelog_fmt("Линии опов.: %s", pu_lines2str(setting.mode_sound.sound_lines));

		break;
		case PU_MODE_VESHANIE_MCHS:
			pu_enable_sound_lines(setting.mode_mchs.sound_lines);

			mchs_mic_start(FALSE, 0xFF);
			screens_goto(0);


			filelog_fmt("Линии опов.: %s", pu_lines2str(setting.mode_mchs.sound_lines));

		break;
		case PU_MODE_VESHANIE_POJAR:
			set_mux_sound_right_left(SLR_UM1_UM2);
			pu_uo_fire = 1;
			pu_start_translation_pojar_repeat = 0;

			if (!pu_start_translation_pojar_now_uo()) {
				if (!pu_start_translation_pojar_next_uo()) {
					ulog("pu: pu_start_translation_pojar_next_uo first FALSE, goto DEJURNIY"CRLF);
					pu_set_mode(PU_MODE_DEJURNIY);
					return FALSE;
				}
				else {
			          pu_start_translation_pojar_repeat = setting.mode_fire[pu_uo_fire].repeat;
				}
			} else {
		          pu_start_translation_pojar_repeat = setting.mode_fire[pu_uo_fire].repeat;
			}



			screens_goto(0);


			filelog_fmt("Входы сигн.: %s", pu_lines2str(pu_get_fire_lines()));

		break;
		case PU_MODE_RUCHNOI_PUSK_PD:
			set_mux_sound_right_left(SLR_UM1_UM2);
			screens_goto(0);
			//if (pu_enable_lines) filelog_fmt("Линии опов.: %s, ID польз.:%lu", pu_lines2str(pu_enable_lines), pd_id);
			filelog_fmt("ID польз.:%lu", pd_id);
		break;

		case PU_MODE_RUCHNOI_PUSK:
			set_mux_sound_right_left(SLR_UM1_UM2);
			screens_goto(0);
		break;
		case PU_MODE_UPRAVLENIE_DRUGIMI_PU:
			set_mux_sound_right_left(SLR_UM1_UM2);
			//screen6_init((uint8_t *)screen6_pu_addrs, sizeof(screen6_pu_addrs));

			uint8_t screen6_pu_addrs[26];
			uint8_t screen6_pu_addrs_cnt = 0;
			for (uint8_t i = 10; i < 250; i += 10) {
				if (i == setting.uart_local_addr) continue;
				screen6_pu_addrs[screen6_pu_addrs_cnt] = i;
				screen6_pu_addrs_cnt++;

			}
			screen6_init(screen6_pu_addrs, screen6_pu_addrs_cnt);

			screens_goto(6);
		break;
		default: break;

	}

	return TRUE;
}


void pu_init(void)
{
	opus_buffer_init();
}

void pulines2str(char * buf, uint16_t buf_len, uint32_t * arr, uint8_t arr_cnt)
{
	uint8_t a, l;
	uint32_t lines;
	uint8_t addr;

	buf[0] = '\0';
	for (a = 0; a < arr_cnt; a++) {
		lines = arr[a];
		addr = a * 10;
		for (l = 0; l < 25; l++) {
			if (GETBIT(lines, l)) {
				if (buf[0] != '\0') strcat_s(buf, buf_len, ",");
				strcat_s(buf, buf_len, int2str(addr));
				strcat_s(buf, buf_len, ":");
				strcat_s(buf, buf_len, int2str(l + 1));
			}
		}
	}
}

void str2pulines(char * str, uint32_t * arr, uint8_t arr_cnt)
{
	char * ptr, * ptr2;
	int addr, line_num;
	uint8_t a;

	for (a = 0; a < arr_cnt; a++) {
		arr[a] = 0;
	}
	if (!str) return;
	if (!strlen(str)) return;


	ptr = str;
	do {
		addr = str2int(ptr);
		if ((addr > 9) && (addr < 251)) {
			ptr2 = strright(ptr, strlen(ptr), ":");
			if (ptr2) {
				line_num = str2int(ptr2);
				if ((line_num > 0) && (line_num < 26)) {
					SETBIT(arr[addr / 10], line_num - 1);
				}
			}
		}
		ptr = strright(ptr, strlen(ptr), ",");
	} while (ptr);
}


/*

//const char pu_test_str[] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvWxyz";
//static uint32_t pu_test_led_color = LED_COLOR_RED;
//static uint8_t pu_test_str_pos = 0;


//Тест индикации
void pu_test(void)
{
	int i;

	for (i = 1; i < 35; i++) {
		cm7core_set_led_color(i, pu_test_led_color);
	}
	cm7core_set_led_color(MKI_LED_TEST, LED_COLOR_YELLOW);
	cm7core_send_leds();

	switch (pu_test_led_color) {
	  case LED_COLOR_RED:
		  pu_test_led_color = LED_COLOR_GREEN;
	  break;
	  case LED_COLOR_GREEN:
		  pu_test_led_color = LED_COLOR_BLUE;
	  break;
	  case LED_COLOR_BLUE:
		  pu_test_led_color = LED_COLOR_RED;
	  break;
	  default: pu_test_led_color = LED_COLOR_RED;
	}

	set_display_all((char *)&pu_test_str[pu_test_str_pos]);
	send_display();

	pu_test_str_pos++;
	if (pu_test_str_pos > sizeof(pu_test_str) - 81) pu_test_str_pos = 0;
}

#include <malloc.h>

void print_mallinfo(void)
{
	 struct mallinfo mi = mallinfo();

	  ulog_fmt("Total non-mmapped bytes (arena): %lu"CRLF, mi.arena);
	  //ulog_fmt("# of free chunks (ordblks): %lu"CRLF, mi.ordblks);
	  //ulog_fmt("# of free fastbin blocks (smblks): %lu"CRLF, mi.smblks);
	  //ulog_fmt("# of mapped regions (hblks): %lu"CRLF, mi.hblks);
	  //ulog_fmt("Bytes in mapped regions (hblkhd): %lu"CRLF, mi.hblkhd);
	  //ulog_fmt("Max. total allocated space (usmblks): %lu"CRLF, mi.usmblks);
	  //ulog_fmt("Free bytes held in fastbins (fsmblks): %lu"CRLF, mi.fsmblks);
	  ulog_fmt("Total allocated space (uordblks): %lu"CRLF, mi.uordblks);
	  ulog_fmt("Total free space (fordblks): %lu"CRLF, mi.fordblks);
	  //ulog_fmt("Topmost releasable block (keepcost): %lu"CRLF, mi.keepcost);

}

*/
