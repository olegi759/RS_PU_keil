/*
 * screen2.c
 *
 *  Created on: 11 апр. 2025 г.
 *      Author: Rafael Boltachev
 */


#include "screen2.h"
#include <string.h>
#include <stdio.h>
#include "str_util.h"
#include "mki_data.h"
#include "pu.h"
#include "mki_data.h"
#include "screens.h"
#include "fault_data.h"
#include "mkk_data.h"
#include "display.h"
#include "cm7core.h"
#include "common.h"
#include "stm_util.h"

//--------------- НЕИСПРАВНОСТИ --------------------



static uint8_t screen2_index = 0;

static char screen2_str_buf[128] = {0};


const char * screen2_get_item(uint8_t index)
{
	uint8_t i, cnt;

	cnt = 0;
	for (i = 0; i < 128; i++) {
		if (fault_is_set(i)) {
			if (index == cnt) {

				snprintf(screen2_str_buf, sizeof(screen2_str_buf), "%d) %s", index + 1, fault2str(i));

				if ((i >= FAULT_MKK_INSIG_1) && (i <= FAULT_MKK_INSIG_24)) {
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), " ");
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), cm7_mkk_get_insig_state_str(i - FAULT_MKK_INSIG_1));
				}
				else if ((i >= FAULT_MKK_SOUND_1) && (i <= FAULT_MKK_SOUND_24)) {
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), " ");
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), cm7_mkk_get_lin_state_str(i - FAULT_MKK_SOUND_1));
				}
				else if ((i >= FAULT_MKK_START_1) && (i <= FAULT_MKK_START_2)) {
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), " ");
					strcat_s(screen2_str_buf, sizeof(screen2_str_buf), cm7_mkk_get_start_state_str(i - FAULT_MKK_START_1));
				}

				return screen2_str_buf;

			}

			cnt++;
		}

	}
	return NULL;
}


void screen2_paint(void)
{
	clear_display();

	if (fault_count()) {
		set_display_all(screen2_get_item(screen2_index));
	} else {
		set_display(0, 0, "НЕИСПРАВНОСТИ");
		set_display(1, 0, "ОТСУТСТВУЮТ");
	}

	//ulog("300"CRLF);
	send_display();

}

void screen2_init(void)
{
	screen2_index = 0;
}

void screen2_button_press(uint8_t key)
{
	switch (key) {
		case MKI_KEY_UP:
			if (screen2_index) {
				screen2_index--;
				screen2_paint();
				ulog_fmt(CONM"screen2: menu index=%d(%s)"CRLF, screen2_index, screen2_get_item(screen2_index));
			}
		break;
		case MKI_KEY_DOWN:
			if ((int)screen2_index < (int)fault_count() - 1) {
				screen2_index++;
				screen2_paint();
				ulog_fmt(CONM"screen2: menu index=%d(%s)"CRLF, screen2_index, screen2_get_item(screen2_index));
			}
		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
			screens_goto(1);
		break;
	}
}

void screen2_button_unpress(uint8_t key)
{

}
