/*
 * pd.c
 *
 *  Created on: 20 РјР°СЂ. 2025 Рі.
 *      Author: Rafael Boltachev
 */


#include "pd.h"
#include "pd_data.h"
#include "pu.h"
#include "pcm1753.h"
#include "opus_util.h"
#include "stm_util.h"
#include "str_util.h"
#include "math_util.h"
#include "mkk_data.h"
#include "fault_data.h"
#include "pu.h"
#include "screen0.h"
#include "screens.h"
#include "uarts.h"
#include "cm7core.h"
#include "mki_data.h"
#include "common.h"
#include "net_util.h"

uint8_t pd_translation_addr = 0;
uint32_t pd_id = 0;


BOOL pd_start_translation(uint32_t freq, BOOL stereo)
{

	pu_stop_translation();

	if (!pu_prepare_audio_play(freq, stereo)) return FALSE;

	return TRUE;
}


void pd_transmit_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len, uint8_t huart_number)
{
	uint8_t packet[5 + cmd_data_len + 2];
	uint16_t packet_len;

	if (huart_number) {
		uarts_transmit_piv(huart_number, daddr, saddr, cmd, cmd_data, cmd_data_len);
	} else {
		packet_len = create_piv_packet_cmd(packet, daddr, saddr, cmd, cmd_data, cmd_data_len);
		//uarts_transmit_it_by_num(huart_number,  packet, packet_len);
		cm7core_pd_send(packet, packet_len);
	}
}


