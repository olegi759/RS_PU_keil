/*
 * screen3.h
 *
 *  Created on: 20 авг. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREEN3_H_
#define CORE_INC_SCREENS_SCREEN3_H_

#include "main.h"

void screen3_init(void);
void screen3_paint(void);
void screen3_button_press(uint8_t key);
void screen3_button_unpress(uint8_t key);

extern uint32_t screen3_pin_code_enter_tick;




#endif /* CORE_INC_SCREENS_SCREEN3_H_ */
