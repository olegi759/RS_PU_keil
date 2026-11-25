/*
 * mdk.c
 *
 *  Created on: 14 РјР°СЂ. 2024 Рі.
 *      Author: Rafael Boltachev
 */

#include "mki.h"

#include <stdio.h>
#include "str_util.h"
#include "stm_util.h"
#include "uart2.h"
#include "utf8_util.h"
#include "math_util.h"
#include "mkk.h"
#include "cm4core.h"
#include "common.h"
#include "fault.h"
#include "fault_data.h"


//РїРѕСЃР»РµРґРЅСЏСЏ РЅР°Р¶Р°С‚Р°СЏ РєР»Р°РІРёС€Р° РЅР° РњРљР
static uint8_t mki_old_key = 0;
//4 СЃС‚СЂРѕРєРё РґРёСЃРїР»РµСЏ РґР»СЏ РњРљР
//static char display_lines[4][20] = { 0 };

//static uint32_t mki_leds_color_need[35] = { 0 }; //РјР°СЃСЃРёРІ С†РІРµС‚РѕРІ СЃРІРµС‚РѕРґРёРѕРґРѕРІ РЅРµРѕР±С…РѕРґРёРјС‹Р№
//static uint32_t mki_leds_color_now[35] = { 0 }; //РјР°СЃСЃРёРІ С†РІРµС‚РѕРІ СЃРІРµС‚РѕРґРёРѕРґРѕРІ С‚РµРєСѓС‰РёР№

static uint16_t mki_tx_buf[MAX_UART2_PACKET_SIZE];

//BOOL mki_int_button_state = MKI_INT_BUTTON_DEFAULT;

BOOL mki_int_p = FALSE;




const uint16_t mki_cmd_version[3] =  { 0x100 | MKI_CMD_VERSION, 0x00, MKI_CMD_VERSION };


//РџРѕСЃР»Р°С‚СЊ Р·Р°РїСЂРѕСЃ РІРµСЂСЃРёРё
void mki_send_version(void)
{
	uart2_transmit(UART2_MUX_MKI, (uint16_t *)mki_cmd_version, 3);
}

const uint16_t mki_cmd_reboot[3] =  { 0x100 | MKI_CMD_REBOOT, 0x00, MKI_CMD_REBOOT };

//РџРѕСЃР»Р°С‚СЊ РїРµСЂРµР·Р°РіСЂСѓР·РєР°
void mki_send_reboot(void)
{
	uart2_transmit(UART2_MUX_MKI, (uint16_t *)mki_cmd_reboot, 3);
}


const uint16_t mki_cmd_key[3] =  { 0x100 | MKI_CMD_KEY, 0x00, MKI_CMD_KEY };

//РџРѕСЃР»Р°С‚СЊ Р·Р°РїСЂРѕСЃ РЅР°Р¶Р°С‚РѕР№ РєРЅРѕРїРєРё
void mki_send_key(void)
{
	uart2_transmit(UART2_MUX_MKI, (uint16_t *)mki_cmd_key, 3);
}


const uint16_t mki_cmd_clear[3] =  { 0x100 | MKI_CMD_LED_CLEAR, 0x00, MKI_CMD_LED_CLEAR };

//РџРѕСЃР»Р°С‚СЊ РІС‹РєР»СЋС‡РёС‚СЊ РІСЃРµ СЃРІРµС‚РѕРґРёРѕРґС‹
void mki_send_led_clear(void)
{
	uart2_transmit(UART2_MUX_MKI, (uint16_t *)mki_cmd_clear, 3);
}



void mki_send_cmd_uint8(uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uint16_t * buf, crc;
	int i;

	buf = mki_tx_buf;

	buf[0] = (0x0100 | cmd);
	crc = cmd;

	i = 0;
	if (cmd_data && cmd_data_len) {
		while (i < cmd_data_len) {
			buf[1 + i] = cmd_data[i];
			crc += cmd_data[i];
			i++;
		}
	}
	i++;
	buf[i] = crc >> 8;
	i++;
	buf[i] = crc & 0xFF;
	i++;

	uart2_transmit(UART2_MUX_MKI, buf, i);
}