///СЃ Р»РѕРєР°Р»СЊРЅС‹С… РџР” huart_number == 0
void pd_rcv(uint8_t * packet, uint16_t packet_len, uint8_t huart_number)
{
	uint8_t info[PU_INFO_SIZE];
	uint8_t buf[MAX_PACKET_SIZE];
	uint32_t freq;
	BOOL stereo;
	uint32_t lines;
	//uint32_t pd_id;
	//int i, j;
	//pu_small_info_t  small_info;
	uint16_t len;




	switch (packet[4]) {

		case PD_CMD_PING:
			pd_transmit_piv(packet[1], packet[0], PD_CMD_PONG, NULL, 0, huart_number);
		break;
		case PD_CMD_INFO:

			if (packet_len != sizeof(pd_onlycommand_packet_t)) return;
			//if (packet_len != 7) return;

			pu_get_info(info);
			pd_transmit_piv(packet[1], packet[0], PD_CMD_INFO_RES, info, sizeof(info), huart_number);

		break;
		case PD_CMD_VESHANIE_PD:


			if (packet_len != sizeof(pd_adio_param_packet_t)) return;

			if (pu_mode > PU_MODE_VESHANIE_PD) { //Р±РѕР»СЊС€Рµ РїСЂРёРѕСЂРёС‚РµС‚
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_PD_RES, buf, 1, huart_number);
				return;
			}


			freq = BE_FOURBYTE2UL(packet[5], packet[6], packet[7], packet[8]);
			stereo = packet[9] ? TRUE : FALSE;
			lines = THREEBYTE2LINES(packet[10], packet[11], packet[12]);
			pd_id = BE_FOURBYTE2UL(packet[13], packet[14], packet[15], packet[16]);

			//freq = ((uint32_t)packet[5] << 24) | ((uint32_t)packet[6]  << 16)| ((uint32_t)packet[7] << 8) | (uint32_t)packet[8];
			//lines = ((uint32_t)packet[12]<< 16) | ((uint32_t)packet[11] << 8 )| packet[10];


			if (freq == 0) {
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_PD_RES, buf, 1, huart_number);
				return;
			}

			if (pu_mode == PU_MODE_VESHANIE_PD) {

				if ((last_frequency != freq) || (pcm1753_GetStereo() != stereo)) {
					 if (!pd_start_translation(freq, stereo)) {
						buf[0] = MKK_ERROR_PARAMS;
						pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_PD_RES, buf, 1, huart_number);
						fulog(CONM"pd: PU_MODE_VESHANIE_PD invalid pd_start_translation"CRLF);
						return;
					 }
					 fulog_fmt(CONM"pd: PU_MODE_VESHANIE_PD change freq:%lu, stereo:%s"CRLF, (unsigned long)freq, bool2str(stereo));
				}
				if (pu_need_lines != lines) {
					pu_enable_sound_lines(lines);
				}


			}
			else {
				pu_set_mode(PU_MODE_VESHANIE_PD);
				if (!pd_start_translation(freq, stereo)) {
					buf[0] = MKK_ERROR_PARAMS;
					pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_PD_RES, buf, 1, huart_number);
					fulog(CONM"pd: PU_MODE_VESHANIE_PD invalid pd_start_translation"CRLF);
					return;
				}
				fulog_fmt(CONM"pd: PU_MODE_VESHANIE_PD set mode VESHANIE_PD freq:%lu, stereo:%s"CRLF, (unsigned long)freq, bool2str(stereo));
				pu_enable_sound_lines(lines);
			}

			pd_transmit_piv(packet[1], packet[0] , PD_CMD_VESHANIE_PD_RES, NULL, 0, huart_number);
			pd_translation_addr = packet[1];
		break;

		case PD_CMD_DEJURNIY:
			if (packet_len != sizeof(pd_dejurniy_packet_t)) return;

			if (pu_mode != PU_MODE_DEJURNIY) {
				pu_set_mode(PU_MODE_DEJURNIY);
				ulog("pd: PD_CMD_DEJURNIY"CRLF);
			}
			pd_transmit_piv(packet[1], packet[0], PD_CMD_DEJURNIY_RES, NULL, 0, huart_number);
			pd_translation_addr = 0;
		break;

		case PD_CMD_ZVUKOVOE_VESHANIE:

			if (packet_len != sizeof(pd_sound_lines_packet_t)) return;

			if (pu_mode > PU_MODE_ZVUKOVOE_VESHANIE) {
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_ZVUKOVOE_VESHANIE_RES, buf, 1, huart_number);
				return;
			}

			if (pu_mode != PU_MODE_ZVUKOVOE_VESHANIE) {
				pu_set_mode(PU_MODE_ZVUKOVOE_VESHANIE);
				ulog("pd: PD_CMD_ZVUKOVOE_VESHANIE"CRLF);
			}

			//lines = packet[5] |  ((uint32_t)packet[6] << 8) |  ((uint32_t)packet[7] << 16);
			lines = THREEBYTE2LINES(packet[5], packet[6], packet[7]);
			pd_id = BE_FOURBYTE2UL(packet[8], packet[9], packet[10], packet[11]);

			if (pu_need_lines != lines) {
				pu_enable_sound_lines(lines);
			}

			pd_transmit_piv(packet[1], packet[0], PD_CMD_ZVUKOVOE_VESHANIE_RES, NULL, 0, huart_number);
			pd_translation_addr = packet[1];
		break;

		case PD_CMD_VESHANIE_POJAR:

			if (packet_len != sizeof(pd_sound_lines_packet_t)) return;

			if (pu_mode > PU_MODE_VESHANIE_POJAR) {
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_POJAR_RES, buf, 1, huart_number);
				return;
			}

			//fire_lines_imit = packet[5] |  ((uint32_t)packet[6] << 8) |  ((uint32_t)packet[7] << 16);
			fire_lines_imit = THREEBYTE2LINES(packet[5], packet[6], packet[7]);
			pd_id = BE_FOURBYTE2UL(packet[8], packet[9], packet[10], packet[11]);


			if (pu_mode != PU_MODE_VESHANIE_POJAR) {
				pu_set_mode(PU_MODE_VESHANIE_POJAR);
				ulog_fmt("pd: PD_CMD_VESHANIE_POJAR fire_lines:%s"CRLF, u24l2bitstr(fire_lines_imit));
			}
			pd_transmit_piv(packet[1], packet[0], PD_CMD_VESHANIE_POJAR_RES, NULL, 0, huart_number);
			pd_translation_addr = packet[1];
		break;

		case PD_CMD_VESHANIE_MCHS:

			if (packet_len != sizeof(pd_sound_lines_packet_t)) return;

			if (pu_mode > PU_MODE_VESHANIE_MCHS) {
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_VESHANIE_MCHS_RES, buf, 1, huart_number);
				return;
			}

			if (pu_mode != PU_MODE_VESHANIE_MCHS) {
				pu_set_mode(PU_MODE_VESHANIE_MCHS);
				ulog("pd: PD_CMD_VESHANIE_MCHS"CRLF);
			}

			//lines = packet[5] |  ((uint32_t)packet[6] << 8) |  ((uint32_t)packet[7] << 16);
			lines = THREEBYTE2LINES(packet[5], packet[6], packet[7]);
			pd_id = BE_FOURBYTE2UL(packet[8], packet[9], packet[10], packet[11]);

			if (pu_need_lines != lines) {
				pu_enable_sound_lines(lines);
			}

			pd_transmit_piv(packet[1], packet[0], PD_CMD_VESHANIE_MCHS_RES, NULL, 0, huart_number);
			pd_translation_addr = packet[1];
		break;

		case PD_CMD_OTKLUCHENIE:

			if (packet_len != sizeof(pd_sound_lines_packet_t)) return;

			//pu_option_otkluchenie_lines = packet[5] |  ((uint32_t)packet[6] << 8) |  ((uint32_t)packet[7] << 16);
			pu_option_otkluchenie_lines = THREEBYTE2LINES(packet[5], packet[6], packet[7]);
			pd_id = BE_FOURBYTE2UL(packet[8], packet[9], packet[10], packet[11]);
			cm7core_send_one_led_color(MKI_LED_DISCONNECT, pu_option_otkluchenie_lines ? LED_COLOR_OPTION_OTKLUCHENIE : LED_COLOR_BLACK);
			ulog_fmt("pd: PD_CMD_OTKLUCHENIE %s"CRLF, u24l2bitstr(pu_option_otkluchenie_lines));

			if (pu_option == PU_OPTION_OTKLUCHENIE) {
				pu_paint_led_lines(pu_option_otkluchenie_lines, LED_COLOR_OPTION_OTKLUCHENIE);
				screen0_update();
			}
			pd_transmit_piv(packet[1], packet[0], PD_CMD_OTKLUCHENIE_RES, NULL, 0, huart_number);

		break;
		case PD_CMD_AVTOMATIKA_OTKLUCHENA:

			if (packet_len != sizeof(pd_sound_lines_packet_t)) return;

			//pu_option_avtomatika_otkluchena_lines =packet[5] |  ((uint32_t)packet[6] << 8) |  ((uint32_t)packet[7] << 16);
			pu_option_avtomatika_otkluchena_lines = THREEBYTE2LINES(packet[5], packet[6], packet[7]);
			pd_id = BE_FOURBYTE2UL(packet[8], packet[9], packet[10], packet[11]);
			cm7core_set_led_color(MKI_LED_AUTOMATIONOFF, pu_option_avtomatika_otkluchena_lines ? LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA : LED_COLOR_BLACK);
			cm7core_send_leds();
			ulog_fmt("pd: PD_CMD_AVTOMATIKA_OTKLUCHENA %s"CRLF, u24l2bitstr(pu_option_avtomatika_otkluchena_lines));

			if (pu_option == PU_OPYION_AVTOMATIKA_OTKLUCHENA) {
				pu_paint_led_lines(pu_option_avtomatika_otkluchena_lines, LED_COLOR_OPTION_AVTOMATIKA_OTKLUCHENA);
				screen0_update();
			}

			pd_transmit_piv(packet[1], packet[0], PD_CMD_AVTOMATIKA_OTKLUCHENA_RES, NULL, 0, huart_number);
		break;
		case PD_CMD_RUCHNOI_PUSK:

			if (packet_len != sizeof(pd_adio_param_packet_t)) return;

			if (pu_mode > PU_MODE_RUCHNOI_PUSK_PD) {
				buf[0] = MKK_ERROR_PARAMS;
				pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_RUCHNOI_PUSK_RES, buf, 1, huart_number);
				return;
			}

			freq = BE_FOURBYTE2UL(packet[5], packet[6], packet[7], packet[8]);
			stereo = packet[9] ? TRUE : FALSE;
			lines = THREEBYTE2LINES(packet[10], packet[11], packet[12]);
			pd_id = BE_FOURBYTE2UL(packet[13], packet[14], packet[15], packet[16]);


			//freq = ((uint32_t)packet[5] << 24) | ((uint32_t)packet[6]  << 16)| ((uint32_t)packet[7] << 8) | (uint32_t)packet[8];
			//lines = ((uint32_t)packet[12]<< 16) | ((uint32_t)packet[11] << 8 )| packet[10];


			if (pu_mode == PU_MODE_RUCHNOI_PUSK_PD) {

				if ((last_frequency != freq) || (pcm1753_GetStereo() != stereo)) {
					if (!pd_start_translation(freq, stereo)) {
						buf[0] = MKK_ERROR_PARAMS;
						pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_RUCHNOI_PUSK_RES, buf, 1, huart_number);
						return;
					}
					ulog_fmt("pd: mode PD_CMD_RUCHNOI_PUSK change frequency:%lu stereo:%s"CRLF, (unsigned long)freq, bool2str(stereo));
				}

				if (pu_need_lines != lines) {
					pu_enable_sound_lines(lines);
					//if (lines) filelog_fmt("Р›РёРЅРёРё РѕРїРѕРІ.: %s", pu_lines2str(lines));
					ulog_fmt("pd: mode PD_CMD_RUCHNOI_PUSK change lines:%s->pu_enable_lines:%s"CRLF, u24l2bitstr(lines), u24l2bitstr(pu_enable_lines));
				}
			} else {
				pu_set_mode(PU_MODE_RUCHNOI_PUSK_PD);
				if (!pd_start_translation(freq, stereo)) {
					buf[0] = MKK_ERROR_PARAMS;
					pd_transmit_piv(packet[1], packet[0] , 0x80 | PD_CMD_RUCHNOI_PUSK_RES, buf, 1, huart_number);
					return;
				}
				pu_enable_sound_lines(lines);
				//if (lines) filelog_fmt("Р›РёРЅРёРё РѕРїРѕРІ.: %s", pu_lines2str(lines));
				ulog_fmt("pd: set mode PD_CMD_RUCHNOI_PUSK lines:%s->pu_enable_lines:%s"CRLF, u24l2bitstr(lines), u24l2bitstr(pu_enable_lines));
			}

			pd_transmit_piv(packet[1], packet[0] , PD_CMD_RUCHNOI_PUSK_RES, NULL, 0, huart_number);
			pd_translation_addr = packet[1];
		break;
		case PD_CMD_SPISOK_PU:
			 pu_fill_local_small_infos();
			 len = pu_get_data_from_small_infos(buf);

 			 pd_transmit_piv(packet[1], packet[0] , PD_CMD_SPISOK_PU_RES, buf, len, huart_number);
		break;
		case PD_CMD_NET_INFO:
			pu_netinfo[0] = packet[5];
			pu_netinfo[1] = packet[6];
			pu_netinfo[2] = packet[7];


 			pd_transmit_piv(packet[1], packet[0] , PD_CMD_NET_INFO_RES, NULL, 0, huart_number); //РѕС‚РІРµС‚

			if (huart_number == 0) {
				uart4_uart7_snd_piv(0xFF, setting.uart_local_addr, PU_CMD_NET_INFO_UPDATE, pu_netinfo, sizeof(pu_netinfo));
			}

		break;

	}
}


