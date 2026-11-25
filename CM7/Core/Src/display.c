/*
 * display.c
 *
 *  Created on: 30 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "display.h"
#include <string.h>
#include "cm7core.h"


//4 строки дисплея для МКИ
static char display_lines[4][20] = { 0 };

const char hd44780_utf_recode[] = {
0x41, 0xa0, 0x42, 0xa1, 0xe0, 0x45, 0xa3, 0xa4, // A-YA ru
0xa5, 0xa6, 0x4b, 0xa7, 0x4d, 0x48, 0x4f, 0xa8,
0x50, 0x43, 0x54, 0xa9, 0xaa, 0x58, 0xe1, 0xab,
0xac, 0xe2, 0xad, 0xae, 0x62, 0xaf, 0xb0, 0xb1,
0x61, 0xb2, 0xb3, 0xb4, 0xe3, 0x65, 0xb6, 0xb7, // a - ya ru
0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0x6f, 0xbe,
0x70, 0x63, 0xbf, 0x79, 0xe4, 0x78, 0xe5, 0xc0,
0xc1, 0xe6, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7
};
//В кодировку дисплея из UTF8
uint8_t hd44780_from_utf(char *dest, uint8_t dest_size, const char *utf8_str)
{
	uint8_t c, prv = 0, cnt = 0;
	char *dst = dest;

	while ((c = *utf8_str)) {
		if (prv == 0xd0) {
			if (c == 0x81)
				c = 0xa2;
			else if (c >= 0x90 && c <= 0xbf)
				c = hd44780_utf_recode[c - 0x90];
			else
				c = 0x3f; // ?
		} else if (prv == 0xd1) {
			if (c == 0x91)
				c = 0xb5;
			else if (c >= 0x80 && c <= 0x8f)
				c = hd44780_utf_recode[c - 0x50];
			else
				c = 0x3f; // ?
		}
		prv = *utf8_str++;
		if (prv != 0xd0 && prv != 0xd1) {
			*dst++ = c;
			cnt++;
			if (cnt >= dest_size) break;
		}
	}
	*dst = 0x00;
	return cnt;
}


//очистить дисплей
void clear_display(void)
{
	memset(display_lines, ' ', sizeof(display_lines));
}

//установить строку в позицию row col
void set_display(uint8_t row, uint8_t col, const char * utf8_str)
{
	char hd44780_str[21];
	uint8_t cnt, c, i;

	if (!utf8_str || !strlen(utf8_str)) return;

	cnt = hd44780_from_utf(hd44780_str, sizeof(hd44780_str), utf8_str);

	i = 0;
	c = col;
	while ((c < 20) && (i < cnt)) {
		display_lines[row][c] = hd44780_str[i];
		i++;
		c++;
	}

}

//Установить текст в массив display_lines из строки hd44780_str в кодировке дисплея
void set_display_all_hd44780(const char * hd44780_str)
{
	memcpy(display_lines[0], &hd44780_str[0], 20 );
	memcpy(display_lines[1], &hd44780_str[20], 20);
	memcpy(display_lines[2], &hd44780_str[40], 20);
	memcpy(display_lines[3], &hd44780_str[60], 20);

}


//Установить текст в массив display_lines из строки utf8_str
void set_display_all(const char * utf8_str)
{
	char hd44780_str[81];
	int i;

	if (!utf8_str || !strlen(utf8_str)) return;


	memset(hd44780_str, 0, sizeof(hd44780_str));
	hd44780_from_utf(hd44780_str, sizeof(hd44780_str), utf8_str);
	for (i = 0; i < 81; ++i) {
		if (hd44780_str[i] == '\0') hd44780_str[i] = ' ';
	}

	set_display_all_hd44780((const char *)hd44780_str);
}


void send_display(void)
{
	cm7core_mki_set_display((char *)display_lines);
}