//РџРѕСЃР»Р°С‚СЊ С‚РµРєСЃС‚ РЅР° РґРёСЃРїР»РµР№ РёР· hd44780_str
void mki_send_display_hd44780(const char * hd44780_str)
{
	mki_send_cmd_uint8(MKI_CMD_DISPLAY, (uint8_t * )hd44780_str, 80);
	//ulog_fmt(CONM"disp %lu"CRLF, HAL_GetTick());
}

//РІРєР»СЋС‡РёС‚СЊ/РІС‹РєР»СЋС‡РёС‚СЊ РўРµСЃС‚ РЅР° РњРљР
void mki_send_test(uint8_t enable)
{
	mki_send_cmd_uint8(MKI_CMD_TEST, &enable, 1);
}



//------------------------------- LED BEGIN------------------------------------

static uint8_t led_color[17] = {0};

//РїРѕСЃР»Р°С‚СЊ РЅР°Р° СЃРІРµС‚РѕРґРёРѕРґС‹
void mki_send_small_leds(void)
{
	mki_send_cmd_uint8(MKI_CMD_SMALL_LEDS, led_color, 17);
}


void mki_set_small_led(uint8_t number, uint8_t color)
{
	uint8_t index;

	if (number > 34) return;

	index = (number - 1) / 2;

	if (ODD(number)) {
		led_color[index] = (led_color[index] & 0x0F) | (color << 4);
	} else {
		led_color[index] = (led_color[index] & 0xF0) | color;
	}
}

void mki_send_one_small_led(uint8_t number, uint8_t color)
{
	mki_set_small_led(number, color);
	mki_send_small_leds();
}



void mki_enable_led_from_lines(uint32_t lines, uint8_t color)
{
	int i;

	for (i = 0; i < 24; i++) {
		 mki_set_small_led(i + 1, GETBIT(lines, i) ? color : LED_COLOR_BLACK);
	}
	mki_send_small_leds();
}


//------------------------------- LED  END ------------------------------------

//РћР±СЂР°Р±РѕС‚С‡РёРє РЅР° РЅР°Р¶Р°С‚РёРµ РєР»Р°РІРёС€С‹
void mki_press(uint8_t key)
{
	ulog_fmt(CONM"mki press %s(%lu)"CRLF, mki_get_key_name(key), (unsigned long)HAL_GetTick());
	cm4core_button_press(key);
}

//РћР±СЂР°Р±РѕС‚С‡РёРє РЅР° РѕС‚Р¶Р°С‚РёРµ РєР»Р°РІРёС€С‹
void mki_unpress(uint8_t key)
{
	ulog_fmt(CONM"mki unpress %s(%lu)"CRLF, mki_get_key_name(key), (unsigned long)HAL_GetTick());
	cm4core_button_unpress(key);
}



//РћР±СЂР°Р±РѕС‚С‡РёРє РЅР° РїСЂРёС…РѕРґ РґР°РЅРЅС‹С… РѕС‚ РњРљР
void mki_rcv(uint16_t * packet, uint16_t packet_len)
{
	uint16_t rcrc, scrc, i;

	if (!(packet[0] & 0x0100)) return;
	packet[0] = packet[0] & 0xFF;


	rcrc = (packet[packet_len - 2] << 8) | packet[packet_len - 1];
	scrc = 0;
	for (i = 0; i < packet_len - 2; ++i) {
		scrc += packet[i];
	}

	if (rcrc != scrc) {
		return;
	}


	//char str[200];
	//usmem2hexstr(packet, packet_len, str, sizeof(str), " ");
	//ulog_fmt("r2(%d) %s"CRLF , packet_len, str);

	fault_unset(FAULT_MKI_NOT_RECV);

	switch (packet[0]) {

		//case MKI_CMD_KEY:
		//	if (!mki_old_key && packet[1]) mki_press(packet[1]);
		//	else if (mki_old_key && !packet[1]) mki_unpress(mki_old_key);
		//	mki_old_key = packet[1];
		//break;

		case MKI_CMD_KEY:
			if (mki_old_key != packet[1]) { //РёР·РјРµРЅРёР»СЃСЏ
				if (mki_old_key) mki_unpress(mki_old_key);
				if (packet[1]) mki_press(packet[1]);
				mki_old_key = packet[1];
			}
			mki_int_p = FALSE;
		break;

		case MKI_CMD_VERSION:
			//11 00 31 00 2E 00 30 00 20 00 32 00 30 00 32 00 36 00 2D 00 30 00 36 00 2D 00
			//packet[packet_len] = 0;

			if (packet_len > 4) {
				for (i = 0; i < MIN(packet_len - 3, sizeof(mki_version) - 1) ; ++i) {
					mki_version[i] = packet[1 + i] & 0xFF;
				}
				ulog_fmt(CONM"MKI Version: %s"CRLF, mki_version);
			}

			//fault_unset(FAULT_MKI_NOT_RECV);

			//char str[200];
			//mem2hexstr(packet, packet_len, str, sizeof(str), " ");
			//ulog(str);
			//ulog(CRLF);

		break;
	}
}

