/*
 * um.c
 *
 *  Created on: 20 мар. 2025 г.
 *      Author: Rafael Boltachev
 */


#include "um.h"
#include "uart6.h"
#include "setting.h"
#include <string.h>
#include "stm_util.h"
#include "str_util.h"
#include "fault.h"
#include "fault_data.h"
#include "pcm1753.h"
//#include "mux_sound.h"
#include "math_util.h"
#include "timer_util.h"
#include "um_data.h"
#include "common.h"

static uint8_t um_addr = 1;
//static uint8_t um_tx_buf[MAX_UART6_PACKET_SIZE];
//static uint32_t um_send_tick[7] = {0};
static uint32_t um_rcv_tick[7] = {0};

static uint8_t um_channel_mode_1[7] = {0};
static uint8_t um_channel_mode_2[7] = {0};
static uint8_t um_device_mistake[7] = {0};
static uint8_t um_test_line1[7] = {0};
static uint8_t um_test_line2[7] = {0};
//static uint32_t um_send_test_tick[7] = {0};


void um_transmit_piv(uint8_t daddr, uint8_t saddr, uint8_t cmd, uint8_t * cmd_data, uint16_t cmd_data_len)
{
	uart6_transmit_piv(UART6_MUX_EXT_AMP, daddr, saddr, cmd, cmd_data, cmd_data_len);
}




void um_rcv_state(uint8_t addr, uint8_t channel_mode_1, uint8_t channel_mode_2, uint8_t device_mistake)
{
	if (um_device_mistake[addr] != device_mistake) {
		ulog_fmt(CONM"um: addr:%d change device_mistake %s->%s"CRLF, addr, uch2bitstr(um_device_mistake[addr]), uch2bitstr(device_mistake));
	}

	if (GETBIT(device_mistake, UM_BIT_PROTECT_CH1)) fault_set(FAULT_UM_PROTECT_CH1_1 + addr - 1);
	else fault_unset(FAULT_UM_PROTECT_CH1_1 + addr - 1);


	if (GETBIT(device_mistake, UM_BIT_PROTECT_CH2)) fault_set(FAULT_UM_PROTECT_CH2_1 + addr - 1);
	else fault_unset(FAULT_UM_PROTECT_CH2_1 + addr - 1);

	if (GETBIT(device_mistake, UM_BIT_BATTERY)) fault_set(FAULT_UM_BATTERY_1 + addr - 1);
	else fault_unset(FAULT_UM_BATTERY_1 + addr - 1);

	if (GETBIT(device_mistake, UM_BIT_LINE_OUTAGE)) fault_set(FAULT_UM_LINE_OUTAGE_1 + addr - 1);
	else fault_unset(FAULT_UM_LINE_OUTAGE_1 + addr - 1);

	if (GETBIT(device_mistake, UM_BIT_CH1_OUTAGE)) fault_set(FAULT_UM_CH1_OUTAGE_1 + addr - 1);
	else fault_unset(FAULT_UM_CH1_OUTAGE_1 + addr - 1);

	if (GETBIT(device_mistake, UM_BIT_CH2_OUTAGE)) fault_set(FAULT_UM_CH2_OUTAGE_1 + addr - 1);
	else fault_unset(FAULT_UM_CH2_OUTAGE_1 + addr - 1);

	//if ((um_channel_mode_1[addr] != channel_mode_1) || (um_channel_mode_2[addr] != channel_mode_1) || (um_device_mistake[addr] != device_mistake)) {
		//fulog_fmt("um: change state %d %d %d"CRLF, channel_mode_1, channel_mode_2, device_mistake);
	//}

	um_channel_mode_1[addr] = channel_mode_1;
	um_channel_mode_2[addr] = channel_mode_2;
	um_device_mistake[addr] = device_mistake;
}

void um_rcv_test_line(uint8_t addr, uint8_t ch1, uint8_t ch2)
{

	if ((um_test_line1[addr] != ch1) || (um_test_line2[addr] != ch2)) {
		ulog_fmt(CONM"um: addr:%d change test line ch1:(%d->%d)  ch2:(%d->%d)"CRLF, addr, um_test_line1[addr], ch1, um_test_line2[addr], ch2);
	}

	um_test_line1[addr] = ch1;
	um_test_line2[addr] = ch2;

	if (ch1) fault_unset(FAULT_UM_TEST_LINE1_1 + addr - 1);
	else fault_set(FAULT_UM_TEST_LINE1_1 + addr - 1);

	if (ch2) fault_unset(FAULT_UM_TEST_LINE2_1 + addr - 1);
	else fault_set(FAULT_UM_TEST_LINE2_1 + addr - 1);
}

void um_rcv(uint8_t * packet, uint16_t packet_len)
{

	//packet[1] - от кого пришли

	um_rcv_tick[packet[1]] = HAL_GetTickNoZero();
	fault_unset(FAULT_UM_CON_1 + packet[1] - 1);

	//fulog_fmt("um: rcv adr:%d ch1:%s ch2:%s err:%02X pwr:%02X lbr:%02X"CRLF, data[1], uch2bitstr(data[5]), uch2bitstr(data[6]), data[7], data[8], data[9]);

	switch (packet[4]) { //cmd
		case UM_CMD_STATE:
			um_rcv_state(packet[1], packet[5], packet[6], packet[7]);
		break;
		case UM_CMD_MODES:
		break;
		case UM_CMD_TEST_LINE:
			//fulog_fmt("um: UM_CMD_TEST_LINE %d %d %d %d %d %d %d %d"CRLF, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
			um_rcv_test_line(packet[1], packet[5], packet[6]);
		break;
	}

}

void um_snd_modes(uint8_t addr, uint8_t mode_ch1, uint8_t mode_ch2)
{
	uint8_t buf[2] = {mode_ch1, mode_ch2};

	um_transmit_piv(addr, setting.uart_local_addr, UM_CMD_MODES, buf, 2);
}

void um_snd_modes_all(uint8_t mode_ch1, uint8_t mode_ch2)
{
	um_snd_modes(0xFF, mode_ch1, mode_ch2);
}

/*
const short um_snd_test_const_20_2047[20] =  { 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047, 2047, -2047 };


void um_snd_test_after(void)
{
	timers_delete(um_snd_test_after);
	um_transmit_piv(um_addr, setting.uart_local_addr , UM_CMD_TEST_LINE, NULL, 0);
}

void um_snd_test(void)
{

    pu_prepare_pcm1753(SAI_AUDIO_FREQUENCY_44K, FALSE);

	set_mux_sound_right_left(SLR_UM1_UM2);

	while (pcm1753_AudioBufferGetFree() >= 2 * 1024) {
		pcm1753_AudioBufferLwrbWrite((int16_t *)um_snd_test_const_20_2047, 20);
	}

	timers_add(um_snd_test_after, 250);
}
*/

void um_snd_query(void)
{
	if (!setting.um_count) return;

	um_addr++;
	if (um_addr > setting.um_count) um_addr = 1;

	um_transmit_piv(um_addr, setting.uart_local_addr, UM_CMD_STATE, NULL, 0);

	if (um_rcv_tick[um_addr]) {
		if (HAL_DiffTick(um_rcv_tick[um_addr]) > setting.um_count * 16000) {
			fault_set(FAULT_UM_CON_1 + um_addr - 1);
		}
	} else {
		um_rcv_tick[um_addr] = HAL_GetTickNoZero();
	}

}


void um_snd_set_addr(uint8_t um_addr, uint8_t set_addr)
{
	um_transmit_piv(um_addr, setting.uart_local_addr , UM_CMD_SET_ADDRESS, &set_addr, 1);
}
