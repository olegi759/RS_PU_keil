/*
 * speaker.h
 *
 *  Created on: 10 июн. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM4_CORE_INC_SPEAKER_H_
#define CM4_CORE_INC_SPEAKER_H_

#include "main.h"

//состояния спикера
typedef enum {
	IS_OFF = 0, //выключен
	IS_UM1, //подлючен к УМ1
	IS_UM2, //подлючен к УМ2
	IS_UM1_UM2, //подлючен к УM1 и УМ2 (DAC_MONO)
	IS_LINE_INPUT, //подлючен к лтнейному входу
} internal_speaker_mux_t;

void internal_speaker_set_mux(internal_speaker_mux_t internal_speaker_mux);



#endif /* CM4_CORE_INC_SPEAKER_H_ */
