/*
 * screens.h
 *
 *  Created on: 9 апр. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREENS_H_
#define CORE_INC_SCREENS_SCREENS_H_


#include "main.h"

void screens_update(void);
void screens_goto(uint8_t screen_new);
void screens_button_press(uint8_t key);
void screens_button_unpress(uint8_t key);
uint8_t screens_now(void);


//uint8_t fault_count(void);
//BOOL fault_is_set(uint8_t num);

void screen0_update(void);

#endif /* CORE_INC_SCREENS_SCREENS_H_ */