void mki_int_change(void)
{
	mki_int_p = TRUE;


	/*
	if (PIN_GET(MKI_INT_P)) {
		mki_int_button_state = MKI_INT_BUTTON_UNPRESS;
		//fulog("1");
	} else {
		mki_int_button_state = MKI_INT_BUTTON_PRESS;
		//fulog("2");
	}
	mki_int_button_flag_fast = TRUE;
	*/
}


void mki_init(void)
{
	  mki_send_led_clear();
}



/*
static uint8_t led_color[17] = {0};

//РїРѕСЃР»Р°С‚СЊ РЅР°Р° СЃРІРµС‚РѕРґРёРѕРґС‹
void mki_send_small_leds(uint8_t * arr17)
{
	uint16_t crc;
	int i;
	uint16_t * tx;

	tx = mki_tx_buf;

	tx[0] = 0x100 | MKI_CMD_SMALL_LEDS;

	crc = MKI_CMD_SMALL_LEDS;


	for (i = 0; i < 17; ++i) {
		tx[i + 1] = arr17[i];
		crc += arr17[i];
	}

	tx[18] = crc >> 8;
	tx[19] = crc & 0xFF;

	uart2_transmit(UART2_MUX_MKI, tx, 20);

	//mki_send_cmd_uint8(MKI_CMD_SMALL_LEDS, arr17, 17);

	mki_snd_tick = HAL_GetTickNoZero();
}





void mki_set_small_led(uint8_t number, uint8_t color)
{
	uint8_t index;

	if (number > 34) return;

	index = (number - 1) / 2;

	if (ODD(number)) {
		led_color[index] = (led_color[index] & 0x0F) | (color << 4);
	} else {
		led_color[index] = (led_color[index] & 0xF0) | color;
	}
}

void mki_send_all_small_leds(void)
{
	 mki_send_small_leds(led_color);
	 cm4core_send_to_CM7_cmd(CORE_CMD_MKI_CHANGE_LEDS_COLOR, led_color, sizeof(led_color));

}



static BOOL led_color_change[35] = { FALSE };

static uint8_t led_color[35] = {0};

void mki_send_all_small_leds(void)
{
	uint8_t buf[255];
	int i, j;

	j = 0;
	for (i = 1; i < 35; i++) {
		if (led_color_change[i]) {
			led_color_change[i] = FALSE;
			buf[j] = i;
			j++;
			buf[j] = led_color[i];
			j++;
		}
	}

	if (!j) return;

	mki_send_cmd_uint8(MKI_CMD_SMALL_LED, buf, j);


	//cm4core_send_to_CM7_cmd(CORE_CMD_MKI_CHANGE_LEDS_COLOR, led_color, sizeof(led_color));

}

void mki_set_small_led(uint8_t number, uint8_t color)
{
	if (number > 34) return;

	//if (led_color[number] == color) return;

	led_color[number] = color;
	led_color_change[number] = TRUE;
}



 */
