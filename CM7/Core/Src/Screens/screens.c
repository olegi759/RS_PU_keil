/*
 * screens.c
 *
 *  Created on: 9 апр. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "screens.h"
#include "screen0.h"
#include "screen1.h"
#include "screen2.h"
#include "screen3.h"
#include "screen4.h"
#include "screen5.h"
#include "screen6.h"
#include "screen7.h"
#include "stm_util.h"
#include "str_util.h"
#include "pu.h"
#include "pd_data.h"
#include "mchs_mic.h"
#include "uarts.h"
#include "dynbuf_util.h"
#include "mki_data.h"
#include "cm7core.h"
#include "beep.h"
#include "mux_sound.h"
#include "common.h"


static uint8_t screen_now = 0;
extern BOOL mode_rucnoi_pusk_to_dejurniy_blocked ;


uint8_t screens_now(void)
{
	return screen_now;
}

void screens_update(void)
{
	switch (screen_now) {
		case 0: screen0_paint(); break;
		case 1: screen1_paint(); break;
		case 2: screen2_paint(); break;
		case 3: screen3_paint(); break;
		case 4: screen4_paint(); break;
		case 5: screen5_paint(); break;
		case 6: screen6_paint(); break;
		case 7: screen7_paint(); break;
		default: break;
	}
}


void screens_goto(uint8_t screen_new)
{
	if (screen_now != screen_new) ulog_fmt("screens: goto screen %d->%d"CRLF, screen_now, screen_new);
	screen_now = screen_new;
	screens_update();
}

//static int her = 0;

void screens_button_press(uint8_t key)
{


	//if (is_set_setting_pin_code()) { //если есть хоть один ПИН

		//if (safety_get_level_from_pin_code() == 0) {
			//if (screen_now != 3) {
				//screen3_init();
				//screens_goto(3);
				//return;
			//}
		//}
	//}

	if (key == MKI_KEY_RESET) {

		if (safety_get_level_from_pin_code() < 2) {
			pu_invalid_access_code(CONM"screens: forbidden - MKI_KEY_RESET by user level"CRLF, 2);
			return;
		}
			if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
				pu_option = PU_OPTION_NONE;
				cm7core_option_change();
				pu_option_avtomatika_otkluchena_lines = 0;
				cm7core_set_led_color(MKI_LED_AUTOMATIONOFF, LED_COLOR_BLACK);
				pu_paint_led_lines(pu_option_avtomatika_otkluchena_lines, LED_COLOR_BLACK);
				screen0_update();
				return;
			}
			else if (pu_option == PU_OPTION_OTKLUCHENIE) {
				pu_option = PU_OPTION_NONE;
				cm7core_option_change();
				pu_option_otkluchenie_lines = 0;
				cm7core_set_led_color(MKI_LED_DISCONNECT, LED_COLOR_BLACK);
				pu_paint_led_lines(pu_option_otkluchenie_lines, LED_COLOR_BLACK);
				screen0_update();
				return;
			}
			else {
				if (pu_mode != PU_MODE_DEJURNIY) {
					mode_rucnoi_pusk_to_dejurniy_blocked = FALSE;
					pu_set_mode(PU_MODE_DEJURNIY);
					screen0_update();
					return;
				}
			}
	}

	//if (key == MKI_KEY_EXIT) {
	//	screen0_update();
	//	return;
	//}

	if (key == MKI_KEY_TEST) {
		//РАЗРЕМАРИТЬ ПОТОМ!!!!
		pu_test_enable_tick = HAL_GetTickNoZero();
		beep_on(BEEP_TIMEOUT_UNLIMIT);
		cm7core_mki_test(1);
		ulog("screens: pu test enable"CRLF);


/*
#include "strlist_util.h"
		char buf[MAX_STR_SIZE];
		cm7core_get_opus_list(buf, sizeof(buf));
		int item_len;
		int cnt = strlist_get_items_count_from_str(buf);
		her++;
		if (her >= cnt) her = 0;
		char * ptr = strlist_get_item_from_str(buf, her, &item_len);
		if (ptr) {
			ptr[item_len] = '\0';

			pu_enable_sound_lines(0x00FFFFFF);
			pu_play_audio_file(ptr);
		}
*/
		return;
	}


	if (key == MKI_KEY_MUTE) {
		MUTE_TOGGLE;
		cm7core_send_one_led_color(MKI_LED_MUTE,  MUTE_IS_ON ? LED_COLOR_YELLOW : LED_COLOR_BLACK);
		ulog_fmt(CONM"MUTE %s"CRLF, (MUTE_IS_ON ? "ON" : "OFF"));
		return;

	}

	if (key == MKI_KEY_MENU) {
		if (screen_now == 1) {
			screens_goto(0);
		} else {
			screen1_init();
			screens_goto(1);
		}
		return;
	}

	if (key == MKI_KEY_MANUALSTART) {

		if (safety_get_level_from_pin_code() < 2) { //Режим: Руч. Пуск (уровень доступа 2,3,4)
			pu_invalid_access_code(CONM"screen1: error - MODE_RUCHNOI_PUSK by user level"CRLF, 2);
			return;
		}

		pu_set_mode(PU_MODE_RUCHNOI_PUSK);
		return;
	}


	switch (screen_now) {
		case 0: screen0_button_press(key); break;
		case 1: screen1_button_press(key); break;
		case 2: screen2_button_press(key); break;
		case 3: screen3_button_press(key); break;
		case 4: screen4_button_press(key); break;
		case 5: screen5_button_press(key); break;
		case 6: screen6_button_press(key); break;
		case 7: screen7_button_press(key); break;
		default: break;
	}

}


void screens_button_unpress(uint8_t key)
{

	switch (screen_now) {
		case 0: screen0_button_unpress(key); break;
		case 1: screen1_button_unpress(key); break;
		case 2: screen2_button_unpress(key); break;
		case 3: screen3_button_unpress(key); break;
		case 4: screen4_button_unpress(key); break;
		case 5: screen5_button_unpress(key); break;
		case 6: screen6_button_unpress(key); break;
		case 7: screen7_button_unpress(key); break;
		default: break;
	}
}


void screen0_update(void)
{
	if (screen_now == 0) {
		screen0_paint();
	}
}


