/*
 * screen5.h
 *
 *  Created on: 16 мар. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREEN5_H_
#define CORE_INC_SCREENS_SCREEN5_H_

#include "main.h"

void screen5_init(BOOL only_opus);
void screen5_paint(void);
void screen5_button_press(uint8_t key);
void screen5_button_unpress(uint8_t key);

//extern char screen5_selected_file[64];

#endif /* CORE_INC_SCREENS_SCREEN5_H_ */
