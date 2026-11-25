/*
 * pu.h
 *
 *  Created on: 26 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_PU_H_
#define CORE_INC_PU_H_

//1#include "main.h"
#include <stdint.h>
#include "def_util.h"
#include "setting_data.h"
#include "mki_data.h"
#include "common.h"


extern uint32_t pu_ruchnoi_pusk_lines;
//extern uint32_t pu_upravlenie_drugimi_pu_lines;


#define	PU_TYPE_PU2  	0
#define	PU_TYPE_PU4		1
#define	PU_TYPE_PU8		2
#define	PU_TYPE_PU12	3
#define	PU_TYPE_PU24	4

#pragma pack(push, 1)

typedef struct {
	uint16_t shapka;
	uint8_t version;
	uint8_t addr_from;
	uint8_t params;
	uint8_t state;
	uint8_t errors;
	uint32_t fire_lines_active: 24;
	uint32_t sound_lines_active: 24;
	uint32_t fire_lines_state : 24;
	uint32_t sound_lines_state: 24;

} pu_proto_state_t;

#pragma pack(pop)


//Режим работы по приоритетам 0 самый слабый

BOOL pu_play_audio_file(const char * filename);

void pu_invalid_access_code(char * ulog_msg, uint8_t need_level);

uint32_t pu_get_fire_lines(void);


const char * pu_mode2str(pu_mode_t mode);
const uint32_t pu_mode2ledcolor(pu_mode_t mode);

BOOL pu_set_mode(pu_mode_t new_pu_mode);




void pu_stop_translation(void);
void pu_dma_stop(void);

uint16_t pu_get_data_from_small_infos(uint8_t * data);
void pu_get_local_small_info(pu_small_info_t  * small_info);
void pu_fill_local_small_infos(void);



extern uint32_t pu_test_enable_tick;

void pu_test(void);
void pu_init(void);

extern uint8_t pu_uo_fire;
//extern uint8_t pu_uo_manual;

void pu_paint_led_lines(uint32_t lines, uint8_t color);
//void pu_led_clear(void);


BOOL pu_enable_sound_lines(uint32_t lines);



//BOOL pu_start_translation_manual_now_uo(void);
//BOOL pu_start_translation_manual_next_uo(void);

//extern BOOL pu_mode_manual_is_start_translation;

uint8_t pu_enable_lines_cnt(void);
uint8_t pu_get_fire_lines_cnt(void);

uint8_t get_file_src(char * file_name);

const char * soundsrc2str(pu_soundsrc_t soundsrc);

//extern uint32_t fire_lines;
extern uint32_t fire_lines_imit;
//extern uint32_t fire_lines_off;
extern uint32_t fire_lines_insig;


#define LED_COLOR_OPTION_OTKLUCHENIE 				LED_COLOR_YELLOW
#define LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA 		LED_COLOR_YELLOW


//Опция автоматика линии
extern uint32_t pu_option_avtomatika_otkluchena_lines;
//Опция отключение линии
extern uint32_t pu_option_otkluchenie_lines;

//void pu_add_log_fmt(const char * fmt, ... );
char * pu_lines2str(uint32_t lines);

uint32_t pu_get_lines_from_leds(void);
uint8_t pu_get_lines_cnt_from_leds(void);

extern BOOL pu_select_group;
uint32_t pu_get_group_lines_from_leds(void);
uint32_t pu_get_manual_lines_from_leds(void);

extern uint8_t pu_start_translation_manual_repeat;

void pu_task(void);

extern uint8_t uo_fire_time_remaining;
//extern uint8_t uo_manual_time_remaining;

void pu_send_change_mode(void);
void pu_recv_change_mode(uint8_t * packet, uint16_t packet_len);


void pulines2str(char * buf, uint16_t buf_len, uint32_t * arr, uint8_t arr_cnt);
void str2pulines(char * str, uint32_t * arr, uint8_t arr_cnt);

#define PU_INFO_SIZE	27

void pu_get_info(uint8_t info[PU_INFO_SIZE]);
extern const uint8_t * pu_enable_lines_ptr;

extern uint8_t faults_small;

#pragma pack(push, 1)



typedef struct {
	uint8_t cmd;
	uint32_t frequency;
	uint8_t stereo;
	uint32_t lines : 24;
} pu_change_state_data_t;

typedef struct {
	uint8_t daddr;
	uint8_t saddr;
	uint16_t data_len;
	pu_change_state_data_t data;
	uint16_t crc;
} pu_change_state_t;







typedef struct {
	uint8_t daddr;
	uint8_t saddr;
	uint16_t data_len;
	uint8_t cmd;
	uint8_t opus_data[];
} pu_cmd_opus_t;



// УПАКОВКА ПАРАМЕТРОВ ЗВУУА

//Два бита на кодировние кодека

//aLaw
#define AUDIO_PARAM_CODEC_ALAW  	0
//uLaw
#define AUDIO_PARAM_CODEC_ULAW  	1
//Спекс
#define AUDIO_PARAM_CODEC_SPEEX  	2
//Опус
#define AUDIO_PARAM_CODEC_OPUS  	3



//Три бита на кодировние частоты


//8000U
#define AUDIO_PARAM_FREQUENCY_8K	0
//11025U
#define AUDIO_PARAM_FREQUENCY_11K	1
//16000U
#define AUDIO_PARAM_FREQUENCY_16K  2
//22050U
#define AUDIO_PARAM_FREQUENCY_22K	3
//32000U
#define AUDIO_PARAM_FREQUENCY_32K  4
//44100U
#define AUDIO_PARAM_FREQUENCY_44K  5
//48000U
#define AUDIO_PARAM_FREQUENCY_48K  6
//96000U
#define AUDIO_PARAM_FREQUENCY_96K  7


//Один бит на кодировние моно или стерео
//моно
#define AUDIO_PARAM_MONO			0
//стерео
#define AUDIO_PARAM_STEREO			1


typedef struct _audio_params {
	uint8_t reserve2: 1;
	uint8_t codec: 2;
	uint8_t reserve1: 1;
	uint8_t frequency: 3;
	uint8_t stereo: 1;
} audio_params_t;


typedef union _audio_params_u
{
  uint8_t audio_params_uint8;
  audio_params_t audio_params;
} audio_params_u_t;


uint8_t audio_pack(uint8_t codec, uint32_t frequency, uint8_t stereo);
void audio_unpack(uint8_t value, uint8_t * codec, uint32_t * frequency, uint8_t * stereo);



void pu_prepare_pcm1753(uint32_t frequency, BOOL stereo);
BOOL pu_prepare_decoder_opus(uint32_t frequency, BOOL stereo);
BOOL pu_prepare_audio_play(uint32_t frequency, BOOL stereo);

BOOL pu_put_opus_in_pcm1753_now(uint8_t * opus, uint16_t opus_len);



#pragma pack(pop)

extern uint32_t last_frequency;
extern uint32_t pu_small_infos_tick[26];
extern pu_small_info_t pu_small_infos[26];

extern uint8_t pu_netinfo[3];

void tilibom_play(char * file_name);

void pu_um_snd_sinus(void);

//BOOL mode_rucnoi_pusk_to_dejurniy_blocked;

#endif /* CORE_INC_PU_H_ */
