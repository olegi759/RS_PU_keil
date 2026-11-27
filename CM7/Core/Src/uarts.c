/*
 * uart.c
 *
 *  Created on: 6 РјР°СЂ. 2024 Рі.
 *      Author: Rafael Boltachev
 */


#include "uarts.h"
#include <string.h>
#include <stdio.h>
#include "math_util.h"
#include "stm_util.h"
#include "timer_util.h"
#include "str_util.h"
#include "opus_util.h"
#include "pcm1753.h"
#include "pd.h"
#include "pd_data.h"
#include "playopus.h"
#include "mchs_mic.h"
#include "net_util.h"
#include "pu.h"
#include "cm7core.h"
#include "fault_data.h"
#include "common.h"
#include "playmp3.h"
#include "opus_buffer.h"

//RS1 СЌС‚Рѕ UART4

#define DEBUG_UART4_UART7	1
#define DEBUG_UART4_UART7_ONLY		  PU_CMD_CHANGE_MODE//PD_CMD_PING//PD_CMD_SPISOK_PU


//РІСЂРµРјСЏ РїСЂРёРЅСЏС‚РёРµ РїР°РєРµС‚Р° РїРѕ РєРѕР»СЊС†Сѓ
static uint32_t rcv_ring_tick[10]	=	{0};

dynamic_buf_t uarts_rx_buf = DYNAMIC_BUF_STATIC_INIT(60 * 1024) ;

#define MAX_COUNT_PACKET_SIZE	128
#define MAX_SOUND_PACKET_SIZE	(OPUS_MAX_ENCODE_SIZE + 10)

static uint8_t p2_sound_rx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t p1_sound_rx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t ext1_count_rx_buf[MAX_COUNT_PACKET_SIZE];
static uint8_t ext1_sound_rx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t ext2_count_rx_buf[MAX_COUNT_PACKET_SIZE];
static uint8_t ext2_sound_rx_buf[MAX_SOUND_PACKET_SIZE];

static uint8_t p2_sound_tx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t p1_sound_tx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t ext1_count_tx_buf[MAX_COUNT_PACKET_SIZE];
static uint8_t ext1_sound_tx_buf[MAX_SOUND_PACKET_SIZE];
static uint8_t ext2_count_tx_buf[MAX_COUNT_PACKET_SIZE];
static uint8_t ext2_sound_tx_buf[MAX_SOUND_PACKET_SIZE];


uint32_t recv_opus_cnt = 0;


static uint16_t sound_tx_id = 0;
static uint16_t sound_rx_id = 0;

uint16_t count_tx_id = 0;
static uint16_t count_rx_id = 0;

//РІСЂРµРјСЏ РїСЂРёС…РѕРґР° РїР°РєРµС‚Р° РїРѕ AUDIO uart8 РёР»Рё uart5
//static uint32_t uart8_uart5_active_tick = 0;

//С‡РµСЂРµР· РєР°РєРѕР№ uart Р±СѓРґРµС‚ РёРґС‚Рё РєРѕРјРјР°РЅРґР°
//static UART_HandleTypeDef * uarts_transmit_cmd_huart = &huart4;


static uint32_t uart8_uart5_snd_ff_tick = 0;
static uint32_t uart4_uart7_snd_ff_tick = 0;


typedef struct __huart_param {
	UART_HandleTypeDef *huart;
	uint8_t number;
	uint8_t * rx_buf;
	uint16_t rx_buf_size;
	uint8_t * tx_buf;
	uint16_t tx_buf_size;
} huart_param_t;

const huart_param_t huart_params[6] = {
		{ &huart1, 1, p2_sound_rx_buf, sizeof(p2_sound_rx_buf), p2_sound_tx_buf, sizeof(p2_sound_tx_buf) },
		{ &huart3, 3, p1_sound_rx_buf, sizeof(p1_sound_rx_buf), p1_sound_tx_buf, sizeof(p1_sound_tx_buf) },
		{ &huart4, 4, ext1_count_rx_buf, sizeof(ext1_count_rx_buf), ext1_count_tx_buf, sizeof(ext1_count_tx_buf) },
		{ &huart5, 5, ext1_sound_rx_buf, sizeof(ext1_sound_rx_buf), ext1_sound_tx_buf, sizeof(ext1_sound_tx_buf) },
		{ &huart7, 7, ext2_count_rx_buf, sizeof(ext2_count_rx_buf), ext2_count_tx_buf, sizeof(ext2_count_tx_buf) },
		{ &huart8, 8, ext2_sound_rx_buf, sizeof(ext2_sound_rx_buf), ext2_sound_tx_buf, sizeof(ext2_sound_tx_buf) }
};


const huart_param_t * get_huart_param_by_handle(UART_HandleTypeDef *huart)
{
	int i;

	for (i = 0; i < 6; i++) {
		if (huart_params[i].huart == huart) return &huart_params[i];
	}
	return NULL;
}

const huart_param_t * get_huart_param_by_number(uint8_t number)
{
	int i;

	for (i = 0; i < 6; i++) {
		if (huart_params[i].number == number) return &huart_params[i];
	}
	return NULL;
}

void uarts_abort_huart(UART_HandleTypeDef * huart)
{
	HAL_UART_AbortReceive_IT(huart);
	HAL_UART_AbortReceive(huart);
	HAL_UART_Abort(huart);
}

void UART_flush(UART_HandleTypeDef *huart) {
  // Flush UART RX buffer if RXNE is set
  if READ_BIT(huart->Instance->ISR, UART_FLAG_RXNE) {
    SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);
  }

  // Not available on F030xx devices!
  SET_BIT(huart->Instance->RQR, UART_TXDATA_FLUSH_REQUEST);

  // Clear All Errors (if needed)
  if (READ_BIT(huart->Instance->ISR, UART_FLAG_ORE | UART_FLAG_FE | UART_FLAG_NE)) {

	__HAL_UART_CLEAR_PEFLAG(huart);
	__HAL_UART_CLEAR_OREFLAG(huart);
	__HAL_UART_CLEAR_FEFLAG(huart);
	__HAL_UART_CLEAR_NEFLAG(huart);
	__HAL_UART_CLEAR_IDLEFLAG(huart);
	__HAL_UART_CLEAR_TXFECF(huart);

  }
}

