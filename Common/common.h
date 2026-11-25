/*
 * common.h
 *
 *  Created on: 11 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include "main.h"

#include "setting_data.h"
#include "shared_memory.h"

#pragma pack(push, 1)

typedef struct _mkk_insig_lin_replay_ {
	//контроль исправностей линий звука
	uint8_t sound[24];
	//контроль исправностей линий пожарной сигнализации
	uint8_t insig[24];
	//контроль исправностей линий старт
	uint8_t start[2];
} mkk_insig_lin_replay_t;

typedef enum {
	PU_MODE_DEJURNIY = 1, //дежурный
	PU_MODE_ZVUKOVOE_VESHANIE,
	PU_MODE_VESHANIE_PD,
	PU_MODE_VESHANIE_POJAR,
	PU_MODE_VESHANIE_MCHS,
	PU_MODE_RUCHNOI_PUSK_PD,
	PU_MODE_RUCHNOI_PUSK,
	PU_MODE_UPRAVLENIE_DRUGIMI_PU
} pu_mode_t;

typedef enum {
	PU_OPTION_NONE = 0,
	PU_OPTION_OTKLUCHENIE,
	PU_OPYION_AVTOMATIKA_OTKLUCHENA
} pu_option_t;


typedef struct {
	uint8_t addr;
	uint8_t mode;
	uint8_t lines_count;
	uint32_t lines : 24;
} pu_small_info_t;


#pragma pack(pop)


//--- Копируемые переменные

extern setting_t setting;

#define  MAX_FAULTS	128
extern uint8_t faults[MAX_FAULTS / 8];

extern uint8_t date_time[5];

extern uint32_t pu_enable_lines;

extern char safety_pin_code[9];

extern mkk_insig_lin_replay_t mkk_insig_lin_replay;

extern pu_mode_t pu_mode;

extern pu_option_t pu_option;

//--- разделяемая память

extern shared_memory_t * sm;

extern uint32_t stm32_uid[3];

void shared_memory_init(void);

char * get_date_time_str(void);
char * get_time_str(void);
uint32_t get_time_stamp(void);


const char * mki_get_key_name(uint8_t key);

#define FACTORY_PIN_CODE_LEVEL_5		"163457"


uint8_t safety_get_level_from_pin_code(void);
char * safety_get_name_from_pin_code(void);


const char * mkk_lin_state2str(uint8_t val);
const char * mkk_insig_state2str(uint8_t val);
const char * mkk_start_state2str(uint8_t val);


const char * pu_mode2str(pu_mode_t mode);
const uint32_t pu_mode2ledcolor(pu_mode_t mode);

BOOL fault_is_set(uint8_t num);
//BOOL fault_have(void);
uint8_t fault_first(void);
uint8_t fault_count(void);
const char * fault2str(uint8_t num);


void fill_crc16(uint8_t * packet, uint16_t packet_len);

BOOL is_correct_crc16(uint8_t * packet, uint16_t packet_len);
BOOL is_correct_datalen(uint8_t * packet, uint16_t packet_len);


uint16_t create_piv_packet(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t * data, uint16_t data_cnt);
uint16_t create_piv_packet_cmd(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len);
uint16_t create_piv_packet_cmd2(uint8_t * buf, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len, uint16_t packet_id);



#pragma pack(push, 1)

typedef struct {
	uint8_t daddr;
	uint8_t saddr;
	uint16_t data_len;
	uint8_t cmd;
	uint8_t cmd_data[];
} pu_packet_cmd_t;

#pragma pack(pop)

pu_packet_cmd_t * parse_piv_packet(uint8_t * packet, uint16_t packet_len);

void filelog(char * str);
void filelog_fmt(const char * fmt, ... );



#define CMD_SETAUDIOPARAMS "SETAUDIOPARAMS "

#define KEY_CHANNELS 	"channels:"
#define KEY_FREQUENCY	"frequency:"
#define KEY_ENDPOINTS	"endpoints:"




#define CMD_AUDIO_ULAW_TYPE						10
#define CMD_AUDIO_ALAW_TYPE						12
#define CMD_AUDIO_MP3_MONO_TYPE					14
#define CMD_AUDIO_MP3_STEREO_TYPE				15
#define CMD_AUDIO_SPEEX_TYPE					16
#define CMD_AUDIO_OPUS_MONO_TYPE				18
#define CMD_AUDIO_OPUS_STEREO_TYPE				19
#define CMD_AUDIO_OPUS_MONO_LEFT_RIGHT_TYPE		20
#define CMD_AUDIO_OPUS_MONO_LEFT_TYPE			21
#define CMD_AUDIO_OPUS_MONO_RIGHT_TYPE			22


#define AUDIO_FREQUENCY_8				30
#define AUDIO_FREQUENCY_11				31
#define AUDIO_FREQUENCY_16				32
#define AUDIO_FREQUENCY_22				33
#define AUDIO_FREQUENCY_24				34
#define AUDIO_FREQUENCY_32				35
#define AUDIO_FREQUENCY_44				36
#define AUDIO_FREQUENCY_48				37






uint8_t freq2byte(uint32_t freq);
uint32_t byte2freq(uint8_t freq);

BOOL chk_str_crc(char * data, uint16_t data_len);



//void clear_clrf(char * data, uint16_t data_len);


#define THREEBYTE2LINES(first_byte, second_byte, third_byte)			LE_THREEBYTE2U24(first_byte, second_byte, third_byte)

char * pu_lines2str(uint32_t lines);

BOOL is_set_setting_pin_code(void);

void ulogmem(void * mem, uint16_t mem_size, BOOL is_us);


BOOL is_file_name_opus(const char * filename);
BOOL is_file_name_mp3(const char * filename);

/*
BOOL uart_9bit_snd(UART_HandleTypeDef *huart, uint8_t * data, uint16_t data_len, uint16_t * buf, uint16_t buf_len, BOOL it);
*/

char * get_pu_name(void);

#endif /* COMMON_COMMON_H_ */
