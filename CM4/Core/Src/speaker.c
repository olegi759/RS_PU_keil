/*
 * speaker.c
 *
 *  Created on: 10 июн. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "speaker.h"
#include "stm_util.h"

//Внутренний динамик ПУ
//EN_SPK - Активное высокое логическое разрешение. Когда этот вывод находится в низком состоянии, все переключатели выключены.
//Когда этот вывод находится в высоком состоянии, логические входы
//A[1:0] определяют, какой переключатель включен.
void internal_speaker_set_mux(internal_speaker_mux_t internal_speaker_mux)
{
	switch (internal_speaker_mux) {
		case IS_OFF:
			PIN_WRITE(EN_SPK, 0);
		break;
		case IS_UM1:
			PIN_WRITE(EN_SPK, 1);
			PIN_WRITE(A0_SPK, 0);
			PIN_WRITE(A1_SPK, 0);
		break;
		case IS_UM2:
			PIN_WRITE(EN_SPK, 1);
			PIN_WRITE(A0_SPK, 1);
			PIN_WRITE(A1_SPK, 0);
		break;
		case IS_UM1_UM2:
			PIN_WRITE(EN_SPK, 1);
			PIN_WRITE(A0_SPK, 0);
			PIN_WRITE(A1_SPK, 1);
		break;
		case IS_LINE_INPUT:
			PIN_WRITE(EN_SPK, 1);
			PIN_WRITE(A0_SPK, 1);
			PIN_WRITE(A1_SPK, 1);
		break;
	}

}