void uarts_ReceiveToIdle_IT_by_param(const huart_param_t * huart_param)
{
	//HAL_UARTEx_ReceiveToIdle_IT(huart_param->huart, huart_param->rx_buf, huart_param->rx_buf_size);

	HAL_StatusTypeDef res;

	//res = HAL_UARTEx_ReceiveToIdle_DMA(huart_param->huart, huart_param->rx_buf, huart_param->rx_buf_size);
	res = HAL_UARTEx_ReceiveToIdle_IT(huart_param->huart, huart_param->rx_buf, huart_param->rx_buf_size);

	if (res != HAL_OK) {
		ulog_fmt(CONM"ERROR HAL_UARTEx_ReceiveToIdle(%d)=%d"CRLF, huart_param->number, res);

		uarts_abort_huart(huart_param->huart);
		UART_flush(huart_param->huart);

		//HAL_UARTEx_ReceiveToIdle_DMA(huart_param->huart, huart_param->rx_buf, huart_param->rx_buf_size);
		HAL_UARTEx_ReceiveToIdle_IT(huart_param->huart, huart_param->rx_buf, huart_param->rx_buf_size);

	}
}




void uarts_transmit_it_by_param(const huart_param_t * huart_param, uint8_t * packet, uint16_t packet_len)
{
	uint16_t len;





	//if (huart_param->huart->gState == HAL_UART_STATE_BUSY_TX) {
	//	ulog_fmt(CONM"uart busy:%d"CRLF,  huart_param->number);
		HAL_UART_AbortTransmit(huart_param->huart);
		//return;
	//}


	len = packet_len < huart_param->tx_buf_size ? packet_len : huart_param->tx_buf_size;
	memcpy(huart_param->tx_buf, packet, len);

	HAL_UART_Transmit_DMA(huart_param->huart, huart_param->tx_buf, len);
	//uarts_transmit_it_tick = HAL_GetTick();


	//if (packet[0] == 0xFF) { //РµСЃР»Рё РїР°РєРµС‚ С€РёСЂРѕРєРѕРІРµС‰Р°С‚РµР»СЊРЅС‹Р№
	//	rcv_ring_tick[huart_param->number] = HAL_GetTickNoZero();
	//}

#ifdef DEBUG_UART4_UART7

	if ((huart_param->number == 4) || (huart_param->number == 7)) {

#ifdef DEBUG_UART4_UART7_ONLY
		if (packet[4] == DEBUG_UART4_UART7_ONLY) {
#endif
			ulog_fmt(CONM"t4_7(%d %d) "CRLF, huart_param->number,  packet_len);
			ulogmem(packet, packet_len, FALSE);
			ulog(CRLF);
#ifdef DEBUG_UART4_UART7_ONLY
		}
#endif
	}
#endif

}

void uarts_transmit_it(UART_HandleTypeDef *huart, uint8_t * packet, uint16_t packet_len)
{
	const huart_param_t * huart_param;

	huart_param = get_huart_param_by_handle(huart);
	if (!huart_param) return;

	uarts_transmit_it_by_param(huart_param, packet, packet_len);
}

void uarts_transmit_it_by_num(uint8_t huart_number, uint8_t * packet, uint16_t packet_len)
{
	const huart_param_t * huart_param;

	/* Third ring line - I2C2: sent through CM4 with the same frame */
	if (huart_number == UART_LINE_I2C2) {
		cm7core_i2c_send(packet, packet_len);
		return;
	}

	huart_param = get_huart_param_by_number(huart_number);
	if (!huart_param) return;

	uarts_transmit_it_by_param(huart_param, packet, packet_len);
}





void pd_check_audio_param(uint8_t cmd)
{
	uint32_t frequency;
	BOOL stereo;

	if (!pd_get_audio_param_from_command(cmd, &frequency, &stereo)) {
		ulog_fmt(CONM"error cmd:%d"CRLF, cmd);
		return;
	}

	if ((pcm1753_GetFrequency() == frequency) && (pcm1753_GetStereo() == stereo)) return;

	ulog_fmt(CONM"pd change audio param cmd:%d f:%lu s:%s"CRLF, cmd, (unsigned long)pcm1753_GetFrequency(), bool2str(pcm1753_GetStereo()));
	pu_prepare_audio_play(frequency, stereo);

}

/*
//РїРѕСЃС‹Р»РєР° РїРѕ РїСЂРѕС‚РѕРєРѕР»Сѓ РєРѕРјРјР°РЅРґС‹ С‡РµСЂРµР· uart4 РёР»Рё uart7
void uarts_transmit_piv(uint8_t huart_number, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uint8_t packet[5 + cmd_data_len + 2];
	uint16_t packet_len;
	packet_len = create_piv_packet_cmd(packet, daddr, saddr, cmd, cmd_data, cmd_data_len);
	uarts_transmit_it_by_num(huart_number,  packet, packet_len);
}
*/

void uarts_transmit_piv2(uint8_t huart_number, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len, uint16_t packet_id)
{
	uint8_t packet[5 + cmd_data_len + 2];
	uint16_t packet_len;
	packet_len = create_piv_packet_cmd2(packet, daddr, saddr, cmd, cmd_data, cmd_data_len, packet_id);
	uarts_transmit_it_by_num(huart_number,  packet, packet_len);
}


