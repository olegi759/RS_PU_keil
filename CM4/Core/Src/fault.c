/*
 * fault_data.c
 *
 *  Created on: 7 мая 2026 г.
 *      Author: Rafael Boltachev
 */


#include "fault.h"
#include "fault_data.h"
#include "mki.h"
#include "mup.h"
#include "mkk.h"
#include "mkk_data.h"
#include "pd_data.h"
#include "math_util.h"
#include "str_util.h"
#include "cm4core.h"
#include <stdarg.h>
#include <stdio.h>
#include "rtc.h"
#include "logfile_util.h"
#include "common.h"





//Установить ошибку ПУ
void fault_set(uint8_t num)
{

	char buf[64];

	if (bitget(faults, num)) return;
	bitset(faults, num, 1);



	buf[0] = '\0';
	if ((num >= FAULT_MKK_INSIG_1) && (num <= FAULT_MKK_INSIG_2)) {
		strcpy_s(buf, sizeof(buf), mkk_get_insig_state_str(num - FAULT_MKK_INSIG_1));
	}
	if ((num >= FAULT_MKK_SOUND_1) && (num <= FAULT_MKK_SOUND_24)) {
		strcpy_s(buf, sizeof(buf), mkk_get_lin_state_str(num - FAULT_MKK_SOUND_1));
	}
	if ((num >= FAULT_MKK_START_1) && (num <= FAULT_MKK_START_2)) {
		strcpy_s(buf, sizeof(buf), mkk_get_start_state_str(num - FAULT_MKK_START_1));
	}



	mki_set_small_led(MKI_LED_FAULT, LED_COLOR_YELLOW); //если есть ощибка светодиод зажечь желтьым
	//mki_set_small_led(MKI_LED_MUTE, LED_COLOR_BLACK); //выключить MUTE для звука
	mki_send_small_leds();

	HAL_Delay(50);
	mkk_send_relay_mux(UART2_MUX_MKKL_1, MKK_RELAY_1, TRUE);
    if (setting.lines_count > 12) {
    	HAL_Delay(50);
	    mkk_send_relay_mux(UART2_MUX_MKKL_2, MKK_RELAY_1, TRUE);
    }

	filelog_fmt("Возникла неисправность: %s %s", fault2str(num), buf);

	cm4core_send_to_CM7_cmd(CORE_CMD_CHANGE_FAULTS, &faults, sizeof(faults));
}


//Снять ошибку
void fault_unset(uint8_t num)
{

	if (!bitget(faults, num)) return;
	bitset(faults, num, 0);

	if (fault_count() == 0) {
		mki_send_one_small_led(MKI_LED_FAULT, LED_COLOR_BLACK); //если нет ошибок погасить светодиод
		HAL_Delay(50);
	    mkk_send_relay_mux(UART2_MUX_MKKL_1, MKK_RELAY_1, FALSE);
	    if (setting.lines_count > 12) {
	    	HAL_Delay(50);
		    mkk_send_relay_mux(UART2_MUX_MKKL_2, MKK_RELAY_1, FALSE);
	    }
	}


	filelog_fmt("Устранена неисправность: %s", fault2str(num));

	cm4core_send_to_CM7_cmd(CORE_CMD_CHANGE_FAULTS, &faults, sizeof(faults));
}




