/*
 * screen1.c
 *
 *  Created on: 9 апр. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "screen1.h"
#include "str_util.h"
#include "mki_data.h"
#include "pu.h"
#include "mki_data.h"
#include "screens.h"
#include "screen2.h"
#include "mkk_data.h"
#include "screen4.h"
#include "stm_util.h"
#include "screen6.h"
#include "display.h"
#include "screen3.h"
#include "beep.h"
#include "common.h"
#include "cm7core.h"

//--------------- МЕНЮ --------------------


#define SCREEN1_MAX_ITEMS	10
static uint8_t screen1_index = 0;

#define SCREEN1_REJIM_STR	"РЕЖИМ: "

#define SCREEN1_ITEM_PU_OPTION_AVTOMATIKA_OTKLUCHENA 	0
#define SCREEN1_ITEM_PU_OPTION_OTKLUCHENIE 				1
#define SCREEN1_ITEM_PU_MODE_DEJURNIY 					2
#define SCREEN1_ITEM_PU_MODE_ZVUKOVOE_VESHANIE 			3
#define SCREEN1_ITEM_PU_MODE_VESHANIE_POJAR 			4
#define SCREEN1_ITEM_PU_MODE_VESHANIE_MCHS 				5
#define SCREEN1_ITEM_PU_MODE_RUCHNOI_PUSK 				6
#define SCREEN1_ITEM_PU_MODE_SETEVOE_UPRAVLENIE 		7
#define SCREEN1_ITEM_FAULTS 							8
#define SCREEN1_ITEM_PIN_CODE							9

void screen1_get_item(uint8_t index, char * buf, uint16_t buf_size)
{
	strcpy_s(buf, buf_size, index == screen1_index ? ">" : " ");

	switch (index) {
		case SCREEN1_ITEM_PU_OPTION_AVTOMATIKA_OTKLUCHENA: strcat_s(buf, buf_size, "АВТОМАТИКА ОТКЛЮЧЕНА"); break;
		case SCREEN1_ITEM_PU_OPTION_OTKLUCHENIE: strcat_s(buf, buf_size, "ОТКЛЮЧЕНИЕ"); break;
		case SCREEN1_ITEM_PU_MODE_DEJURNIY: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_DEJURNIY)); break;
		case SCREEN1_ITEM_PU_MODE_ZVUKOVOE_VESHANIE: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_ZVUKOVOE_VESHANIE)); break;
		case SCREEN1_ITEM_PU_MODE_VESHANIE_POJAR: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_VESHANIE_POJAR)); break;
		case SCREEN1_ITEM_PU_MODE_VESHANIE_MCHS: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_VESHANIE_MCHS)); break;
		case SCREEN1_ITEM_PU_MODE_RUCHNOI_PUSK: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_RUCHNOI_PUSK)); break;
		case SCREEN1_ITEM_PU_MODE_SETEVOE_UPRAVLENIE: strcat_s(buf, buf_size, SCREEN1_REJIM_STR); strcat_s(buf, buf_size, pu_mode2str(PU_MODE_UPRAVLENIE_DRUGIMI_PU)); break;
		case SCREEN1_ITEM_FAULTS: strcat_s(buf, buf_size, "НЕИСПРАВНОСТИ"); break;
		case SCREEN1_ITEM_PIN_CODE: strcat_s(buf, buf_size, "ВВОД ПИН КОДА"); break;
		default: break;
	}
}


void screen1_paint(void)
{

	char buf[MAX_STR_SIZE];
	int start_index;

	clear_display();

	start_index = screen1_index - 3;
	if (start_index < 0) start_index = 0;

	screen1_get_item(start_index,  buf, sizeof(buf));
	set_display(0, 0, buf);
	screen1_get_item(start_index + 1, buf, sizeof(buf));
	set_display(1, 0, buf);
	screen1_get_item(start_index + 2, buf, sizeof(buf));
	set_display(2, 0, buf);
	screen1_get_item(start_index + 3, buf, sizeof(buf));
	set_display(3, 0, buf);

	//ulog("300"CRLF);
	send_display();

}

void screen1_init(void)
{
	screen1_index = 0;
}

void screen1_button_press(uint8_t key)
{
	char buf[MAX_STR_SIZE];

	switch (key) {
		case MKI_KEY_UP:
			if (screen1_index) {
				screen1_index--;
				screen1_paint();
			}
			screen1_get_item(screen1_index,  buf, sizeof(buf));
			ulog_fmt("screen: menu index=%d(%s)"CRLF, screen1_index, buf);
		break;
		case MKI_KEY_DOWN:
			if (screen1_index < SCREEN1_MAX_ITEMS -1) {
				screen1_index++;
				screen1_paint();
			}
			screen1_get_item(screen1_index,  buf, sizeof(buf));
			ulog_fmt("screen: menu index=%d(%s)"CRLF, screen1_index, buf);
		break;
		case MKI_KEY_ENTER:
			switch (screen1_index) {
				case SCREEN1_ITEM_PU_OPTION_AVTOMATIKA_OTKLUCHENA:
					if (safety_get_level_from_pin_code() < 2) { //Автоматика отключена (уровень доступа 2, 3,4)
						pu_invalid_access_code(CONM"screen1: error OPTION_AVTOMATIKA_OTKLUCHENA by user level"CRLF, 2);
					 	return;
					}
					pu_option = PU_OPYION_AVTOMATIKA_OTKLUCHENA;
					cm7core_option_change();
					pu_paint_led_lines(pu_option_avtomatika_otkluchena_lines, LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA);
					screens_goto(0);
				break;
				case  SCREEN1_ITEM_PU_OPTION_OTKLUCHENIE:
					if (safety_get_level_from_pin_code() < 2) { //Отключение (уровень доступа 2, 3,4)
						pu_invalid_access_code(CONM"screen1: error OPTION_OTKLUCHENIE by user level"CRLF, 2);
						return;
					}
					pu_option = PU_OPTION_OTKLUCHENIE;
					cm7core_option_change();
					pu_paint_led_lines(pu_option_otkluchenie_lines, LED_COLOR_OPTION_OTKLUCHENIE);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_DEJURNIY:
					/* manual (panel) fire activation is cancelled here */
					fire_lines_manual = 0;
					pu_set_mode(PU_MODE_DEJURNIY);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_ZVUKOVOE_VESHANIE:
					if (safety_get_level_from_pin_code() < 2) { //Режим: Звук. Вещ. (уровень доступа 2,3,4)
						pu_invalid_access_code(CONM"screen1: error MODE_ZVUKOVOE_VESHANIE by user level"CRLF, 2);
						return;
					}
					pu_set_mode(PU_MODE_ZVUKOVOE_VESHANIE);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_VESHANIE_POJAR:
					if (safety_get_level_from_pin_code() < 2) { //Режим: Вещ. Пожар (уровень доступа 2,3,4)
						pu_invalid_access_code(CONM"screen1: error MODE_VESHANIE_POJAR by user level"CRLF, 2);
						return;
					}
					fire_lines_manual = 0xFFF;   /* manual (panel) activation */
					pu_set_mode(PU_MODE_VESHANIE_POJAR);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_VESHANIE_MCHS:
					if (safety_get_level_from_pin_code() < 2) { //Режим: Вещ. Пожар (уровень доступа 2,3,4)
						pu_invalid_access_code(CONM"screen1: error MODE_VESHANIE_MCHS by user level"CRLF, 2);
						return;
					}
					pu_set_mode(PU_MODE_VESHANIE_MCHS);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_RUCHNOI_PUSK:
					if (safety_get_level_from_pin_code() < 2) { //Режим: Вещ. Пожар (уровень доступа 2,3,4)
						pu_invalid_access_code(CONM"screen1: error MODE_RUCHNOI_PUSK by user level"CRLF, 2);
						return;
					}
					pu_set_mode(PU_MODE_RUCHNOI_PUSK);
					screens_goto(0);
				break;
				case SCREEN1_ITEM_PU_MODE_SETEVOE_UPRAVLENIE:
					if (safety_get_level_from_pin_code() < 2) { //
						pu_invalid_access_code(CONM"screen1: error MODE_SETEVOE_UPRAVLENIE by user level"CRLF, 2);
						return;
					}
					pu_set_mode(PU_MODE_UPRAVLENIE_DRUGIMI_PU);
				break;
				case SCREEN1_ITEM_FAULTS:
					screen2_init();
					screens_goto(2);
				break;
				case SCREEN1_ITEM_PIN_CODE:
					screen3_init();
					screens_goto(3);
				break;
				default: break;
			}

		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
		case MKI_KEY_EXIT:
			screens_goto(0);
		break;
	}
}

void screen1_button_unpress(uint8_t key)
{
}