void uarts_transmit_piv(uint8_t huart_number, uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{

	if ((huart_number == 5) || (huart_number == 8) ) {
		sound_tx_id++;
		uarts_transmit_piv2(huart_number, daddr, saddr, cmd, cmd_data, cmd_data_len, sound_tx_id);
	}
	else if ((huart_number == 4) || (huart_number == 7) ) {
		count_tx_id++;
		uarts_transmit_piv2(huart_number, daddr, saddr, cmd, cmd_data, cmd_data_len, count_tx_id);
	}
}


void HAL_UARTS_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	const huart_param_t * huart_param;



	huart_param = get_huart_param_by_handle(huart);
	if (!huart_param) return;

	//ulog_fmt("r(%d:%d)"CRLF, huart_param->number, Size);

	if (Size < 7) return;

	//if (Size < 7) {
	//	if ((huart_param->number == 4) || (huart_param->number == 7)) return; //РљРѕРјРјР°РЅРґС‹ РѕС‚ uart4 Рё uart7 СЃР»РµРІР° Рё СЃРїСЂР°РІР°
	//	if ((huart_param->number == 8) || (huart_param->number == 5)) return; //Р—РІСѓРє РѕС‚ uart8 Рё uart5 РЎР»РµРІР° Рё СЃРїСЂР°РІР°
	//}


	huart_param->rx_buf[Size] = huart_param->number;

	uarts_rx_buf.busy = TRUE;
	dynamic_buf_push(&uarts_rx_buf, huart_param->rx_buf, Size + 1);
	uarts_rx_buf.busy = FALSE;

	uarts_ReceiveToIdle_IT_by_param(huart_param);
}




// ------------------------------------------------------------------------------
// -------------------------------- UART4 AND UART7 -----------------------------
// ------------------------------------------------------------------------------


//РІСЂРµРјСЏ РїСЂРёРµРјР° РїР°РєРµС‚Р° РїРѕ РєРѕР»СЊС†Сѓ 0 - UART4, 1 - UART7
//static uint32_t uart4_uart7_rcv_ring_tick[2]	=	{0};



//РЅР° РєР°РєРѕР№ uart4 РёР»Рё uart7 РїСЂРёС…РѕРґРёС‚ РєРѕРјРјР°РЅРґР°
//static uint8_t uart4_uart7_rcv_huart_number[26] = { [0 ... 25] = 0 };
//static uint32_t uart4_uart7_rcv_huart_number_tick[26] = { [0 ... 25] = 0 };







static uint8_t pu_cmd_change_mode_state = 0;
static uint8_t pu_cmd_change_mode_addr = 0;



void uart4_uart7_snd(uint8_t * packet, uint16_t packet_len)
{
	uarts_transmit_it_by_num(4, packet, packet_len);
	uarts_transmit_it_by_num(7, packet, packet_len);
	uarts_transmit_it_by_num(UART_LINE_I2C2, packet, packet_len);

	if (packet[0] == 0xFF) uart4_uart7_snd_ff_tick = HAL_GetTickNoZero();
}

/* Forward frame to the other ring lines (except the source one):
   UART4, UART7 and I2C2 are equal ring lines */
static void uart4_uart7_snd_others(uint8_t src_line, uint8_t * packet, uint16_t packet_len)
{
	if (src_line != 4) uarts_transmit_it_by_num(4, packet, packet_len);
	if (src_line != 7) uarts_transmit_it_by_num(7, packet, packet_len);
	if (src_line != UART_LINE_I2C2) uarts_transmit_it_by_num(UART_LINE_I2C2, packet, packet_len);
}

void uart4_uart7_snd_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{

	uint8_t packet[5 + cmd_data_len + 2];
	uint16_t packet_len;

	count_tx_id++;
	packet_len = create_piv_packet_cmd2(packet, daddr, saddr, cmd, cmd_data, cmd_data_len, count_tx_id);

	uart4_uart7_snd(packet, packet_len);
}


void uart4_uart7_recv_change_mode(uint8_t * packet, uint16_t packet_len)
{

	uint32_t lines, flines;
	int i;
	uint8_t saddr_index;
	char file_name[64];
	pu_small_info_t * pu_small_info;


	pu_small_info = (pu_small_info_t *)&packet[5];

	saddr_index = pu_small_info->addr / 10;



	if (pu_small_info->mode != pu_small_infos[saddr_index].mode) {
		ulog_fmt(CONM"PU%d CHANGE MODE %s->%s"CRLF, pu_small_info->addr, pu_mode2str(pu_small_infos[saddr_index].mode), pu_mode2str(pu_small_info->mode));
	}



	switch (pu_small_info->mode) {
		case PU_MODE_VESHANIE_MCHS:
			if (!setting.netfunc_mchs.pu_addr) break;
			if (!setting.netfunc_mchs.sound_lines) break;
			if (setting.netfunc_mchs.pu_addr != pu_small_info->addr) break;
			if (pu_cmd_change_mode_state) break;

			pu_prepare_audio_play(MCHS_MIC_AUDIO_FREQUENCY, FALSE);
			pu_enable_sound_lines(setting.netfunc_mchs.sound_lines);
			pu_cmd_change_mode_state = 1;
			pu_cmd_change_mode_addr = pu_small_info->addr;
			ulog_fmt(CONM"cmd_change_state saddr:%d mode:PU_MODE_VESHANIE_MCHS"CRLF, pu_small_info->addr);

		break;
		case PU_MODE_VESHANIE_POJAR:

			if (pu_cmd_change_mode_state) break;

			flines = pu_small_info->lines;


			lines = 0;
			for (i = 1; i <= 10; i++) {
				if (!setting.netfunc_fire[i].pu_addr || !setting.netfunc_fire[i].fire_line || !setting.netfunc_fire[i].sound_lines || (setting.netfunc_fire[i].file[0] == '\0')) continue;
				if (setting.netfunc_fire[i].pu_addr != pu_small_info->addr) continue;
				if (!bitget(&flines, (setting.netfunc_fire[i].fire_line - 1))) continue;
				strcpy_s(file_name, sizeof(file_name), setting.netfunc_fire[i].file);
				lines |= setting.netfunc_fire[i].sound_lines;
			}

			if (!lines) break;

			if (!cm7core_file_exist(file_name)) break;

			pu_enable_sound_lines(lines);

			pu_play_audio_file(file_name);
			pu_cmd_change_mode_state = 2;
			pu_cmd_change_mode_addr = pu_small_info->addr;
			ulog_fmt("cmd_change_state saddr:%d mode:PU_MODE_VESHANIE_POJAR fire_lines:%lu"CRLF, pu_small_info->addr, (unsigned long)flines);

		break;
		default:
			if (pu_cmd_change_mode_state) {

				if (pu_small_info->addr == pu_cmd_change_mode_addr) {

					if (pu_cmd_change_mode_state == 2) {
						playopus_stop();
						playmp3_stop();
					}
					pu_enable_sound_lines(0);
					pu_cmd_change_mode_state = 0;
					pu_cmd_change_mode_addr = 0;
				}
			}
		break;

	}



	pu_small_infos[saddr_index].addr = pu_small_info->addr;
	pu_small_infos[saddr_index].lines_count = pu_small_info->lines_count;
	pu_small_infos[saddr_index].mode = pu_small_info->mode;
	pu_small_infos[saddr_index].lines = pu_small_info->lines;
	pu_small_infos_tick[saddr_index] = HAL_GetTickNoZero();

}


