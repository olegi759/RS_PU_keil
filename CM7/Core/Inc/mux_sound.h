/*
 * mux1204.h
 *
 *  Created on: 20 дек. 2023 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MUX_SOUND_H_
#define CORE_INC_MUX_SOUND_H_

#include "main.h"

//состояния MUX SOUND
typedef enum {
	S_OFF = 0, //выключен
	S_UM1, //подлючен к УМ1
	S_UM2, //подлючен к УМ2
	S_MONO, //подлючен к MONO
	S_LIN, //подлючен к линейному входу
} sound_mux_t;



//состояния обоих MUX SOUND
typedef enum {
	SLR_OFF = 0, //оба выключены
	SLR_UM1, //оба подлючены к УМ1
	SLR_UM1_UM2, //1-й подлючен к УМ1 - 2-й к УМ2
	SLR_UM2, //оба подлючен к УМ2
	SLR_UM2_UM1, //1-й подлючен к УМ2 - 2-й к УМ1
	SLR_MONO, //подлючен к MONO
	SLR_LIN, //подлючен к линейному входу
} sound_rl_mux_t;

void set_mux_sound_right_left(sound_rl_mux_t sound_rl_mux);


#define MUTE_OFF		PIN_SET(SP_ON)
#define MUTE_ON			PIN_CLR(SP_ON)
#define MUTE_TOGGLE		PIN_TOGGLE(SP_ON)
#define MUTE_IS_ON		(PIN_GET(SP_ON) == 0)
/*

#define MUX_SOUND_DAC_L		0
#define MUX_SOUND_DAC_R		1
#define MUX_SOUND_DAC_MONO	2
#define MUX_SOUND_LIN		3

void set_mux_sound_left_enable(BOOL enable);
void set_mux_sound_left_mux(uint8_t mux);
void set_mux_sound_right_enable(BOOL enable);
void set_mux_sound_right_mux(uint8_t mux);
void set_mux_sound_left_right_enable(BOOL enable);
void set_mux_sound_left_right_preset(uint8_t val);
*/







/*
void internal_speaker_set_enable(BOOL enable);
void internal_speaker_set_channel(BOOL channel_right);
void internal_speaker_set_from_int_spk(uint8_t int_spk);
*/

#endif /* CORE_INC_MUX_SOUND_H_ */