BOOL pd_get_audio_param_from_command(uint8_t cmd, uint32_t * freq, BOOL * stereo)
{
	switch (cmd) {
		case PD_CMD_OPUS48mono:
			*freq = 48000;
			*stereo = FALSE;
			break;
		case PD_CMD_OPUS16mono:
			*freq = 16000;
			*stereo = FALSE;
			break;
		case PD_CMD_OPUS48ster:
			*freq = 48000;
			*stereo = TRUE;
			break;
		case PD_CMD_OPUS16ster:
			*freq = 16000;
			*stereo = TRUE;
			break;
		default: return FALSE;
	}
	return TRUE;
}

uint8_t pd_get_opus_cmd_from_frequency_stereo(uint32_t frequency, BOOL stereo)
{
	if ((frequency == 48000) && (stereo == FALSE)) return PD_CMD_OPUS48mono;
	if ((frequency == 16000) && (stereo == FALSE)) return PD_CMD_OPUS16mono;
	if ((frequency == 48000) && (stereo == TRUE)) return PD_CMD_OPUS48ster;
	if ((frequency == 16000) && (stereo == TRUE)) return PD_CMD_OPUS16ster;
	return PD_CMD_OPUS16mono;
}

/*


//РѕС‚РєСѓРґР° РїСЂРёС€Р»Рё РґР°РЅРЅС‹Рµ СЃ РџР”
//static uart6_mux_t mux_pd_rcv[3]= {UART6_MUX_TMP, UART6_MUX_P1C, UART6_MUX_P2C};
//РІСЂРµРјСЏ РєРѕРіРґР° РїСЂРёС€Р»Рё РґР°РЅРЅС‹Рµ СЃ РџР”
static uint32_t mux_pd_rcv_tick[4][3]= {0};

static uint8_t pd_send_addr = 1;
uint8_t pd_translation_addr = 0;

static uint8_t pd_tx_buf[MAX_UART6_PACKET_SIZE];


//static uint32_t pd_send_tick[3] = {0};

void pd_on_sai_full(void)
{

}

void pd_on_sai_empty(void)
{

}



BOOL pd_start_translation(uint32_t freq, BOOL stereo)
{

	pu_stop_translation();

	if (!pu_prepare_audio_play(freq, stereo)) return FALSE;

	return TRUE;
}









void pd_rcv(uart6_mux_t mux, uint8_t * packet, uint16_t packet_len)
{
	//data[0] - daddr
	//data[1] - saddr
	//data[2] and data[3] - len
	//data[4] - cmd

	//AA 0B 09 00 C1 80 3E 00 00 00 00 00 00 64 C8


	if ((packet[1] != 1) && (packet[1] != 2)) return;

	mux_pd_rcv_tick[mux][packet[1]] = HAL_GetTick();

	if (packet[1] == 1) { //РїСЂРёС€Р»Рё РґР°РЅРЅС‹Рµ РѕС‚ РџР” СЃ Р°РґСЂРµСЃРѕРј 1
		fault_unset(FAULT_PD_CON_1);
	}
	else if (packet[1] == 2) { //РїСЂРёС€Р»Рё РґР°РЅРЅС‹Рµ РѕС‚ РџР” СЃ Р°РґСЂРµСЃРѕРј 2
		fault_unset(FAULT_PD_CON_2);
	}
	else return;

	//pd_send_tick[packet[1]] = 0;


	if (packet[0] != setting.uart_local_addr) { //РїСЂРёС€Р»Рё РґР°РЅРЅС‹Рµ РѕС‚ РџР” РЅРѕ РЅРµ Рє РјРѕРµРјСѓ РџРЈ
		packet[1] = setting.uart_local_addr + packet[1]; //С€Р»СЋР·
		fill_crc16(packet, packet_len);
		uart4_uart7_snd(packet, packet_len);
		return;
	}

	pd_parse_rcv(mux, packet, packet_len, 0);
}


static uart6_mux_t mux_pd_snd_query = UART6_MUX_P1C;

void pd_transmit_piv_nomux(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{

	uart6_mux_t mux;

	if (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P1C][daddr]) < 20000) { //РµСЃР»Рё РїСЂРёС…РѕРґРёР»Рё РїР°РєРµС‚С‹ РЅРµРґР°РІРЅРѕ
		mux = UART6_MUX_P1C;
	}
	else if (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P2C][daddr]) < 20000) { //РµСЃР»Рё РїСЂРёС…РѕРґРёР»Рё РїР°РєРµС‚С‹ РЅРµРґР°РІРЅРѕ
		mux = UART6_MUX_P2C;
	} else {
		mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
		mux = mux_pd_snd_query;
	}

	uart6_transmit_piv(mux, daddr, saddr , cmd, cmd_data, cmd_data_len, pd_tx_buf);
}



//const uint8_t pd_query_packet_1[7] = {1, 0, 0, 1, PD_CMD_QUERY, 0, 0};
//const uint8_t pd_query_packet_2[7] = {2, 0, 0, 1, PD_CMD_QUERY, 0, 0};

void pd_snd_query(void)
{


	switch (setting.pd_count) {
		case 0: return;
		case 1: //РџР”1
			pd_send_addr = 1;
			mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
		break;
		case 2: //РџР”2
			pd_send_addr = 2;
			mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
		break;
		case 3: //РџР”1 Рё РџР”2
			if (pd_send_addr == 2) {
				pd_send_addr = 1;
				mux_pd_snd_query = (mux_pd_snd_query == UART6_MUX_P1C ? UART6_MUX_P2C : UART6_MUX_P1C);
			}
			else pd_send_addr = 2;

		break;
	}


	BOOL t1, t2;

	t1 = (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P1C][pd_send_addr]) > 20000);
	t2 = (HAL_DiffTick(mux_pd_rcv_tick[UART6_MUX_P2C][pd_send_addr]) > 20000);


	if (t1 && t2) {
		fault_set(FAULT_PD_CON_1 + (pd_send_addr - 1));
	}
	else if (t1 && !t2) {
		fault_set(FAULT_PD_RING);
	}
	else if (!t1 && t2) {
		fault_set(FAULT_PD_RING);
	}
	else {
		fault_unset(FAULT_PD_RING);
	}

	uart6_transmit_piv(mux_pd_snd_query, pd_send_addr, setting.uart_local_addr, PD_CMD_QUERY, NULL, 0, pd_tx_buf);


	if (pd_send_tick[pd_send_addr]) {

		if (HAL_DiffTick(pd_send_tick[pd_send_addr]) > (setting.pd_count == 3 ? 2 : 1) * 100000) {
			if (pd_send_addr == 1) fault_set(FAULT_PD_CON_1);
			else fault_set(FAULT_PD_CON_2);
			pd_send_tick[pd_send_addr] = 0;
		}


	}

	pd_send_tick[pd_send_addr] = HAL_GetTickNoZero();

}

void pd_init(void)
{
	mux_pd_rcv_tick[UART6_MUX_P1C][1] = HAL_GetTick();
	mux_pd_rcv_tick[UART6_MUX_P1C][2] = HAL_GetTick();
	mux_pd_rcv_tick[UART6_MUX_P2C][1] = HAL_GetTick();
	mux_pd_rcv_tick[UART6_MUX_P2C][2] = HAL_GetTick();
}

*/
