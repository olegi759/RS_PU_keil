/*
 * mux1204.c
 *
 *  Created on: 20 дек. 2023 г.
 *      Author: Rafael Boltachev
 */

#include "mux_sound.h"
#include "stm_util.h"



void set_mux_sound_right(sound_mux_t sound_mux)
{
	switch (sound_mux) {
		case S_OFF:
			PIN_WRITE(EN_SOUND_R, 0);
		break;
		case S_UM1:
			PIN_WRITE(EN_SOUND_R, 1);
			PIN_WRITE(A0_SOUND_R, 0);
			PIN_WRITE(A1_SOUND_R, 0);
		break;
		case S_UM2:
			PIN_WRITE(EN_SOUND_R, 1);
			PIN_WRITE(A0_SOUND_R, 1);
			PIN_WRITE(A1_SOUND_R, 0);
		break;
		case S_MONO:
			PIN_WRITE(EN_SOUND_R, 1);
			PIN_WRITE(A0_SOUND_R, 0);
			PIN_WRITE(A1_SOUND_R, 1);
		break;
		case S_LIN:
			PIN_WRITE(EN_SOUND_R, 1);
			PIN_WRITE(A0_SOUND_R, 1);
			PIN_WRITE(A1_SOUND_R, 1);
		break;
	}

}


void set_mux_sound_left(sound_mux_t sound_mux)
{
	switch (sound_mux) {
		case S_OFF:
			PIN_WRITE(EN_SOUND_L, 0);
		break;
		case S_UM1:
			PIN_WRITE(EN_SOUND_L, 1);
			PIN_WRITE(A0_SOUND_L, 0);
			PIN_WRITE(A1_SOUND_L, 0);
		break;
		case S_UM2:
			PIN_WRITE(EN_SOUND_L, 1);
			PIN_WRITE(A0_SOUND_L, 1);
			PIN_WRITE(A1_SOUND_L, 0);
		break;
		case S_MONO:
			PIN_WRITE(EN_SOUND_L, 1);
			PIN_WRITE(A0_SOUND_L, 0);
			PIN_WRITE(A1_SOUND_L, 1);
		break;
		case S_LIN:
			PIN_WRITE(EN_SOUND_L, 1);
			PIN_WRITE(A0_SOUND_L, 1);
			PIN_WRITE(A1_SOUND_L, 1);
		break;
	}

}


void set_mux_sound_right_left(sound_rl_mux_t sound_rl_mux)
{
	switch (sound_rl_mux) {
		case SLR_OFF:
			set_mux_sound_right(S_OFF);
			set_mux_sound_left(S_OFF);
		break;
		case SLR_UM1:
			set_mux_sound_right(S_UM1);
			set_mux_sound_left(S_UM1);
		break;
		case SLR_UM1_UM2:
			set_mux_sound_right(S_UM1);
			set_mux_sound_left(S_UM2);
		break;
		case SLR_UM2:
			set_mux_sound_right(S_UM2);
			set_mux_sound_left(S_UM2);
		break;
		case SLR_UM2_UM1:
			set_mux_sound_right(S_UM2);
			set_mux_sound_left(S_UM1);
		break;
		case SLR_MONO:
			set_mux_sound_right(S_MONO);
			set_mux_sound_left(S_MONO);
		break;
		case SLR_LIN:
			set_mux_sound_right(S_LIN);
			set_mux_sound_left(S_LIN);
		break;
	}
}




/*

//Управвление MUX_SOUND_1(right) и MUX_SOUND_2(left)


//включить/выключить канал звука left
void set_mux_sound_left_enable(BOOL enable)
{
	PIN_WRITE(EN_SOUND_L, enable);
}

//мултипликсировать канал звука left (DAC_L = 0, DAC_R = 1, DAC_MONO = 2, LIN_R = 3)
void set_mux_sound_left_mux(uint8_t mux)
{
	PIN_WRITE(A0_SOUND_L, mux & 0x01);
	PIN_WRITE(A1_SOUND_L, (mux & 0x02) >> 1);
}

//включить/выключить канал звука right
void set_mux_sound_right_enable(BOOL enable)
{
	PIN_WRITE(EN_SOUND_R, enable);
}

//мултипликсировать канал звука left (DAC_L = 0, DAC_R = 1, DAC_MONO = 2, LIN_L = 3)
void set_mux_sound_right_mux(uint8_t mux)
{
	PIN_WRITE(A0_SOUND_R, mux & 0x01);
	PIN_WRITE(A1_SOUND_R, (mux & 0x02) >> 1);
}

//включить/выключить оба канала
void set_mux_sound_left_right_enable(BOOL enable)
{
	set_mux_sound_left_enable(enable);
	set_mux_sound_right_enable(enable);
}


const uint8_t mux_preset_pins[13][6] = {
		{0, 0, 0, 0, 0, 0}, //0
		{1, 0, 0, 1, 0, 0}, //1
		{1, 1, 0, 1, 1, 0}, //2
		{1, 0, 0, 1, 1, 0}, //3
		{1, 1, 0, 1, 0, 0}, //4
		{1, 0, 1, 1, 0, 1}, //5
		{1, 1, 1, 1, 1, 1}, //6
		{1, 1, 1, 1, 0, 1}, //7
		{1, 0, 1, 1, 1, 1}, //8
		{1, 0, 0, 1, 1, 1}, //9
		{1, 1, 1, 1, 0, 0}, //10
		{1, 1, 1, 1, 1, 0}, //11
		{1, 1, 0, 1, 1, 1}, //12
};

//мултипликсировать оба канала по mux_preset_pins
void set_mux_sound_left_right_preset(uint8_t val)
{
	if (val > 12) val = 0;

	PIN_WRITE(EN_SOUND_R, mux_preset_pins[val][0]);
	PIN_WRITE(A0_SOUND_R, mux_preset_pins[val][1]);
	PIN_WRITE(A1_SOUND_R, mux_preset_pins[val][2]);

	PIN_WRITE(EN_SOUND_L, mux_preset_pins[val][3]);
	PIN_WRITE(A0_SOUND_L, mux_preset_pins[val][4]);
	PIN_WRITE(A1_SOUND_L, mux_preset_pins[val][5]);

}

*/