void uart4_uart7_rcv_spisok_pu(uint8_t huart_number, uint8_t * packet, uint16_t packet_len)
{
	uint8_t buf[MAX_PACKET_SIZE];
	int i, j;
	pu_small_info_t  * pu_small_info;
	uint8_t saddr_index;


	i = 5;
	while (i < packet_len - 2) {
		pu_small_info = (pu_small_info_t  *)&packet[i];
		saddr_index = pu_small_info->addr / 10;

		pu_small_infos[saddr_index].addr = pu_small_info->addr;
		pu_small_infos[saddr_index].lines_count = pu_small_info->lines_count;
		pu_small_infos[saddr_index].mode = pu_small_info->mode;
		pu_small_infos[saddr_index].lines = pu_small_info->lines;


		i += sizeof(pu_small_info_t);
	}


	j = 0;
	for (i = 5; i < packet_len - 2; i++) {
		buf[j] = packet[i];
		j++;
	}


	pu_get_local_small_info((pu_small_info_t  *)&buf[j]);
	j += sizeof(pu_small_info_t);


	/* answer to PU list request - to the other ring lines */
	{
		uint8_t reply[5 + j + 2];
		uint16_t reply_len;

		reply_len = create_piv_packet_cmd(reply, packet[0], packet[1], PD_CMD_SPISOK_PU, buf, j);
		uart4_uart7_snd_others(huart_number, reply, reply_len);
	}


}

/*

void uart4_uart7_ring_check(void)
{
	if (rcv_ring_tick[4] < 20000) { //UART4
		cm7core_fault_unset(FAULT_PU_RING_CMD);
	}
	else if (rcv_ring_tick[7] < 20000) { //UART7
		cm7core_fault_unset(FAULT_PU_RING_CMD);
	}
	else { //СЂР°Р·СЂС‹РІ РєРѕР»СЊС†Р°
		cm7core_fault_set(FAULT_PU_RING_CMD);
	}
}

*/


//РљРѕРјРјР°РЅРґС‹ РѕС‚ uart4 Рё uart7 СЃР»РµРІР° Рё СЃРїСЂР°РІР°
void uart4_uart7_rcv(uint8_t huart_number, uint8_t * packet, uint16_t packet_len)
{

	uint16_t rx_id;

	#ifdef DEBUG_UART4_UART7

#ifdef DEBUG_UART4_UART7_ONLY
	if (packet[4] == DEBUG_UART4_UART7_ONLY) {
#endif
	ulog_fmt("r4_7(%d %d) "CRLF, huart_number,  packet_len);
	ulogmem(packet, packet_len, FALSE);
	ulog(CRLF);
#ifdef DEBUG_UART4_UART7_ONLY
}
#endif

#endif

	if (!is_correct_crc16(packet, packet_len)) return;




	if ( packet[1] == setting.uart_local_addr ) { //РїР°РєРµС‚ РѕС‚ СЃРµР±СЏ
		rcv_ring_tick[huart_number] = HAL_GetTick();
		return;
	}

	if ( (packet[0] == setting.uart_local_addr + 1) || (packet[0] == setting.uart_local_addr + 2) ) { //РїР°РєРµС‚ РёР· СЃРµС‚Рё Рє РјРѕРёРј РџР”

		rx_id = (packet[2] << 8) | packet[3];
		if (rx_id) {
			if (rx_id <= count_rx_id) { //РґСѓР±Р»РёСЂСѓСЋС‰РёР№ РїР°РєРµС‚
				if (count_rx_id - rx_id < 100) return;
			}
		}
		count_rx_id = rx_id;

		packet[0] -= setting.uart_local_addr;
		rx_id = packet_len - 6;
		packet[2] = rx_id >> 8;
		packet[3] = rx_id & 0xFF;
		fill_crc16(packet, packet_len);
		cm7core_pd_send(packet, packet_len); //РїРµСЂРµСЃС‹Р»Р°РµРј РЅР° РџР”
		return;
	}

	if ((packet[0] != setting.uart_local_addr) && (packet[0] != 0xFF) ) { //РїР°РєРµС‚ РЅРµ РјРЅРµ
		uart4_uart7_snd_others(huart_number, packet, packet_len); //РЅРµ СЃРІРѕР№ РїР°РєРµС‚ РїРµСЂРµРґР°РµРј РґР°Р»СЊС€Рµ С‡РµСЂРµР· С€Р»СЋР·
		return;
	}

	//С‚СѓС‚ РїР°РєРµС‚С‹ РјРЅРµ РёР»Рё С€РёСЂРѕРєРѕРІРµС‰Р°С‚РµР»СЊРЅС‹Сѓ

	rx_id = (packet[2] << 8) | packet[3];
	if (rx_id) {
		if (rx_id <= count_rx_id) { //РґСѓР±Р»РёСЂСѓСЋС‰РёР№ РїР°РєРµС‚
			if (count_rx_id - rx_id < 100) return;
		}
	} else {
		if (!count_rx_id) return;
	}
	count_rx_id = rx_id;




	//ulog_fmt("rcv_4_7_0xFF"CRLF);
	if (packet[0] == 0xFF) {

		switch (packet[4]) {

			case PU_CMD_CHANGE_MODE:
				uart4_uart7_recv_change_mode(packet, packet_len);
			break;

			case PU_CMD_NET_INFO_UPDATE:
				pu_netinfo[0] = packet[5];
				pu_netinfo[1] = packet[6];
				pu_netinfo[2] = packet[7];
			break;


			//case PD_CMD_PING:
			//	uarts_transmit_piv(huart_number, packet[1], setting.uart_local_addr, PD_CMD_PONG, NULL, 0);
			//	ulog_fmt(CONM"tc(%d)-> PONG"CRLF, huart_number, packet[1]);
			//break;

			//case PD_CMD_SPISOK_PU:

			//	if ( (packet[1] == setting.uart_local_addr + 1) || (packet[1] == setting.uart_local_addr + 2) ) { //РїР°РєРµС‚ РёР· СЃРµС‚Рё Рє РјРѕРёРј РџР”
			//		packet[0] = packet[1] - setting.uart_local_addr;
			//		packet[1] = setting.uart_local_addr;
			//		packet[4] = PD_CMD_SPISOK_PU_RES;
			//		fill_crc16(packet, packet_len);
			//		cm7core_pd_send(packet, packet_len);
			//		return;
			//	}

				//uart4_uart7_rcv_spisok_pu(huart_number, packet, packet_len);

			//return;
		}
	}

	if (packet[0] == setting.uart_local_addr) {
		//РІ pd_rcv РІС…РѕРґРёРј С‚РѕР»СЊРєРѕ СЃ Р»РѕРєР°Р»СЊРЅС‹Рј Р°РґСЂРµСЃРѕРј
		pd_rcv(packet, packet_len, huart_number);
	} else {
		uart4_uart7_snd_others(huart_number, packet, packet_len); //РЅРµ СЃРІРѕР№ РїР°РєРµС‚ РїРµСЂРµРґР°РµРј РґР°Р»СЊС€Рµ С‡РµСЂРµР· С€Р»СЋР·
	}

}




