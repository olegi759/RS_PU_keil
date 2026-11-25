/*
 * screen1.h
 *
 *  Created on: 9 апр. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREEN1_H_
#define CORE_INC_SCREENS_SCREEN1_H_

#include "main.h"

void screen1_init(void);
void screen1_paint(void);
void screen1_button_press(uint8_t key);
void screen1_button_unpress(uint8_t key);

#endif /* CORE_INC_SCREENS_SCREEN1_H_ */
