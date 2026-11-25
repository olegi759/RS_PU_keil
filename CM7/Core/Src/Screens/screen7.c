/*
 * screen7.c
 *
 *  Created on: 7 июл. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "screen7.h"
#include "screens.h"
#include "str_util.h"
#include "mki_data.h"
#include "stm_util.h"
#include "display.h"
#include "cm7core.h"
#include "shared_memory.h"
#include "common.h"

//--------------- СООБЩЕНИЕ --------------------

static char screen7_msg[81 * 2];
static uint8_t screen7_return_screen_number;

void screen7_init(char * msg, uint8_t return_screen_number)
{
	strcpy_s(screen7_msg, sizeof(screen7_msg), msg);
	screen7_msg[sizeof(screen7_msg) - 1] = '\0';
	screen7_return_screen_number = return_screen_number;
}

void screen7_paint(void)
{
	set_display_all(screen7_msg);
	send_display();
}

void screen7_button_press(uint8_t key)
{
	switch (key) {
		case MKI_KEY_RESET:
		case MKI_KEY_ENTER:
			 screens_goto(screen7_return_screen_number);
		break;
	}
}

void screen7_button_unpress(uint8_t key)
{

}
