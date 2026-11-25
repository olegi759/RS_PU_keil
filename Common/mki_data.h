/*
 * piv_data.h
 *
 *  Created on: 12 марта. 2021 г.
 *      Author: Rafael Boltachev
 *
 */

#ifndef MKI_DATA_H_
#define MKI_DATA_H_

//Проткол обмена с МКИ

//Скорость 115200.
//UART 9 bit. девятый бит устанавливается только в первом байте в пакете.

//Запрос: [Комманда][ ..... Данные комманды .....][CRC]
//Ответ ОК:  [Комманда][ ..... Данные ответа на комманду .....][CRC]
//Ответ ERROR:  [0x80 | Комманда][Код ошибки][CRC]

#define	MKI_KEY_1				1
#define	MKI_KEY_2				2
#define	MKI_KEY_3				3
#define	MKI_KEY_4				4
#define	MKI_KEY_5				5
#define	MKI_KEY_6				6
#define	MKI_KEY_7				7
#define	MKI_KEY_8				8
#define	MKI_KEY_9				9
#define	MKI_KEY_10				10
#define	MKI_KEY_11				11
#define	MKI_KEY_12				12
#define	MKI_KEY_13				13
#define	MKI_KEY_14				14
#define	MKI_KEY_15				15
#define	MKI_KEY_16				16
#define	MKI_KEY_17				17
#define	MKI_KEY_18				18
#define	MKI_KEY_19				19
#define	MKI_KEY_20				20
#define	MKI_KEY_21				21
#define	MKI_KEY_22				22
#define	MKI_KEY_23				23
#define	MKI_KEY_24				24
#define	MKI_KEY_TEST			25
#define	MKI_KEY_MUTE			26
#define	MKI_KEY_RESET			27
#define	MKI_KEY_MIC				28
#define	MKI_KEY_MENU			29
#define	MKI_KEY_LEFT			30
#define	MKI_KEY_UP				31
#define	MKI_KEY_RIGHT			32
#define	MKI_KEY_DELETE			33
#define	MKI_KEY_EXIT			34
#define	MKI_KEY_DOWN			35
#define	MKI_KEY_ENTER			36
#define	MKI_KEY_ZONES			37
#define	MKI_KEY_GROUPS			38
#define	MKI_KEY_ALLZONES		39
#define	MKI_KEY_MANUALSTART		40


#define	MKI_LED_1				1
#define	MKI_LED_2				2
#define	MKI_LED_3				3
#define	MKI_LED_4				4
#define	MKI_LED_5				5
#define	MKI_LED_6				6
#define	MKI_LED_7				7
#define	MKI_LED_8				8
#define	MKI_LED_9				9
#define	MKI_LED_10				10
#define	MKI_LED_11				11
#define	MKI_LED_12				12
#define	MKI_LED_13				13
#define	MKI_LED_14				14
#define	MKI_LED_15				15
#define	MKI_LED_16				16
#define	MKI_LED_17				17
#define	MKI_LED_18				18
#define	MKI_LED_19				19
#define	MKI_LED_20				20
#define	MKI_LED_21				21
#define	MKI_LED_22				22
#define	MKI_LED_23				23
#define	MKI_LED_24				24
#define	MKI_LED_FIRE			25
#define	MKI_LED_NET				26
#define	MKI_LED_BATTERY			27
#define	MKI_LED_FAULT			28
#define	MKI_LED_DISCONNECT		29
#define	MKI_LED_AUTOMATIONOFF	30
#define	MKI_LED_TEST			31
#define	MKI_LED_MUTE			32
#define	MKI_LED_MICROPHONE		33
#define	MKI_LED_MANUALSTART		34

//#define LED_COLOR_BLACK		0x000000
//#define LED_COLOR_MAROON	0x000080
//#define LED_COLOR_GREEN		0x00FF00
//#define LED_COLOR_OLIVE		0x008080
//#define LED_COLOR_NAVY		0x800000
//#define LED_COLOR_TEAL		0x808000
//#define LED_COLOR_GRAY		0x101010
//#define LED_COLOR_SILVER	0xC0C0C0
//#define LED_COLOR_RED		0xFF0000
//#define LED_COLOR_BLUE		0x0000FF
//#define LED_COLOR_WHITE		0xFFFFFF
//жельый
//#define LED_COLOR_YELLOW	0xFFFF00
//светло фиолетовый
//#define LED_COLOR_FUCHSIA	0xFF00FF
//светло голубой
//#define LED_COLOR_AQUA		0x00FFFF

