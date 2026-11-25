/*
 * screen7.h
 *
 *  Created on: 7 июл. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_SCREENS_SCREEN7_H_
#define CM7_CORE_INC_SCREENS_SCREEN7_H_

#include "main.h"

void screen7_init(char * msg, uint8_t return_screen_number);
void screen7_paint(void);
void screen7_button_press(uint8_t key);
void screen7_button_unpress(uint8_t key);



#endif /* CM7_CORE_INC_SCREENS_SCREEN7_H_ */
