/*
 * screen5.c
 *
 *  Created on: 16 мар. 2026 г.
 *      Author: Rafael Boltachev
 */


#include "screen5.h"
#include "screens.h"
#include "fatfs_util.h"
#include "str_util.h"
#include "stm_util.h"
#include "mki_data.h"
//#include "external_flash.h"
#include "dynbuf_util.h"
#include "display.h"
#include "cm7core.h"
#include "shared_memory.h"
#include "sendopus.h"
#include "pu.h"
#include "screen6.h"
#include "uarts.h"

//--------------- ВЫБОР ИЗ ФЙЛОВ OPUS --------------------

static int screen5_index = 0;

static dynamic_buf_t screen5_dyn_buf = DYNAMIC_BUF_STATIC_INIT(16 * 1024);

static BOOL screen5_only_opus = FALSE;
//char screen5_selected_file[64] = {'\0'};

void screen5_get_item(uint8_t index, char * buf, uint16_t buf_size)
{
	size_t data_len;
	char * ptr;


	buf[0] = '\0';
	ptr = dynamic_buf_get_at(&screen5_dyn_buf, index, &data_len);
	if (!ptr) return;

	strcpy_s(buf, buf_size, index == screen5_index ? ">" : " ");
	strncat_s(buf, buf_size, ptr, data_len);
}


void screen5_paint(void)
{

	char buf[MAX_STR_SIZE];
	int start_index;


	clear_display();

	start_index = screen5_index - 3;
	if (start_index < 0) start_index = 0;

	screen5_get_item(start_index,  buf, sizeof(buf));
	set_display(0, 0, buf);
	screen5_get_item(start_index + 1, buf, sizeof(buf));
	set_display(1, 0, buf);
	screen5_get_item(start_index + 2, buf, sizeof(buf));
	set_display(2, 0, buf);
	screen5_get_item(start_index + 3, buf, sizeof(buf));
	set_display(3, 0, buf);

	send_display();

}



void screen5_init(BOOL only_opus)
{
	char buf[SM_MAX_PACKET_SIZE];
	char * ptr, * ptr2;
	uint16_t rc;
	int len;

	screen5_only_opus = only_opus;

	screen5_index = 0;
	dynamic_buf_clear(&screen5_dyn_buf);

	buf[0] = '\0';
	if (screen5_only_opus) {
		rc = cm7core_get_opus_list(buf, sizeof(buf));
	} else {
		rc = cm7core_get_mp3_and_opus_list(buf, sizeof(buf));

	}
	if (!rc) return;
	buf[rc] = 0;

	ptr = buf;
	while ( (ptr2 = strright(ptr, strlen(ptr), CRLF)) != NULL) {
		len = ((ptr2 - ptr) - strlen(CRLF));
		dynamic_buf_push(&screen5_dyn_buf, ptr, len);
		ptr = ptr2;
	}
}



void screen5_button_press(uint8_t key)
{
	size_t data_len;
	char * ptr;
	int i;
	uint32_t lines;
	char buf[MAX_STR_SIZE];

	switch (key) {
		case MKI_KEY_UP:
			if (screen5_index) {
				screen5_index--;
				screen5_paint();
			}
			screen5_get_item(screen5_index, buf, sizeof(buf));
			ulog_fmt("screen5: index=%d, item=%s"CRLF, screen5_index, buf);
		break;
		case MKI_KEY_DOWN:
			if (screen5_index < screen5_dyn_buf.count - 1) {
				screen5_index++;
				screen5_paint();
			}
			screen5_get_item(screen5_index, buf, sizeof(buf));
			ulog_fmt("screen5: index=%d item=%s"CRLF, screen5_index, buf);
		break;
		case MKI_KEY_ENTER:
			ptr = dynamic_buf_get_at(&screen5_dyn_buf, screen5_index, &data_len);
			//ulog_fmt("screen5: select=%s"CRLF, (ptr ? ptr : "NULL"));
			buf[0] = '\0';
			if (ptr) strncpy_s(buf, sizeof(buf), ptr, data_len);
			dynamic_buf_clear(&screen5_dyn_buf);

			if (buf[0] == '\0') {
				pu_set_mode(PU_MODE_DEJURNIY);
				screens_goto(0);
				break;
			}

			ulog_fmt("screen5: select file=%s"CRLF, buf);

			//strcpy_s(screen5_selected_file, sizeof(screen5_selected_file), buf);

			if (screen5_only_opus) {

				//запуск трансляции с файла
				if (!sendopus_start((const char *)buf,  0xFF)) {
					pu_set_mode(PU_MODE_DEJURNIY);
					screens_goto(0);
					return;
				}

				for (i = 1; i <= 25; i++) {
					if (!screen6_lines[i]) continue;

					if (i * 10 == setting.uart_local_addr) {
						pu_enable_sound_lines(screen6_lines[i]);
						continue;
					}

					uarts_transmit_veshanie_pd(i * 10, sendopus_samplerate, sendopus_channels == 2, screen6_lines[i]);
					HAL_Delay(50);
				}
			} else {
				lines = pu_get_manual_lines_from_leds();
				pu_enable_sound_lines(lines);

				if (setting.mode_manual_tilibom_file) {
					tilibom_play(buf);
				} else {
					pu_play_audio_file((const char *)buf);
				}

			}
			screens_goto(0);
			filelog_fmt("Плеер");
		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
			ulog("screen5: cancel"CRLF);
			dynamic_buf_clear(&screen5_dyn_buf);
			pu_set_mode(PU_MODE_DEJURNIY);
			screens_goto(0);
		break;
	}
}

void screen5_button_unpress(uint8_t key)
{
}