//#define MKI_COLOR_BLINK		0x800000

//темно жельый
//#define LED_COLOR_BLACK_YELLOW	0x808000

//Запрос нажатой клавиши клавиатуры
#define MKI_CMD_KEY			0x01
//Запрос 	[MKI_CMD_KEY][CRC]
//Ответ OK	[MKI_CMD_KEY][НАЖАТАЯ КЛАВИША 1 байт][CRC]

//Запрос на изменение дисплея
#define MKI_CMD_DISPLAY		0x02
//Запрос 	[MKI_CMD_DISPLAY][80 байт символов дисплея][CRC]
//Ответ OK	[MKI_CMD_DISPLAY][CRC]


//Запрос на изменение цвета светодиода
#define MKI_CMD_LED			0x03
//Запрос 	[MKI_CMD_LED][НОМЕР СВЕТОДИОДА 1 байт][R - 1 байт][G - 1 байт][B - 1 байт][CRC]
//Ответ OK	[MKI_CMD_LED][CRC]

//Запрос на изменение всех цветов светодиодов на черный
#define MKI_CMD_LED_CLEAR	0x04
//Запрос 	[MKI_CMD_LED_CLEAR][CRC]
//Ответ OK	[MKI_CMD_LED_CLEAR][CRC]

//Не используется
#define MKI_CMD_DISP_XY		0x05

//Не используется
#define MKI_CMD_DISPLINE	0x06


//Запрос на изменение цветов светодиодов
#define MKI_CMD_LEDS		0x07
//Запрос 	[MKI_CMD_LEDS][R - 1 байт][G - 1 байт][B - 1 байт] ...34 светодиода ... [R - 1 байт][G - 1 байт][B - 1 байт][CRC]
//Ответ OK	[MKI_CMD_LEDS][CRC]

//Запрос на перезагрузку
#define MKI_CMD_REBOOT		0x10
//Запрос 	[MKI_CMD_REBOOT][CRC]
//Ответ OK	[MKI_CMD_REBOOT][CRC]

//Запрос на версию
#define MKI_CMD_VERSION		0x11
//Запрос 	[MKI_CMD_VERSION][CRC]
//Ответ OK	[MKI_CMD_VERSION][СТРОКА ВЕРСИИ ПРОШИВКИ МКИ][CRC]



//Запрос на изменение цвета светодиода
#define MKI_COL_SMALL_LED_BLUE_BIT			0
#define MKI_COL_SMALL_LED_GREEN_BIT			1
#define MKI_COL_SMALL_LED_RED_BIT			2
#define MKI_COL_SMALL_LED_FLASH_BIT			3


#define LED_COLOR_BLACK		0
#define LED_COLOR_BLUE		1
#define LED_COLOR_GREEN		2
#define LED_COLOR_AQUA		3
#define LED_COLOR_RED		4
#define LED_COLOR_FUCHSIA	5
#define LED_COLOR_YELLOW	6
#define LED_COLOR_WHITE		7

//все 34 светодиода (17 байт)
#define MKI_CMD_SMALL_LEDS			0x12
//Запрос 	[MKI_CMD_SMALL_LEDS][ЦВЕТ СВЕТОДИОДА 4 бита][ЦВЕТ СВЕТОДИОДА 4 бита]....[CRC]
//Ответ OK	[MKI_CMD_SMALL_LEDS][CRC]


#define MKI_CMD_TEST				0x13
//Запрос 	[MKI_CMD_TEST]Включен/Выключен 1 байт][CRC]
//Ответ OK	[MKI_CMD_TEST][CRC]


#define MKI_CMD_SMALL_LED			0x14
//Запрос 	[MKI_CMD_SMALL_LED][НОМЕР СВЕТОДИОДА 1 байт][ЦВЕТ СВЕТОДИОДА 1 байт]....[CRC]
//Ответ OK	[MKI_CMD_SMALL_LED][CRC]


//Код ошибки неправильные параметры
#define MKI_ERROR_PARAMS	0xF1

//Код ошибки неправильное выполнение комманды
#define MKI_ERROR_RESULT	0xF2

#pragma pack(push, 1)

typedef struct _mki_led_param {
	uint8_t number;
	uint32_t color;
} mki_led_param_t;

#pragma pack(pop)



#endif
