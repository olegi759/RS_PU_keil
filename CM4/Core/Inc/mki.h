/*
 * mdk.h
 *
 *  Created on: 14 мар. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MKI_H_
#define CORE_INC_MKI_H_

#include "main.h"
#include "mki_data.h"


void mki_send_key(void);

//void mki_send_many_leds(mki_led_param_t * mki_led_param, uint8_t count);
//void mki_send_one_led(uint8_t number, uint32_t color);
//void mki_send_leds(void);
//void mki_set_led(uint8_t number, uint32_t color);
//uint32_t mki_get_led(uint8_t number);
//void mki_send_leds_by_arr(uint32_t * arr);
void mki_send_led_clear(void);

void mki_send_version(void);
void mki_send_reboot(void);


//void mki_set_display(uint8_t row, uint8_t col, const char * utf8_str);
//void mki_send_display(void);
//void mki_clear_display(void);
//void mki_set_display_all(const char * hd44780_str);
//void mki_set_display_all_utf8(const char * utf8_str);

void mki_send_display_hd44780(const char * hd44780_str);


void mki_rcv(uint16_t * data, uint16_t data_len);
void mki_init(void);

uint8_t hd44780_from_utf(char *dest, uint8_t dest_size, const char *utf8_str);


void mki_press(uint8_t key);
void mki_unpress(uint8_t key);

void mki_send_test(uint8_t enable);

void mki_set_small_led(uint8_t number, uint8_t color);
void mki_send_small_leds(void);
void mki_send_one_small_led(uint8_t number, uint8_t color);
void mki_enable_led_from_lines(uint32_t lines, uint8_t color);

void mki_int_change(void);


#define MKI_INT_BUTTON_DEFAULT 	0
#define MKI_INT_BUTTON_PRESS 	1
#define MKI_INT_BUTTON_UNPRESS 	2

//extern BOOL mki_int_button_state;
extern BOOL mki_int_p;

#endif /* CORE_INC_MKI_H_ */
