/*
 * screen6.c
 *
 *  Created on: 17 мар. 2026 г.
 *      Author: Rafael Boltachev
 */


#include "screen6.h"
#include <stdio.h>
#include "screens.h"
#include "str_util.h"
#include "stm_util.h"
#include "mki_data.h"
#include "cm7core.h"
#include "math_util.h"
#include "display.h"
#include "pu.h"
#include "common.h"
#include "screen5.h"
#include "uarts.h"
#include "mchs_mic.h"

//--------------- ВЫБОР ИЗ СПИСКА ПУ --------------------

static int screen6_index = 1;
//линии 1...25 пу
uint32_t screen6_lines[26] = {0};
static uint8_t screen6_pu_addrs[26] = {0};
static uint8_t screen6_pu_addrs_cnt = 0;



void screen6_get_item(uint8_t index, char * buf, uint16_t buf_size)
{
	uint8_t pu_addr;
	uint8_t lines_index;

	buf[0] = '\0';
	if (index >= screen6_pu_addrs_cnt) return;

	pu_addr = screen6_pu_addrs[index];
	lines_index = pu_addr / 10;

	snprintf(buf, buf_size, "%sРС-ПУ АДР:%d ВЫБР.-%d", (index == screen6_index ? ">" : " "), pu_addr,  bitis1cnt(&screen6_lines[lines_index], 24));
}


void screen6_paint(void)
{

	char buf[MAX_STR_SIZE];
	int start_index;
	uint8_t pu_addr;
	uint8_t lines_index;


	clear_display();

	start_index = screen6_index - 3;
	if (start_index < 0) start_index = 0;

	screen6_get_item(start_index,  buf, sizeof(buf));
	set_display(0, 0, buf);
	screen6_get_item(start_index + 1, buf, sizeof(buf));
	set_display(1, 0, buf);
	screen6_get_item(start_index + 2, buf, sizeof(buf));
	set_display(2, 0, buf);
	screen6_get_item(start_index + 3, buf, sizeof(buf));
	set_display(3, 0, buf);

	send_display();
	HAL_Delay(50);

	pu_addr = screen6_pu_addrs[screen6_index];
	lines_index = pu_addr / 10;

	pu_paint_led_lines(screen6_lines[lines_index], pu_mode2ledcolor(pu_mode));

}





void screen6_init(uint8_t * pu_addrs, uint8_t pu_addrs_cnt)
{
	screen6_index = 0;
	memset(screen6_lines, 0, sizeof(screen6_lines));
	memcpy(screen6_pu_addrs, pu_addrs, pu_addrs_cnt);
	screen6_pu_addrs_cnt = pu_addrs_cnt;
	//pu_led_clear();
	pu_paint_led_lines(0, LED_COLOR_BLACK);

}



void screen6_button_press(uint8_t key)
{
	uint8_t pu_addr;
	uint8_t lines_index;
	int i;
	char buf[MAX_STR_SIZE];

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
		case	MKI_KEY_11:
		case	MKI_KEY_12:


		case	MKI_KEY_13:
		case	MKI_KEY_14:
		case	MKI_KEY_15:
		case	MKI_KEY_16:
		case	MKI_KEY_17:
		case	MKI_KEY_18:
		case	MKI_KEY_19:
		case	MKI_KEY_20:
		case	MKI_KEY_21:
		case	MKI_KEY_22:
		case	MKI_KEY_23:
		case	MKI_KEY_24:

			pu_addr = screen6_pu_addrs[screen6_index];
			lines_index = pu_addr / 10;

			if (bitget(&screen6_lines[lines_index], key - 1)) {
				bitset(&screen6_lines[lines_index], key - 1, 0);
			} else {
				bitset(&screen6_lines[lines_index], key - 1, 1);
			}

			screen6_paint();

		break;

		case MKI_KEY_UP:
			if (screen6_index > 0) {
				screen6_index--;
				screen6_paint();
			}
			screen6_get_item(screen6_index,  buf, sizeof(buf));
			ulog_fmt("screen6: index=%d(%s)"CRLF, screen6_index, buf);
		break;
		case MKI_KEY_DOWN:
			if (screen6_index < screen6_pu_addrs_cnt - 1) {
				screen6_index++;
				screen6_paint();
			}
			screen6_get_item(screen6_index,  buf, sizeof(buf));
			ulog_fmt("screen6: index=%d(%s)"CRLF, screen6_index, buf);
		break;

		case MKI_KEY_ENTER:
			ulog("screen6: enter select"CRLF);
			pu_paint_led_lines(0, LED_COLOR_BLACK);

			//запуск трансляции с файла
			screen5_init(TRUE);
			screens_goto(5);
		break;
		case MKI_KEY_MIC:
			ulog("screen6: mic select"CRLF);
			pu_paint_led_lines(0, LED_COLOR_BLACK);

			//запуск трансляции с микрофона
			for (i = 1; i <= 25; i++) {
				if (!screen6_lines[i]) continue;

				if (i * 10 == setting.uart_local_addr) {
					pu_enable_sound_lines(screen6_lines[i]);
					continue;
				}

				uarts_transmit_veshanie_pd(i * 10, MCHS_MIC_AUDIO_FREQUENCY, FALSE, screen6_lines[i]);
				HAL_Delay(50);
			}
			mchs_mic_start(TRUE, 0xFF);
			screens_goto(0);
			filelog_fmt("Микрофон");
		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
			ulog("screen6: reset cancel"CRLF);
			pu_paint_led_lines(0, LED_COLOR_BLACK);
			pu_set_mode(PU_MODE_DEJURNIY);
			screens_goto(0);
		break;
	}
}

void screen6_button_unpress(uint8_t key)
{
}