void uarts_transmit_dejurniy(uint8_t daddr)
{
	uart4_uart7_snd_piv(daddr, setting.uart_local_addr, PD_CMD_DEJURNIY, NULL, 0);

}
void uarts_transmit_veshanie_pd(uint8_t daddr, uint32_t frequency, BOOL stereo, uint32_t sound_lines)
{
	pd_audio_param_cmd_data_t pd_audio_data;

	pd_audio_data.frequency = HTONL(frequency);
    pd_audio_data.stereo = stereo;
	pd_audio_data.sound_lines = sound_lines;

	uart4_uart7_snd_piv(daddr, setting.uart_local_addr, PD_CMD_VESHANIE_PD, (uint8_t *)&pd_audio_data, sizeof(pd_audio_data));
}


//РїРѕСЃС‹Р»РєР° РґР»СЏ netfun СЃРµС‚РµРІС‹С… С„-С†РёР№ PU_CMD_CHANGE_STATE
void uarts_transmit_change_mode(void)
{

	pu_small_info_t small_info;

	//РўРђРљ РџРћРҐРћР–Р• РќР•Р›Р¬Р—РЇ memset(&small_info, 0, sizeof(small_info));


	small_info.addr = setting.uart_local_addr;
	small_info.mode = pu_mode;
	small_info.lines_count = setting.lines_count;
	small_info.lines = pu_get_fire_lines();


	uart4_uart7_snd_piv(0xFF, setting.uart_local_addr, PU_CMD_CHANGE_MODE, (uint8_t *)&small_info, sizeof(small_info));
}


void uart4_uart7_ring_check(void)
{
	if (!uart4_uart7_snd_ff_tick) return;
	if (HAL_DiffTick(uart4_uart7_snd_ff_tick) < 1000) return;

	if ((rcv_ring_tick[4] < uart4_uart7_snd_ff_tick) && (rcv_ring_tick[7] < uart4_uart7_snd_ff_tick) ) {
		cm7core_fault_set(FAULT_PU_RING_CMD);
	} else {
		cm7core_fault_unset(FAULT_PU_RING_CMD);
	}
}


// ------------------------------------------------------------------------------
// -------------------------------- UART8 AND UART5 -----------------------------
// ------------------------------------------------------------------------------

//static uint16_t uart8_uart5_snd_packet_id = 0;
//static uint16_t uart8_uart5_rcv_packet_id = 0;



//РІСЂРµРјСЏ РїСЂРёРµРјР° РїР°РєРµС‚Р° РїРѕ РєРѕР»СЊС†Сѓ 0 - UART8, 1 - UART5
//static uint32_t uart8_uart5_rcv_ring_tick[2] =	{0};

//static uint16_t last_rx_ids[9] = {0};

