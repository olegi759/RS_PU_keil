/*
 * beep.c
 *
 *  Created on: 28 июл. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "beep.h"
#include "stm_util.h"
#include "pu.h"
#include "screens.h"

static BOOL beep_enable = FALSE;
static uint32_t beep_tick = 0;
static uint32_t beep_timeout = 0;
static uint32_t beep_start_tick = 0;


void beep_on(uint32_t timeout)
{
	beep_timeout = timeout;
	beep_tick = HAL_GetTick();
	beep_start_tick = HAL_GetTick();
	beep_enable = TRUE;
	PIN_CLR(OUT_BIP_P);
}

void beep_off(void)
{
	beep_enable = FALSE;
	PIN_CLR(OUT_BIP_P);
}

//на входе 1000 Гц - 1 млс
void beep_task(void)
{
	if (beep_enable) {


		if (HAL_DiffTick(beep_tick) <  1000) {
			PIN_TOGGLE(OUT_BIP_P);
		}
		else if (HAL_DiffTick(beep_tick) > 4000) {
			beep_tick = HAL_GetTick();
		}

		if (beep_timeout && (HAL_DiffTick(beep_start_tick) > beep_timeout)) {
			beep_off();
		}

	}
}
