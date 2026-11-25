/*
 * screen0.c
 *
 *  Created on: 7 мар. 2025 г.
 *      Author: Rafael Boltachev
 */


#include "screen0.h"
#include "mki_data.h"
#include "utf8_util.h"
#include "str_util.h"
#include "mki_data.h"
#include "stm_util.h"
#include "pu.h"
#include "mkk_data.h"
#include "screens.h"
#include "fault_data.h"
#include "screen1.h"
#include "mchs_mic.h"
#include "mux_sound.h"
#include "math_util.h"
#include "beep.h"
#include "pcm1753.h"
#include "screen4.h"
#include "pd_data.h"
#include "uarts.h"
#include "net_util.h"
#include "screen6.h"
#include "cm7core.h"
#include "display.h"
#include "screen3.h"
#include "common.h"
#include "screen5.h"


//--------------- ОСНОВНОЙ ЭКРАН --------------------

#define SCREEN0_DELIMETR	"#"
extern BOOL mode_rucnoi_pusk_to_dejurniy_blocked ;


void screen0_paint(void)
{
	//ulog("100"CRLF);
	uint8_t fcount;
	uint8_t en_lines_cnt;
	int i, cnt;


	fcount = fault_count();
	en_lines_cnt = pu_enable_lines_cnt();

	clear_display();

	//ulog("200"CRLF);

	set_display(0, 0, get_pu_name());
	set_display(0, 12, SCREEN0_DELIMETR);
	set_display(0, 13, int2str(safety_get_level_from_pin_code()));
	set_display(0, 14, SCREEN0_DELIMETR);
	set_display(0, 15, get_time_str());

	set_display(1, 0, "РЕЖИМ ");
	set_display(1, 6, SCREEN0_DELIMETR);
	set_display(1, 7, pu_mode2str(pu_mode));

	if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
		set_display(2, 0, "ОПЦИЯ АВТ. ОТКЛ.");
		set_display(3, 0, "ВЫБРАНО ВЫХОДОВ-");
		set_display(3, 16, int2str(bitis1cnt(&pu_option_avtomatika_otkluchena_lines, 24)));
	}
	else if (pu_option == PU_OPTION_OTKLUCHENIE) {
		set_display(2, 0, "ОПЦИЯ ОТКЛЮЧЕНИЕ");
		set_display(3, 0, "ВЫБРАНО ВХОДОВ-");
		set_display(3, 15, int2str(bitis1cnt(&pu_option_otkluchenie_lines, 24)));
	}
	else {

		if (pu_mode != PU_MODE_UPRAVLENIE_DRUGIMI_PU) {

			set_display(2, 0, "НЕИСП.");
			set_display(2, 6, SCREEN0_DELIMETR);
			if (fcount > 0) {
				set_display(2, 7, "ЕСТЬ");
				set_display(2, 12, int2str(fcount));
			} else {
				set_display(2, 7, "НЕТ");
			}

		}

		switch (pu_mode) {
			case PU_MODE_DEJURNIY:
				if (fcount > 0) set_display(3, 0, fault2str(fault_first()));
			break;
			case PU_MODE_VESHANIE_PD:
			break;
			case PU_MODE_ZVUKOVOE_VESHANIE:
				set_display(1, 16, SCREEN0_DELIMETR);
				set_display(1, 17, int2str(en_lines_cnt));
				set_display(3, 0, "ИСТОЧ."SCREEN0_DELIMETR);
				set_display(3, 7, soundsrc2str(setting.mode_sound.soundsrc));
			break;
			case PU_MODE_VESHANIE_MCHS:
				set_display(1, 16, SCREEN0_DELIMETR);
				set_display(1, 17, int2str(en_lines_cnt));
				set_display(3, 0, "ИСТОЧ."SCREEN0_DELIMETR"ЗВУК МЧС");
			break;
			case PU_MODE_VESHANIE_POJAR:
				set_display(1, 16, "/");
				set_display(1, 17, int2str(pu_uo_fire));

				set_display(3, 0, "ВХОДЫ");
				set_display(3, 6, SCREEN0_DELIMETR);
				set_display(3, 7, int2str(pu_get_fire_lines_cnt()));
				set_display(3, 10, SCREEN0_DELIMETR);
				set_display(3, 11, "ЗОН/");
				set_display(3, 15, int2str(en_lines_cnt));


				set_display(3, 17, SCREEN0_DELIMETR);
				set_display(3, 18, int2str(uo_fire_time_remaining));
			break;
			case PU_MODE_RUCHNOI_PUSK_PD:
			case PU_MODE_RUCHNOI_PUSK:
				set_display(1, 16, "/");
				//set_display(1, 17, int2str(pu_uo_manual));

				set_display(3, 0, pu_select_group ? "ВЫБОР ГР." : "ВЫБОР ЗОН");
				set_display(3, 9, SCREEN0_DELIMETR);
				set_display(3, 10, "ВЫБ.-");
				set_display(3, 15, int2str(pu_get_lines_cnt_from_leds()));

				set_display(3, 17, SCREEN0_DELIMETR);
				//set_display(3, 18, int2str(uo_manual_time_remaining));

			break;
			case PU_MODE_UPRAVLENIE_DRUGIMI_PU:

				set_display(2, 0, "ВОСПР.");
				set_display(2, 6, SCREEN0_DELIMETR);
				set_display(2, 7, mchs_mic_is_dma_started() ? "МИКРОФОН" : "ФАЙЛ");


				set_display(3, 0, "ВЫБОР ЗОН");
				set_display(3, 9, SCREEN0_DELIMETR);
				set_display(3, 10, "ВЫБ.-");
				cnt = 0;
				for (i = 1; i <= 25; i++) {
					 cnt += bitis1cnt(&screen6_lines[i], 24);
				}
				set_display(3, 15, int2str(cnt));
				set_display(3, 17, SCREEN0_DELIMETR);
				//set_display(3, 18, int2str(uo_manual_time_remaining));

			break;

			default : break;
		}
	}
	//ulog("300"CRLF);
	send_display();
	//ulog("400"CRLF);
}


