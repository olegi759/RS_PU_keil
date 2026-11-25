/*
 * setting_data.h
 *
 *  Created on: 23 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_SETTING_DATA_H_
#define COMMON_SETTING_DATA_H_

//#pragma pack(push, 1)



//Источники звука
typedef enum {
	PU_SOUNDSRC_MICROPHONE = 0,
	PU_SOUNDSRC_FILE,
	PU_SOUNDSRC_LINEINIY_VHOD
} pu_soundsrc_t;


//максимальный размер имени файла
#define SETTING_MAX_FILE_NAME_SIZE	64
//максимальное количество входов пожарной сигнализации
#define MAX_FIRE_INPUT	12
//максимальное количество входов ПД
#define MAX_PD_INPUT	24
//максимальное количество уровней оповещения
#define MAX_UO	10


typedef struct _mode_mchs {
	uint32_t sound_lines;  //Режим МЧС - включение битовое поле
} mode_mchs_t;



//typedef struct _mode_manual {
//	BOOL enable;
//	uint32_t delay;
//	uint8_t repeat;
//	char file[SETTING_MAX_FILE_NAME_SIZE];
//} mode_manual_t;



typedef struct _mode_fire {
	BOOL enable;
	uint32_t delay;
	uint8_t repeat;
	uint32_t input_lines[25];
	BOOL tilibom;
	char file[SETTING_MAX_FILE_NAME_SIZE];
} mode_fire_t;






typedef struct _mode_sound {
	//BOOL enable;
	uint32_t sound_lines;  //Режим звуковое вещанте - включение битовое поле
	//uint32_t delay;
	pu_soundsrc_t soundsrc;
	BOOL tilibom;
	char file[SETTING_MAX_FILE_NAME_SIZE];
} mode_sound_t;


typedef struct _netfunc_fire {
	uint8_t pu_addr;	//адрес ПУ
	uint8_t fire_line;	//номер линии пожарной сигнализации
	uint32_t sound_lines; //звуковые линии
	char file[SETTING_MAX_FILE_NAME_SIZE];
} netfunc_fire_t;

typedef struct _netfunc_mchs {
	uint8_t pu_addr;	//адрес ПУ
	uint32_t sound_lines; //звуковые линии
} netfunc_mchs_t;

typedef struct _user {
	char pin_code[9];
	uint8_t level;
	char name[12];
} user_t;

typedef struct  _chipNetInfo{
	//net
	uint8_t mac[6]; //мак адрес
	BOOL dhcp; //DHCP
	uint32_t ip; //ip - адрес
	uint32_t mask; //ip - маска подсети
	uint32_t gw; //ip - шлююза
	uint32_t dns; // ip - dns сервера
} _chipNetInfo;

typedef struct _setting {
	//char name[28 + 1]; //имя прибора
	uint8_t uart_local_addr; //локальный UART адрес
	uint16_t sai_volume; //громкость SAI
	uint8_t lines_count; //кол-во линий 6/12

	_chipNetInfo chipsNetInfo[2];

	uint8_t pd_count; //кол-во ПД
	uint8_t um_count; //кол-во УМ

	uint16_t mic_volume; //громксоть микрофона
	uint16_t mchs_volume; //громксоть мчс


	uint32_t mkk_um1; //Включение усилителя мощности на УМ1 - битовое поле
	uint32_t mkk_um2; //Включение усилителя мощности на УМ2 - битовое поле
	uint8_t mkk_procent; //отклонения линий вещания
	uint32_t mkk_sound_lines;	//Контроль исправности линий звукового оповещения - включение битовое поле
	uint32_t mkk_insig_lines;	//Контроль исправности входов сигнализации - включение битовое поле
	uint8_t mkk_start_lines;	//Контроль исправности входов старт - включение битовое поле


	mode_mchs_t mode_mchs;

	//mode_manual_t mode_manual[11];

	BOOL mode_manual_tilibom_mic;
	BOOL mode_manual_tilibom_file;


	mode_fire_t mode_fire[11];

	mode_sound_t mode_sound;

	uint8_t int_spk; //внешний динамик подключение

	//uint16_t zone_numbers[13]; //ноиера зоны
	//char zone_descriptions[13][64]; //описание зоны

	uint32_t groups[25];

	BOOL mic_is_fix; //микрофон с фиксацией

	//BOOL manual_cycle;
	BOOL fire_cycle;

	user_t users[11];

	//char pin_code_level_1[9];
	//char pin_code_level_2[9];
	//char pin_code_level_3[9];
	//char pin_code_level_4[9];

	//char web_login[20];
	//char web_password[20];

	uint32_t log_file_size;

	netfunc_fire_t netfunc_fire[11];

	netfunc_mchs_t netfunc_mchs;

	uint32_t control_time_sec; //контроля ЛО через 80 секунд, 2 часа или 12 часов


} setting_t;



//#pragma pack(pop)

#endif /* COMMON_SETTING_DATA_H_ */