//Р—РІСѓРє РѕС‚ uart8 Рё uart5 РЎР»РµРІР° Рё СЃРїСЂР°РІР°
void uart8_uart5_rcv(uint8_t huart_number, uint8_t * packet, uint16_t packet_len)
{

	uint16_t rx_id;
	uint8_t pu_net_info_bit;

	//ulog_fmt(CONM"r8_5(%d %d) ", huart_number,  packet_len);
	//ulogmem(packet, packet_len, FALSE);
	//ulog(CRLF);

	if (!is_correct_crc16(packet, packet_len)) return;


	//if (rx_id == sound_rx_id) return; //РґСѓР±Р»РёСЂСѓСЋС‰РёР№ РїР°РєРµС‚
	//sound_rx_id = rx_id;

	if (packet[1] == setting.uart_local_addr) { //СЃРІРѕР№ РїР°РєРµС‚ РїРѕ РєСЂСѓРіСѓ
		rcv_ring_tick[huart_number] = HAL_GetTick();
		return; //СЌС‚Рѕ РјРѕР№ РїР°РєРµС‚ РїРѕ РєРѕР»СЊС†Сѓ
	}


	if ( (packet[0] != 0xFF) &&  (packet[0] != setting.uart_local_addr)) { //РЅРµ РјРЅРµ
		uarts_transmit_it_by_num((huart_number == 5 ? 8 : 5), packet, packet_len); //РЅРµ СЃРІРѕР№ РїР°РєРµС‚ РїРµСЂРµРґР°РµРј РґР°Р»СЊС€Рµ С‡РµСЂРµР· С€Р»СЋР·
		return;
	}

	//С‚СѓС‚ daddr РёР»Рё РјРѕР№ РёР»Рё С€РёСЂРѕРєРѕРІРµС‰Р°С‚РµР»СЊРЅС‹Р№

	rx_id = (packet[2] << 8) | packet[3];

	//if ((rx_id == last_rx_ids[8]) || (rx_id == last_rx_ids[5])) {
		//last_rx_ids[huart_number] = rx_id;
		//return;
	//}

	//last_rx_ids[huart_number] = rx_id;

	//fulog_fmt("r_%d(%d)"CRLF, huart_number, rx_id);

	//fulog_fmt("r_%d(%d %d)"CRLF, huart_number, rx_id, sound_rx_id);

	if (rx_id) {
		if (rx_id <= sound_rx_id) { //РґСѓР±Р»РёСЂСѓСЋС‰РёР№ РїР°РєРµС‚
			if (sound_rx_id - rx_id < 100) return;
		}
	} else {
		if (!sound_rx_id) return;
	}
	sound_rx_id = rx_id;

	//fulog_fmt("rok_%d(%d %d)"CRLF, huart_number, rx_id, sound_rx_id);

	recv_opus_cnt++;


	switch (packet[4]) {
		case PD_CMD_OPUS48mono:
		case PD_CMD_OPUS16mono:
		case PD_CMD_OPUS48ster:
		case PD_CMD_OPUS16ster:
			if ((pu_mode == PU_MODE_VESHANIE_PD) || (pu_mode == PU_MODE_RUCHNOI_PUSK_PD) || (pu_cmd_change_mode_state == 1)) {

				pu_net_info_bit = setting.uart_local_addr / 10;
				if (pu_net_info_bit) {
					pu_net_info_bit--;
					if ( bitget(pu_netinfo, pu_net_info_bit)) break;
				}

				pd_check_audio_param(packet[4]);

				if (pcm1753_GetFrequency() > 16000) {
					opus_buffer_put(&packet[5], packet_len - 7);
				} else {
					pu_put_opus_in_pcm1753_now(&packet[5], packet_len - 7);
				}
				//fulog_fmt(CONM"r:%lu"CRLF, HAL_GetTick());
			}
		break;
	}

	if (packet[0] == 0xFF) { //РµСЃР»Рё С€РёСЂРѕРєРѕРІРµС€Р°С‚РµР»СЊРЅС‹Р№
		uarts_transmit_it_by_num((huart_number == 5 ? 8 : 5), packet, packet_len); //С€РёСЂРѕРєРѕРІРµС‰Р°С‚РµР»СЊРЅС‹Р№ РїР°РєРµС‚ РїРµСЂРµРґР°РµРј РґР°Р»СЊС€Рµ С‡РµСЂРµР· С€Р»СЋР·
	}


}

//#define UART8_UART5_SND_COUNT	10
//static uint8_t uart8_uart5_snd_cnt = UART8_UART5_SND_COUNT;
//static BOOL uart8_uart5_snd_num = FALSE;


//РџРѕСЃР»Р°С‚СЊ Р·РІСѓРє РІ uart8 РёР»Рё РІ uart5
//Р’ РїСЂРѕРІРѕРґР°С… РёРґРµС‚ РІРјРµСЃС‚Рµ: Р’Р›Р•Р’Рћ-(UART5 Рё UART4),   РџР РђР’Рћ-(UART8 Рё UART7)
void uart8_uart5_snd(uint8_t * packet, uint16_t packet_len)
{

	uarts_transmit_it_by_num(5, packet, packet_len);
	uarts_transmit_it_by_num(8, packet, packet_len);

	if (packet[0] == 0xFF) uart8_uart5_snd_ff_tick = HAL_GetTickNoZero();

}


void uart8_uart5_snd_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{

	uint8_t packet[5 + cmd_data_len + 2];
	uint16_t packet_len;

	sound_tx_id++;
	packet_len = create_piv_packet_cmd2(packet, daddr, saddr, cmd, cmd_data, cmd_data_len, sound_tx_id);

	uart8_uart5_snd(packet, packet_len);
}


void uart8_uart5_ring_check(void)
{
	if (!uart8_uart5_snd_ff_tick) return;
	if (HAL_DiffTick(uart8_uart5_snd_ff_tick) < 1000) return;

	if ((rcv_ring_tick[5] < uart8_uart5_snd_ff_tick) && (rcv_ring_tick[8] < uart8_uart5_snd_ff_tick) ) {
		cm7core_fault_set(FAULT_PU_RING_AUDIO);
	} else {
		cm7core_fault_unset(FAULT_PU_RING_AUDIO);
	}

}

/*
static uint32_t uart8_uart5_snd_ring_audio_check_tick = 0;

//РІС‹Р·С‹РІР°С‚СЊ СЂР°Р· РІ СЃРµРєСѓРЅРґСѓ
void uart8_uart5_snd_ring_check(void)
{

	if ((pu_mode == PU_MODE_VESHANIE_MCHS) || (pu_mode == PU_MODE_UPRAVLENIE_DRUGIMI_PU)) return;

	if (HAL_DiffTick(uart8_uart5_active_tick) < 1000) return; //РІРёРґРёРј Р°РєС‚РёРІРЅРѕСЃС‚СЊ РЅР° AUDIO

	if (HAL_DiffTick(uart8_uart5_snd_ring_audio_check_tick) > 1000) { //СЃ РјРѕРјРµРЅС‚Р° РѕС‚РїСЂР°РІРєРё РїСЂРѕС€Р»Р° СЃРµРєСѓРЅРґР°

		if ( (rcv_ring_tick[5] > 80000) || (rcv_ring_tick[8] > 80000) ){ //UART5 UART8 СЂР°Р·СЂС‹РІ РєРѕР»СЊС†Р°
			cm7core_fault_set(FAULT_PU_RING_AUDIO);
		}
		else {
			cm7core_fault_unset(FAULT_PU_RING_AUDIO);
		}
	}


	if (HAL_DiffTick(uart8_uart5_snd_ring_audio_check_tick) < 60000) return;

	uart8_uart5_snd_piv(0xFF, setting.uart_local_addr, PU_CMD_RING_AUDIO_CHECK, NULL, 0);
	uart8_uart5_snd_ring_audio_check_tick = HAL_GetTick();
}
*/