uint32_t get_all_enable_lines(void)
{
	if (setting.lines_count < 3) { //2
		return 0x03;
	}
	else if (setting.lines_count < 5) { //4
		return 0x0F;
	}
	else if (setting.lines_count < 9) { //8
		return 0xFF;
	}
	else if (setting.lines_count < 13) { //12
		return 0xFFF;
	}
	return 0xFFFFFF;

}

void screen0_button_press(uint8_t key)
{
	//int i;
	//pd_audio_param_cmd_data_t pd_audio_data;
	uint32_t lines;

	switch (key) {

		case	MKI_KEY_1:
		case	MKI_KEY_2:
		case	MKI_KEY_3:
		case	MKI_KEY_4:
		case	MKI_KEY_5:
		case	MKI_KEY_6:
		case	MKI_KEY_7:
		case	MKI_KEY_8:
		case	MKI_KEY_9:
		case	MKI_KEY_10:
		case	MKI_KEY_11:
		case	MKI_KEY_12:
		case	MKI_KEY_13:
		case	MKI_KEY_14:
		case	MKI_KEY_15:
		case	MKI_KEY_16:
		case	MKI_KEY_17:
		case	MKI_KEY_18:
		case	MKI_KEY_19:
		case	MKI_KEY_20:
		case	MKI_KEY_21:
		case	MKI_KEY_22:
		case	MKI_KEY_23:
		case	MKI_KEY_24:

			if (pu_select_group) {
				if (key > MKI_KEY_10) return;
			}

			if (setting.lines_count < 3) { //2
				if (key > MKI_KEY_2) return;
			}
			else if (setting.lines_count < 5) { //4
				if (key > MKI_KEY_4) return;
			}
			else if (setting.lines_count < 9) { //8
				if (key > MKI_KEY_8) return;
			}
			else if (setting.lines_count < 13) { //12
				if (key > MKI_KEY_12) return;
			}

			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEYS by user level"CRLF, 2);
				return;
			}

			if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
				INVBIT(pu_option_avtomatika_otkluchena_lines, (key - 1));
				cm7core_set_led_color(MKI_LED_AUTOMATIONOFF, pu_option_avtomatika_otkluchena_lines ? LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA : LED_COLOR_BLACK);
				pu_paint_led_lines(pu_option_avtomatika_otkluchena_lines, LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA);
				screen0_update();
			}
			else if (pu_option == PU_OPTION_OTKLUCHENIE) {
				INVBIT(pu_option_otkluchenie_lines, (key - 1));
				cm7core_set_led_color(MKI_LED_DISCONNECT, pu_option_otkluchenie_lines ? LED_COLOR_OPTION_OTKLUCHENIE : LED_COLOR_BLACK);
				pu_paint_led_lines(pu_option_otkluchenie_lines, LED_COLOR_OPTION_OTKLUCHENIE);
				screen0_update();
			}
			else if (pu_mode == PU_MODE_RUCHNOI_PUSK) {
				INVBIT(pu_ruchnoi_pusk_lines, (key - 1));
				pu_paint_led_lines(pu_ruchnoi_pusk_lines, pu_mode2ledcolor(PU_MODE_RUCHNOI_PUSK));
				screen0_update();
				ulog_fmt(CONM"pu_ruchnoi_pusk_lines:%s"CRLF ,u24l2bitstr(pu_ruchnoi_pusk_lines));
			}
			//ЭТО на screen6
			//else if (pu_mode == PU_MODE_UPRAVLENIE_DRUGIMI_PU) {
			//	pu_paint_led_lines(pu_upravlenie_drugimi_pu_lines, pu_mode2ledcolor(PU_MODE_UPRAVLENIE_DRUGIMI_PU));
			//	screen0_update();
			//	ulog_fmt(CONM"pu_upravlenie_drugimi_pu_lines:%s"CRLF ,u24l2bitstr(pu_upravlenie_drugimi_pu_lines));
			//}

		break;
		case MKI_KEY_ALLZONES:

			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEY_ALLZONES by user level"CRLF, 2);
				return;
			}

			if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
				cm7core_set_led_color(MKI_LED_AUTOMATIONOFF, LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA);
				pu_option_avtomatika_otkluchena_lines = get_all_enable_lines();
				pu_paint_led_lines(pu_option_avtomatika_otkluchena_lines, LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA);
				screen0_update();
			}
			else if (pu_option == PU_OPTION_OTKLUCHENIE) {
				cm7core_set_led_color(MKI_LED_DISCONNECT, LED_COLOR_OPTION_OTKLUCHENIE);
				pu_option_otkluchenie_lines = get_all_enable_lines();
				pu_paint_led_lines(pu_option_otkluchenie_lines, LED_COLOR_OPTION_OTKLUCHENIE);
				screen0_update();
			}
			else if (pu_mode == PU_MODE_RUCHNOI_PUSK) {
				pu_ruchnoi_pusk_lines = get_all_enable_lines();
				pu_paint_led_lines(pu_ruchnoi_pusk_lines, pu_mode2ledcolor(PU_MODE_RUCHNOI_PUSK));
				screen0_update();
			}
			//else if (pu_mode == PU_MODE_SETEVOE_UPRAVLENIE) {
			//	for (i = MKI_LED_1; i <= MKI_LED_12; i++) {
			//		mki_set_led(i, pu_mode2ledcolor(PU_MODE_RUCHNOI_PUSK));
			//	}
			//	mki_send_leds();
			//	screen0_paint();
			//}

		break;
		case MKI_KEY_ENTER:

			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEY_ENTER by user level"CRLF, 2);
				return;
			}

			if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
				pu_option = PU_OPTION_NONE;
				pu_paint_led_lines(0, LED_COLOR_BLACK);
				screen0_paint();
			}
			else if (pu_option == PU_OPTION_OTKLUCHENIE) {
				pu_option = PU_OPTION_NONE;
				pu_paint_led_lines(0, LED_COLOR_BLACK);
				screen0_paint();
			}
			else if (pu_mode == PU_MODE_RUCHNOI_PUSK) {

				if (!pu_get_lines_cnt_from_leds()) {
					ulog("pu: abort ruchnoi pusk (not selected lines)"CRLF);
					SMALL_BEEP;
					pu_set_mode(PU_MODE_DEJURNIY);
					return;
				}

				//pu_dma_stop();


				screen5_init(FALSE);
				screens_goto(5);

				//filelog_fmt("Плеер, Линии опов.:%s", pu_lines2str(pu_ruchnoi_pusk_lines));

			}
			//else if (pu_mode == PU_MODE_SETEVOE_UPRAVLENIE) {


			//	pd_audio_data.frequency = HTONL(MCHS_MIC_AUDIO_FREQUENCY);
			//    pd_audio_data.stereo = FALSE;
			//	pd_audio_data.sound_lines = pu_get_lines_from_leds();

			//	uarts_transmit_piv_nohuart(str2int(setevoe_upravlenie_addr_pu), setting.uart_local_addr, PD_CMD_RUCHNOI_PUSK, (uint8_t *)&pd_audio_data, sizeof(pd_audio_data));

			//}

		break;
		case MKI_KEY_MIC:

			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEY_MIC by user level"CRLF, 2);
				return;
			}

			if (pu_mode == PU_MODE_RUCHNOI_PUSK) {


				//if (!pu_get_lines_cnt_from_leds()) {
				//	ulog("pu: abort ruchnoi pusk (not selected lines)"CRLF);
				//	SMALL_BEEP;
				//	pu_set_mode(PU_MODE_DEJURNIY);
				//	return;
				//}


				//pu_dma_stop();



				MUTE_OFF;
				cm7core_send_one_led_color(MKI_LED_MUTE, LED_COLOR_BLACK);

				if (setting.mic_is_fix) {


					if (mchs_mic_is_dma_started()) {
						mode_rucnoi_pusk_to_dejurniy_blocked = TRUE;
						mchs_mic_stop();
					} else {
						lines = pu_get_manual_lines_from_leds();
						pu_enable_sound_lines(lines);
						//mchs_mic_start(TRUE, 0xFF);

						if (setting.mode_manual_tilibom_mic) {
							mode_rucnoi_pusk_to_dejurniy_blocked = TRUE;
							tilibom_play("mic");
						} else {
							mode_rucnoi_pusk_to_dejurniy_blocked = TRUE;
							mchs_mic_start(TRUE, 0xFF);
						}

						filelog_fmt("Микрофон, Линии опов.:%s", pu_lines2str(lines));
					}


				} else {

					lines = pu_get_manual_lines_from_leds();
					pu_enable_sound_lines(lines);
					mode_rucnoi_pusk_to_dejurniy_blocked = TRUE;
					mchs_mic_start(TRUE, 0xFF);
					filelog_fmt("Микрофон, Линии опов.:%s", pu_lines2str(lines));
				}


				//pu_enable_sound_lines(lines);


				//MUTE_OFF;
				//filelog_fmt("Микрофон, Линии опов.:%s", pu_lines2str(lines));

				//cm7core_send_one_led_color(MKI_LED_MUTE,  MUTE_IS_ON ? LED_COLOR_YELLOW : LED_COLOR_BLACK);
				//ulog_fmt(CONM"MUTE %s"CRLF, (MUTE_IS_ON ? "ON" : "OFF"));

				//if (setting.mode_manual_tilibom_mic) {
					//tilibom_play("mic");
				//} else {
					//mchs_mic_start(TRUE, 0xFF);
				//}



			}
			else if (pu_mode == PU_MODE_VESHANIE_MCHS) {

			}
			else if (pu_mode == PU_MODE_ZVUKOVOE_VESHANIE) {

				if (setting.mode_sound.soundsrc == PU_SOUNDSRC_MICROPHONE) {
						if (mchs_mic_is_dma_started()) mchs_mic_stop();
						else mchs_mic_start(TRUE, 0xFF);
				}

			}
			//else if (pu_mode == PU_MODE_SETEVOE_UPRAVLENIE) {

			    //pd_audio_data.stereo = FALSE;
				//pd_audio_data.sound_lines = pu_get_lines_from_leds();
				//uarts_transmit_piv_nohuart(str2int(setevoe_upravlenie_addr_pu), setting.uart_local_addr, PD_CMD_VESHANIE_PD, (uint8_t *)&pd_audio_data, sizeof(pd_audio_data));

				//mchs_mic_start(TRUE, str2int(setevoe_upravlenie_addr_pu));
			//}
		break;
		case MKI_KEY_ZONES:
			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEY_ZONES by user level"CRLF, 2);
				return;
			}
			pu_select_group = FALSE;
			screen0_update();
		break;
		case MKI_KEY_GROUPS:
			if (safety_get_level_from_pin_code() < 2) {
				pu_invalid_access_code(CONM"screen0: forbidden - MKI_KEY_GROUPS by user level"CRLF, 2);
				return;
			}
			pu_select_group = TRUE;
			screen0_update();
		break;
	}
}

void screen0_button_unpress(uint8_t key)
{
	switch (key) {
		case MKI_KEY_MIC:
			if (pu_mode == PU_MODE_RUCHNOI_PUSK) {

				if (setting.mic_is_fix) {

				}
				else {
					mode_rucnoi_pusk_to_dejurniy_blocked = TRUE;
					mchs_mic_stop();
				}
			}
			else if (pu_mode == PU_MODE_VESHANIE_MCHS) {

			}
		break;
	}

}
