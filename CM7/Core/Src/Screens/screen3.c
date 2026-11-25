/*
 * screen3.c
 *
 *  Created on: 20 авг. 2025 г.
 *      Author: Rafael Boltachev
 */


#include "screen3.h"
#include "screens.h"
#include "str_util.h"
#include "mki_data.h"
#include "stm_util.h"
#include "display.h"
#include "cm7core.h"
#include "shared_memory.h"
#include "common.h"
#include "screen3.h"

//--------------- ВВОД ПИН КОДА --------------------

//Тайминг ввода пин кода
uint32_t screen3_pin_code_enter_tick = 0;
static char screen3_safety_pin_code[sizeof(safety_pin_code)] = {'\0'};

void screen3_paint(void)
{
	clear_display();
	set_display(0, 0, "ВВЕДИТЕ PIN КОД");
	set_display(2, 0, screen3_safety_pin_code);
	send_display();
}

void screen3_init(void)
{
	memset(screen3_safety_pin_code, 0, sizeof(screen3_safety_pin_code));
	//memset(safety_pin_code, 0, sizeof(safety_pin_code));
	//cm7core_send_to_CM4_cmd(CORE_CMD_SAFETY_PINCODE_CHANGE, (uint8_t *)safety_pin_code, strlen(safety_pin_code));
}

void screen3_button_press(uint8_t key)
{
	switch (key) {

		case	MKI_KEY_1:
		case	MKI_KEY_2:
		case	MKI_KEY_3:
		case	MKI_KEY_4:
		case	MKI_KEY_5:
		case	MKI_KEY_6:
		case	MKI_KEY_7:
		case	MKI_KEY_8:
		case	MKI_KEY_9:
		case	MKI_KEY_10:
			if (key == MKI_KEY_10) strcat_s(screen3_safety_pin_code, sizeof(screen3_safety_pin_code), "0");
			else strcat_s(screen3_safety_pin_code, sizeof(screen3_safety_pin_code), int2str(key));
			screen3_paint();
		break;
		case MKI_KEY_DELETE:
			if (strlen(screen3_safety_pin_code)) {
			 	safety_pin_code[strlen(screen3_safety_pin_code) - 1] = '\0';
			 	screen3_paint();
			}
		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
			//screen3_init();
			//screen3_pin_code_enter_tick = HAL_GetTickNoZero();
			screens_goto(0);
		break;
		case MKI_KEY_ENTER:
			strcpy_s(safety_pin_code, sizeof(safety_pin_code), screen3_safety_pin_code);
			screen3_pin_code_enter_tick = HAL_GetTickNoZero();
			cm7core_send_to_CM4_cmd(CORE_CMD_CHANGE_SAFETY_PINCODE_CM4, (uint8_t *)safety_pin_code, strlen(safety_pin_code));
			screens_goto(0);
			if (safety_get_name_from_pin_code()) filelog_fmt("Пользователь '%s' открыл сессию", safety_get_name_from_pin_code());
			//else filelog_fmt("Пользователь вышел из сессии");
			ulog_fmt(CONM"safety_get_level_from_pin_code=%d name=%s"CRLF, safety_get_level_from_pin_code(), safety_get_name_from_pin_code());

		break;
	}
}

void screen3_button_unpress(uint8_t key)
{

}