//uint8_t get_opus_cmd_from_pcm1753(void)
//{
//	return pd_get_opus_cmd_from_frequency_stereo(pcm1753_GetFrequency(), pcm1753_GetFrequency());
//}


//РїРѕСЃР»Р°С‚СЊ РґР°РЅРЅС‹Рµ РѕРїСѓСЃР° С‡РµСЂРµР· uart5 uart8
void uart8_uart5_send_opus_data_cmd(uint8_t * opus_data, int opus_data_len, uint8_t daddr, uint8_t cmd)
{

	uart8_uart5_snd_piv(daddr, setting.uart_local_addr, cmd, opus_data, opus_data_len);

	/*
	uint8_t packet[MAX_PACKET_SIZE];
	uint16_t packet_len;

	packet[0] = daddr;
	packet[1] = setting.uart_local_addr;
	packet_len = 1 + opus_data_len;

	sound_tx_id++;
	//packet[2] = packet_len >> 8;
	//packet[3] = packet_len & 0xFF;

	packet[2] = sound_tx_id >> 8;
	packet[3] = sound_tx_id & 0xFF;

	packet[4] = cmd;
	memcpy(&packet[5], opus_data, opus_data_len);
	packet_len = 7 + opus_data_len;
	fill_crc16(packet, packet_len);
	uart8_uart5_snd(packet, packet_len);
	*/
}

void uart8_uart5_send_opus_data(uint8_t * opus_data, int opus_data_len, uint8_t daddr, uint32_t frequency, BOOL stereo)
{
	uart8_uart5_send_opus_data_cmd(opus_data, opus_data_len, daddr, pd_get_opus_cmd_from_frequency_stereo(frequency, stereo));
}




// ------------------------------------------------------------------------------
// -------------------------------- UART1 AND UART3 -----------------------------
// ------------------------------------------------------------------------------


//Р—РІСѓРє РћС‚ РџР”1 Рё РџР”2
void uart1_uart3_rcv(uint8_t huart_number, uint8_t * packet, uint16_t  packet_len)
{
	if (!is_correct_crc16(packet, packet_len)) return;

	if ((packet[0] == setting.uart_local_addr) || (packet[0] == 0xFF)) {
		if ((pu_mode == PU_MODE_VESHANIE_PD) || (pu_mode == PU_MODE_RUCHNOI_PUSK_PD)) {


			pd_check_audio_param(packet[4]);
			if (pcm1753_GetFrequency() > 16000) {
				opus_buffer_put(&packet[5], packet_len - 7);
			} else {
				pu_put_opus_in_pcm1753_now(&packet[5], packet_len - 7);
			}


		}
	}


	if (packet[0] != setting.uart_local_addr) { //РЅРµ РёРЅРµ РёР»Рё С€РёСЂРѕРєРѕРІРµС‰Р°С‚РµР»СЊРЅС‹Р№
		//РїРѕСЃР»РІС‚СЊ Р·РІСѓРє РІ СЃРµС‚СЊ
		packet[1] = setting.uart_local_addr;
		sound_tx_id++;
		packet[2] = sound_tx_id >> 8;
		packet[3] = sound_tx_id & 0xFF;
		fill_crc16(packet, packet_len);
		uart8_uart5_snd(packet, packet_len);
	}

}

// ------------------------------------------------------------------------------
// -------------------------------- OTHER -----------------------------
// ------------------------------------------------------------------------------


void uarts_rcv_data(uint8_t huart_number, uint8_t * packet, uint16_t packet_len)
{
	//fulog_fmt("r %d %d"CRLF, huart_number, data_len);

	switch (huart_number) {
		case 1: uart1_uart3_rcv(huart_number, packet, packet_len); break;
		case 3: uart1_uart3_rcv(huart_number, packet, packet_len); break;
		case 4: uart4_uart7_rcv(huart_number, packet, packet_len); break;
		case 5: uart8_uart5_rcv(huart_number, packet, packet_len); break;
		case 7: uart4_uart7_rcv(huart_number, packet, packet_len); break;
		case 8: uart8_uart5_rcv(huart_number, packet, packet_len); break;
	}
}



void uarts_task_data_rx(void)
{
	uint8_t buf[MAX_SOUND_PACKET_SIZE];
	size_t len;

	if (!dynamic_buf_pop(&uarts_rx_buf, buf, sizeof(buf), &len)) return;

	uarts_rcv_data(buf[len - 1], buf, len - 1);
}




void uarts_task(void)
{
	if (DYNAMIC_BUF_IS_EMPTY(uarts_rx_buf)) return;
	if (DYNAMIC_BUF_GET_BUSY(uarts_rx_buf)) return;
	uarts_task_data_rx();
}





void uarts_ring_init(void)
{
	int i;

	for (i = 0; i < 10; i++) rcv_ring_tick[i] = HAL_GetTick();
}




void uarts_init_huart(UART_HandleTypeDef * huart)
{
	const huart_param_t * huart_param;

	uarts_abort_huart(huart);
	huart_param = get_huart_param_by_handle(huart);
	if (huart_param) {
		uarts_ReceiveToIdle_IT_by_param(huart_param);
	}
}


