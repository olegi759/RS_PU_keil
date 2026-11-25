/*
 * screen3.h
 *
 *  Created on: 20 авг. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SCREENS_SCREEN4_H_
#define CORE_INC_SCREENS_SCREEN4_H_

#include "main.h"

void screen4_init(void);
void screen4_paint(void);
void screen4_button_press(uint8_t key);
void screen4_button_unpress(uint8_t key);

extern char setevoe_upravlenie_addr_pu[4];

#endif /* CORE_INC_SCREENS_SCREEN3_H_ */
