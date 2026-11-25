/*
 * display.h
 *
 *  Created on: 30 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_DISPLAY_H_
#define CM7_CORE_INC_DISPLAY_H_

#include "main.h"

void clear_display(void);
void set_display(uint8_t row, uint8_t col, const char * utf8_str);
void set_display_all(const char * str);
void send_display(void);


#endif /* CM7_CORE_INC_DISPLAY_H_ */