void uarts_init(void)
{


	PIN_CLR(UART_P1_SOUND_EN); //СЃ РџР” Р·РІСѓРє С‚РѕР»СЊРєРѕ РїСЂРёРЅРёРјР°РµРј
	PIN_CLR(UART_P2_SOUND_EN); //СЃ РџР” Р·РІСѓРє С‚РѕР»СЊРєРѕ РїСЂРёРЅРёРјР°РµРј



	uarts_abort_huart(&huart1);
	uarts_abort_huart(&huart3);
	uarts_abort_huart(&huart4);
	uarts_abort_huart(&huart5);
	uarts_abort_huart(&huart7);
	uarts_abort_huart(&huart8);



	UART_flush(&huart1);
	UART_flush(&huart3);
	UART_flush(&huart4);
	UART_flush(&huart5);
	UART_flush(&huart7);
	UART_flush(&huart8);


	uarts_ReceiveToIdle_IT_by_param(&huart_params[0]); //uart1
	uarts_ReceiveToIdle_IT_by_param(&huart_params[1]); //uart3
	uarts_ReceiveToIdle_IT_by_param(&huart_params[2]); //uart4
	uarts_ReceiveToIdle_IT_by_param(&huart_params[3]); //uart5
	uarts_ReceiveToIdle_IT_by_param(&huart_params[4]); //uart7
	uarts_ReceiveToIdle_IT_by_param(&huart_params[5]); //uart8

	uarts_ring_init();
}


/* Periodic ring walk: UART4 -> UART7 -> I2C2 (round robin).
   Every step sends the same PING frame; RX/processing goes through the
   same uart4_uart7_rcv() logic as for UART4/UART7. */
static uint8_t uarts_ring_line = 0;

void uarts_send_ring(void)
{
	uint8_t line;
	uint8_t packet[7];
	uint16_t packet_len;

	switch (uarts_ring_line) {
		case 0:  line = 4; break;
		case 1:  line = 7; break;
		default: line = UART_LINE_I2C2; break;
	}

	uarts_ring_line++;
	if (uarts_ring_line >= 3) uarts_ring_line = 0;

	packet_len = create_piv_packet_cmd(packet, setting.uart_local_addr, setting.uart_local_addr, PD_CMD_PING, NULL, 0);
	uarts_transmit_it_by_num(line, packet, packet_len);
}


/*

//static BOOL uarts_ring_uart_left_right = FALSE;

void uarts_send_ring(void)
{

	uarts_ring_uart_left_right = !uarts_ring_uart_left_right;

	uarts_transmit_piv(uarts_ring_uart_left_right ? 4 : 7, setting.uart_local_addr, setting.uart_local_addr, PD_CMD_PING, NULL, 0);
	uarts_transmit_piv(uarts_ring_uart_left_right ? 8 : 5, setting.uart_local_addr, setting.uart_local_addr, PD_CMD_PING, NULL, 0);


	if (HAL_DiffTick(uart8_uart5_rcv_ring_tick) > 20000) {
		fault_set(FAULT_PU_RING_AUDIO);
	} else {
		fault_unset(FAULT_PU_RING_AUDIO);
	}

	if (HAL_DiffTick(uart4_uart7_rcv_ring_tick) > 20000) {
		fault_set(FAULT_PU_RING_CMD);
	} else {
		fault_unset(FAULT_PU_RING_CMD);
	}

}

//РљРѕРјРјР°РЅРґС‹ РѕС‚ uart4 Рё uart7 СЃР»РµРІР° Рё СЃРїСЂР°РІР°
void uart4_uart7_snd(uint8_t * packet, uint16_t packet_len)
{
	int index;

	index = packet[0] / 10; //daddr/10

	if (uart4_uart7_rcv_huart_number[index] && (HAL_DiffTick(uart4_uart7_rcv_huart_number_tick[index]) < 1000) ) {
		uarts_transmit_it_by_num(uart4_uart7_rcv_huart_number[index], packet, packet_len);
	} else { //Р»РёРЅРёСЏ СЂР°Р·РѕСЂРІР°РЅР°
		uarts_transmit_it_by_num(4, packet, packet_len);
		uarts_transmit_it_by_num(7, packet, packet_len);
	}
}


//static uint32_t last_pd_snd_tick = 0;
//static uint32_t opus_rcv_cnt = 0;
//uint32_t uarts_transmit_it_tick = 0;

dynamic_buf_t uarts_tx_buf = DYNAMIC_BUF_STATIC_INIT(60 * 1024) ;

void uarts_trans(uint8_t * packet, uint16_t packet_len, uint8_t huart_number, uint8_t mux)
{
	uint8_t buf[1000];

	memcpy(buf, packet, packet_len);
	buf[packet_len] = huart_number;
	buf[packet_len + 1] = mux;
	dynamic_buf_push(&uarts_tx_buf, packet, packet_len + 2);
}

void uarts_trans_task(void)
{
	uint8_t mux;
	const huart_param_t * huart_param;
	size_t data_len;
	uint8_t buf[1000];

	if (DYNAMIC_BUF_IS_EMPTY(uarts_tx_buf)) return;

	huart_param = get_huart_param_by_number(uarts_tx_buf.first->data[uarts_tx_buf.first->data_len - 2]);
	if (!huart_param) {
		dynamic_buf_pop(&uarts_tx_buf, buf, sizeof(buf), &data_len);
		return;
	}

	if (HAL_UART_GetState(huart_param->huart) == HAL_UART_STATE_BUSY_TX) return;

	mux = uarts_tx_buf.first->data[uarts_tx_buf.first->data_len - 1];

	dynamic_buf_pop(&uarts_tx_buf,  huart_param->tx_buf, huart_param->tx_buf_size, &data_len);
	if (data_len < 2) return;

	if (huart_param->number == 6) {
		if (huart6.RxState != HAL_UART_STATE_READY) HAL_UART_AbortReceive_IT(&huart6);
		uart6_set_mux(mux);
		HAL_UARTEx_ReceiveToIdle_IT(&huart6, huart_param->rx_buf, huart_param->rx_buf_size);
	}
	else if (huart_param->number == 2) {
		if (huart2.RxState != HAL_UART_STATE_READY) HAL_UART_AbortReceive_IT(&huart2);
		uart2_set_mux(mux);
		HAL_UARTEx_ReceiveToIdle_IT(&huart2, huart_param->rx_buf, huart_param->rx_buf_size);
	}

	HAL_UART_Transmit_IT(huart_param->huart, huart_param->tx_buf, data_len - 2);



}
*/
