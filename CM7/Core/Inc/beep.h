/*
 * beep.h
 *
 *  Created on: 28 июл. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_BEEP_H_
#define CORE_INC_BEEP_H_

#include "main.h"

#define BEEP_TIMEOUT_UNLIMIT	0

void beep_on(uint32_t timeout);
void beep_off(void);
void beep_task(void);


#define SMALL_BEEP		beep_on(500)


#endif /* CORE_INC_BEEP_H_ */
