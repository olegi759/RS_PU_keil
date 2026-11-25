/*
 * pd_data.h
 *
 *  Created on: 14 апр. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_PD_DATA_H_
#define COMMON_PD_DATA_H_



#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


//Проткол Информационного Взаимодействия (ПИВ) между ПУ и ПД в общем виде
//[uint8_t Адрес кому][uint8_t Адрес от кого][uint16_t длина данных][....данные ....][uint16_t контрольная сумма]
//Данные протокола описаны ниже


//Запрос у ПД комманд:
//[uint8_t PD_CMD_QUERY]
#define	PD_CMD_QUERY							0x55




// ----------------------- ПИНГ -------------------------------------------
#define PD_CMD_PING								0x3F
//[uint8_t PD_CMD_RING]
#define PD_CMD_PONG								0x6F
//[uint8_t PD_CMD_PONG]


// ------------------- ЗАПРОС у ПУ его состояния ---------------------------------------
//ЗАПРОС
#define	PD_CMD_INFO								0x40
//[uint8_t PD_CMD_INFO]


//ОТВЕТ
#define	PD_CMD_INFO_RES							0x70
//[uint8_t PD_CMD_INFO_RES][uint8_t Режим работы ПУ][uint8_t Параметры режима работы ПУ][uint8_t Ошибки ПУ] +
//[uint8_t Состояние линий пожарной сигнализации 1..8][uint8_t Состояние линий пожарной сигнализации 9..17][uint8_t Состояние линий пожарной сигнализации 18..24] +
//[uint8_t Состояние линий линий вещания 1..8][uint8_t Состояние линий вещания 9..17][uint8_t Состояние линий вещания 18..24]
//[uint8_t Состояние ЛО 1..8][uint8_t Состояние ЛО 9..17][uint8_t Состояние ЛО 18..24]
//[uint8_t Состояние ВС 1..8][uint8_t Состояние ВС 9..17][uint8_t Состояние ВС 18..24]
//[uint8_t год][uint8_t месяц][uint8_t день][uint8_t час][uint8_t минут]
//[uint8_t текущее УО][uint8_t время сек. до старта УО]
//[uint8_t кол-во линий ПУ]
//[uint24_t 3 байта NETINFO]




//Ошибки ПУ - битовон поле, состояние битов ниже
//ошибка модуля МУП
#define PD_CMD_QUERY_FAULT_SMALL_MUP				0
//ошибка модуля МКК линии сигнализации
#define PD_CMD_QUERY_FAULT_SMALL_MKK_INSIG			1
//ошибка модуля МКК линии вещания
#define PD_CMD_QUERY_FAULT_SMALL_MKK_SOUND			2
//ошибка модуля МКК линии дополнительной ф-ции
#define PD_CMD_QUERY_FAULT_SMALL_MKK_START			3
//ошибка соединения с ПД
#define PD_CMD_QUERY_FAULT_SMALL_PD_CON				4
//ошибка соединения с УМ
#define PD_CMD_QUERY_FAULT_UM_CON					5
//ошибка УМ
#define PD_CMD_QUERY_FAULT_UM						6
//ошибка устройтва на линии дополнительной ф-ции (внешних приборов)
#define PD_CMD_QUERY_FAULT_SMALL_MKK_START_DEVICE	7

#pragma pack(push, 1)
//Ошибки ПУ - битовон поле, состояние битов ниже
typedef struct _pu_errors {
	//ошибка модуля МУП
	uint8_t PD_CMD_QUERY_FAULT_SMALL_MUP_BIT: 1;        		//0
	//ошибка модуля МКК линии сигнализации
	uint8_t PD_CMD_QUERY_FAULT_SMALL_MKK_INSIG_BIT: 1;      	//1
	//ошибка модуля МКК линии вещания
	uint8_t PD_CMD_QUERY_FAULT_SMALL_MKK_SOUND_BIT: 1;      	//2
	//ошибка модуля МКК линии дополнительной ф-ции
	uint8_t PD_CMD_QUERY_FAULT_SMALL_MKK_START_BIT: 1;      	//3
	//ошибка соединения с ПД
	uint8_t PD_CMD_QUERY_FAULT_SMALL_PD_CON_BIT: 1;        		//4
	//ошибка соединения с УМ
	uint8_t PD_CMD_QUERY_FAULT_UM_CON_BIT: 1;          			//5
	//ошибка УМ
	uint8_t PD_CMD_QUERY_FAULT_UM_BIT: 1;            			//6
	//ошибка устройтва на линии дополнительной ф-ции
	uint8_t PD_CMD_QUERY_FAULT_SMALL_MKK_START_DEVICE_BIT: 1;  	//7
} pu_errors_t;
#pragma pack(pop)



// ------------------- ЗАПРОС к ПУ перейди в режим Трансляцию ПД ---------------------------------------

//ЗАПРОС
#define	PD_CMD_VESHANIE_PD						0x41
//[uint8_t PD_CMD_VESHANIE_PD][uint32_t frequency][uint8_t stereo][uint8_t Включить линию вещания 1..8][uint8_t Включить линию вещания 9..16][uint8_t Включить линию вещания 17..24][uint32_t ID]

//ОТВЕТ
#define	PD_CMD_VESHANIE_PD_RES					0x71
//[uint8_t PD_CMD_VESHANIE_PD_RES]


// ------------------- ЗАПРОС к ПУ перейди в режим Дежурный ---------------------------------------

//ЗАПРОС
#define	PD_CMD_DEJURNIY							0x42
//[uint8_t PD_CMD_DEJURNIY]

//ОТВЕТ
#define	PD_CMD_DEJURNIY_RES						0x72
//[uint8_t PD_CMD_DEJURNIY]


// ------------------- ЗАПРОС к ПУ перейди в режим Звуковое вещание ---------------------------------------

//ЗАПРОС
#define	PD_CMD_ZVUKOVOE_VESHANIE				0x43
//[uint8_t PD_CMD_ZVUKOVOE_VESHANIE][uint8_t Включить линию вещания 1..8][uint8_t Включить линию вещания 9..16][uint8_t Включить линию вещания 17..24][uint32_t ID]

//ОТВЕТ
#define	PD_CMD_ZVUKOVOE_VESHANIE_RES			0x73
//[uint8_t PD_CMD_ZVUKOVOE_VESHANIE_RES]



// ------------------- ЗАПРОС к ПУ перейди в режим Пожар ---------------------------------------

//ЗАПРОС
#define	PD_CMD_VESHANIE_POJAR					0x44
//[uint8_t PD_CMD_VESHANIE_POJAR_POJAR][uint8_t Имитация состояний линий пожарной сигнализации 1..8][uint8_t Имитация состояний линий пожарной сигнализации 9..16][uint8_t Имитация состояний линий пожарной сигнализации 17..24][uint32_t ID]


//ОТВЕТ
#define	PD_CMD_VESHANIE_POJAR_RES				0x74
//[uint8_t PD_CMD_VESHANIE_POJAR_POJAR_RES]


// ------------------- ЗАПРОС к ПУ перейди в режим вещание МЧС ---------------------------------------

//ЗАПРОС переход в режим вещание МЧС:
#define	PD_CMD_VESHANIE_MCHS					0x45
//[uint8_t PD_CMD_VESHANIE_MCHS][uint8_t Включить линию вещания 1..8][uint8_t Включить линию вещания 9..16][uint8_t Включить линию вещания 17..24][uint32_t ID]

//ОТВЕТ
#define	PD_CMD_VESHANIE_MCHS_RES				0x75
//[uint8_t PD_CMD_VESHANIE_MCHS_RES]


// ------------------- ЗАПРОС к ПУ опция Отключение ---------------------------------------

//ЗАПРОС (БЫЛО 0x51 ИЗМЕНИЛ НА 0x46)
#define	PD_CMD_OTKLUCHENIE						0x46
//[uint8_t PD_CMD_OTKLUCHENIE][uint8_t Отключить(разорвать) линию пожарной сигнализации 1..8][uint8_t Отключить(разорвать) линию пожарной сигрализации 9..16][uint8_t Отключить(разорвать) линию пожарной сигнализации 17..24][uint32_t ID]

//ОТВЕТ
#define	PD_CMD_OTKLUCHENIE_RES					0x76
//[uint8_t PD_CMD_OTKLUCHENIE_RES]


// ------------------- ЗАПРОС к ПУ Автоматика отключена ---------------------------------------

//ЗАПРОС (БЫЛО 0x52 ИЗМЕНИЛ НА 0x47)
#define	PD_CMD_AVTOMATIKA_OTKLUCHENA			0x47
//[uint8_t PD_CMD_AVTOMATIKA_OTKLUCHENA][uint8_t Отключить(разорвать) линию вещания 1..8][uint8_t Отключить(разорвать) линию вещания 9..16][uint8_t Отключить(разорвать) линию вещания 17..24][uint32_t ID]


//ОТВЕТ
#define	PD_CMD_AVTOMATIKA_OTKLUCHENA_RES		0x77
//[uint8_t PD_CMD_AVTOMATIKA_OTKLUCHENA_RES]


// ------------------- ЗАПРОС к ПУ переход в режим Ручной ПУСК ---------------------------------------

//ЗАПРОС (БЫЛО 0x61 ИЗМЕНИЛ НА 0x48)
#define	PD_CMD_RUCHNOI_PUSK						0x48
//[uint8_t PD_CMD_RUCHNOI_PUSK][uint32_t frequency][uint8_t stereo][uint8_t Включить линию вещания 1..8][uint8_t Включить линию вещания 9..16][uint8_t Включить линию вещания 17..24][uint32_t ID]

//ОТВЕТ
#define	PD_CMD_RUCHNOI_PUSK_RES					0x78
//[uint8_t PD_CMD_RUCHNOI_PUSK_RES]


// ------------------- ЗАПРОС к ПУ дай список ПУ в сети ---------------------------------------

//ЗАПРОС
#define	PD_CMD_SPISOK_PU						0x49
//[uint8_t PD_CMD_SPISOK_PU]

//ОТВЕТ
#define	PD_CMD_SPISOK_PU_RES					0x79
//[uint8_t PD_CMD_SPISOK_PU_RES][uint8_t адрес ПУ][uint8_t Режим работы ПУ][uint8_t Кол-во линий на ПУ][uint24_t состояние пожарных линий].....[uint8_t адрес ПУ][uint8_t Режим работы ПУ][uint8_t Кол-во линий на ПУ][uint24_t состояние пожарных линий]

// -------------------------------------------------------------------------------------------------

#define	PD_CMD_NET_INFO			0x75
//[uint8_t PD_CMD_NET_INFO][uint24_t bytes]

#define	PD_CMD_NET_INFO_RES		0x76
//[uint8_t PD_CMD_NET_INFO_RES]



#define	PU_CMD_NET_INFO_UPDATE			0x40

#define PU_CMD_CHANGE_MODE				0x39

#define PU_CMD_RING_AUDIO_CHECK			0x38

#define PU_CMD_RING_CMD_CHECK			0x37

// -------------------------------------------------------------------------------------------------
// для звукового пакета. Opus 48k mono
#define PD_CMD_OPUS48mono      0x71
// для звукового пакета. Opus 16k mono
#define PD_CMD_OPUS16mono      0x72
// для звукового пакета. Opus 48k stereo
#define PD_CMD_OPUS48ster      0x73
// для звукового пакета. Opus 16k stereo
#define PD_CMD_OPUS16ster      0x74


#pragma pack(push, 1)

typedef struct {
	uint8_t daddr;
	uint8_t saddr;
	uint16_t data_len;
} pd_proto_header_t;

typedef struct {
	uint8_t cmd;
} pd_onlycommand_data_t;


typedef struct {
	pd_proto_header_t header;
	pd_onlycommand_data_t data;
	uint16_t crc;
} pd_onlycommand_packet_t;

typedef pd_onlycommand_packet_t pd_dejurniy_packet_t;



typedef struct {
	uint8_t pu_state;
	uint8_t pu_params;
	uint8_t pu_errors;
	uint32_t fire_lines : 24;
	uint32_t sound_lines : 24;
	uint32_t otkluchenie_lines : 24;
	uint32_t avtomatika_otkluchena_lines : 24;
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t pu_uo_fire;
	uint8_t pu_uo_time_remaining;
	uint8_t lines_count;
} pd_info_cmd_data_t;

typedef struct {
	uint8_t cmd;
	pd_info_cmd_data_t cmd_data;
} pd_info_data_t;

typedef struct {
	pd_proto_header_t header;
	pd_info_data_t data;
	uint16_t crc;
} pd_info_packet_t;


typedef struct {
	uint32_t frequency;
	uint8_t stereo;
	uint32_t sound_lines: 24;
	uint32_t id;
} pd_audio_param_cmd_data_t;

typedef struct {
	uint8_t cmd;
	pd_audio_param_cmd_data_t cmd_data;
} pd_adio_param_data_t;

typedef struct {
	pd_proto_header_t header;
	pd_adio_param_data_t data;
	uint16_t crc;
} pd_adio_param_packet_t;

typedef pd_adio_param_packet_t pd_veshanie_pd_packet_t;
typedef pd_adio_param_packet_t pd_ruchnoi_pusk_packet_t;



typedef struct {
	uint32_t sound_lines: 24;
	uint32_t id;
} pd_sound_lines_cmd_data_t;


typedef struct {
	uint8_t cmd;
	pd_sound_lines_cmd_data_t cmd_data;
} pd_sound_lines_data_t;

typedef struct {
	pd_proto_header_t header;
	pd_sound_lines_data_t data;
	uint16_t crc;
} pd_sound_lines_packet_t;



typedef pd_sound_lines_packet_t pd_zvukovoe_veshanie_packet_t;
typedef pd_sound_lines_packet_t pd_veshanie_pojar_packet_t;
typedef pd_sound_lines_packet_t pd_otkluchenie_packet_t;
typedef pd_sound_lines_packet_t pd_avtomatika_otkluchena_packet_t;


typedef struct {
	uint8_t addr;
	uint8_t mode;
	uint8_t lines_count;
} pd_spiskok_pu_item_t;


#pragma pack(pop)


/*
//Два бита на кодировние кодека

//aLaw
#define PD_AUDIO_CODEC_ALAW  	0
//uLaw
#define PD_AUDIO_CODEC_ULAW  	1
//Спекс
#define PD_AUDIO_CODEC_SPEEX  	2
//Опус
#define PD_AUDIO_CODEC_OPUS  	3



//Три бита на кодировние частоты


//8000U
#define PD_AUDIO_FREQUENCY_8K	0
//11025U
#define PD_AUDIO_FREQUENCY_11K	1
//16000U
#define PD_AUDIO_FREQUENCY_16K  2
//22050U
#define PD_AUDIO_FREQUENCY_22K	3
//32000U
#define PD_AUDIO_FREQUENCY_32K  4
//44100U
#define PD_AUDIO_FREQUENCY_44K  5
//48000U
#define PD_AUDIO_FREQUENCY_48K  6
//96000U
#define PD_AUDIO_FREQUENCY_96K  7


//Один бит на кодировние моно или стерео
//моно
#define PD_AUDIO_MONO			0
//стерео
#define PD_AUDIO_STEREO			1

#pragma pack(push, 1)

typedef struct _audio_params {
	uint8_t reserve1: 1;
	uint8_t codec: 2;
	uint8_t reserve2: 1;
	uint8_t frequency: 3;
	uint8_t stereo: 1;
} audio_params_t;


typedef union _audio_params_u
{
  uint8_t audio_params_uint8;
  audio_params_t audio_params;
} audio_params_u_t;


#pragma pack(pop)
*/


#ifdef __cplusplus
}
#endif

#endif /* COMMON_PD_DATA_H_ */
