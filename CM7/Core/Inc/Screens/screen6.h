/*
 * screen6.h
 *
 *  Created on: 17 мар. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREEN6_H_
#define CORE_INC_SCREENS_SCREEN6_H_


#include "main.h"

void screen6_init(uint8_t * pu_addrs, uint8_t pu_addrs_cnt);
void screen6_paint(void);
void screen6_button_press(uint8_t key);
void screen6_button_unpress(uint8_t key);


extern uint32_t screen6_lines[26];
#endif /* CORE_INC_SCREENS_SCREEN6_H_ */
