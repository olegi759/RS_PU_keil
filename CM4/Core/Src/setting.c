/*
 * setting.c
 *
 *  Created on: 24 нояб. 2023 г.
 *      Author: Rafael Boltachev
 */

#include "setting.h"
#include <string.h>
#include <stdio.h>
#include "str_util.h"
#include "net_util.h"
#include "stm_util.h"
#include "math_util.h"
#include "common.h"
#include "speaker.h"
#include "w5500.h"

//FATFS fatfs = {0};
//BOOL fatfs_mount = FALSE;



void mode_mchs_to_str(mode_mchs_t * mode_mchs, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "ls:%u", mode_mchs->sound_lines);
}

void str_to_mode_mchs(char * buf, uint16_t buf_len, mode_mchs_t * mode_mchs)
{
	char * ptr;

	ptr = strright(buf, buf_len, "ls:");
	if (ptr) mode_mchs->sound_lines = str2ul(ptr);
}

/*
void mode_manual_to_str(mode_manual_t * mode_manual, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "en:%s,f:%s,dl:%u,rp:%d", bool2str(mode_manual->enable), mode_manual->file, mode_manual->delay, mode_manual->repeat);
}

void str_to_mode_manual(char * buf, uint16_t buf_len, mode_manual_t * mode_manual)
{
	char * ptr, * ptr2;

	ptr = strright(buf, buf_len, "en:");
	if (ptr) mode_manual->enable = str2bool(ptr);

	ptr = strright(buf, buf_len, "dl:");
	if (ptr) mode_manual->delay = str2ull(ptr);

	ptr = strright(buf, buf_len, "rp:");
	if (ptr) mode_manual->repeat = str2int(ptr);

	ptr = strright(buf, buf_len, "f:");
	if (ptr) {
		ptr2 = strstr_s(ptr, strlen(ptr), ",");
		strncpy_s(mode_manual->file, sizeof(mode_manual->file), ptr, ptr2 - ptr);
	}


}
*/

void mode_fire_to_str(mode_fire_t * mode_fire, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "en:%s,dl:%u,rp:%d,tb:%s,f:%s,i1:%u,i2:%u,i3:%u,i4:%u,i5:%u,i6:%u,i7:%u,i8:%u,i9:%u,i10:%u,i11:%u,i12:%u,i13:%u,i14:%u,i15:%u,i16:%u,i17:%u,i18:%u,i19:%u,i20:%u,i21:%u,i22:%u,i23:%u,i24:%u",
			bool2str(mode_fire->enable), mode_fire->delay, mode_fire->repeat,  bool2str(mode_fire->tilibom), mode_fire->file,
			mode_fire->input_lines[1], mode_fire->input_lines[2], mode_fire->input_lines[3], mode_fire->input_lines[4],  mode_fire->input_lines[5], mode_fire->input_lines[6],
			mode_fire->input_lines[7], mode_fire->input_lines[8], mode_fire->input_lines[9], mode_fire->input_lines[10],  mode_fire->input_lines[11], mode_fire->input_lines[12],
            mode_fire->input_lines[13], mode_fire->input_lines[14], mode_fire->input_lines[15], mode_fire->input_lines[16],  mode_fire->input_lines[17], mode_fire->input_lines[18],
            mode_fire->input_lines[19], mode_fire->input_lines[20], mode_fire->input_lines[21], mode_fire->input_lines[22],  mode_fire->input_lines[23], mode_fire->input_lines[24]
	);
}

void str_to_mode_fire(char * buf, uint16_t buf_len, mode_fire_t * mode_fire)
{
	char * ptr, * ptr2;

	ptr = strright(buf, buf_len, "en:");
	if (ptr) mode_fire->enable = str2bool(ptr);

	ptr = strright(buf, buf_len, "dl:");
	if (ptr) mode_fire->delay = str2ull(ptr);

	ptr = strright(buf, buf_len, "rp:");
	if (ptr) mode_fire->repeat = str2int(ptr);

	ptr = strright(buf, buf_len, "tb:");
	if (ptr) mode_fire->tilibom = str2bool(ptr);

	ptr = strright(buf, buf_len, "f:");
	if (ptr) {
		ptr2 = strstr_s(ptr, strlen(ptr), ",");
		strncpy_s(mode_fire->file, sizeof(mode_fire->file), ptr, ptr2 - ptr);
	}


	ptr = strright(buf, buf_len, "i1:");
	if (ptr) mode_fire->input_lines[1] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i2:");
	if (ptr) mode_fire->input_lines[2] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i3:");
	if (ptr) mode_fire->input_lines[3] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i4:");
	if (ptr) mode_fire->input_lines[4] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i5:");
	if (ptr) mode_fire->input_lines[5] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i6:");
	if (ptr) mode_fire->input_lines[6] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i7:");
	if (ptr) mode_fire->input_lines[7] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i8:");
	if (ptr) mode_fire->input_lines[8] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i9:");
	if (ptr) mode_fire->input_lines[9] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i10:");
	if (ptr) mode_fire->input_lines[10] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i11:");
	if (ptr) mode_fire->input_lines[11] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i12:");
	if (ptr) mode_fire->input_lines[12] = str2ul(ptr);


	ptr = strright(buf, buf_len, "i13:");
	if (ptr) mode_fire->input_lines[13] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i14:");
	if (ptr) mode_fire->input_lines[14] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i15:");
	if (ptr) mode_fire->input_lines[15] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i16:");
	if (ptr) mode_fire->input_lines[16] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i17:");
	if (ptr) mode_fire->input_lines[17] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i18:");
	if (ptr) mode_fire->input_lines[18] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i19:");
	if (ptr) mode_fire->input_lines[19] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i20:");
	if (ptr) mode_fire->input_lines[20] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i21:");
	if (ptr) mode_fire->input_lines[21] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i22:");
	if (ptr) mode_fire->input_lines[22] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i23:");
	if (ptr) mode_fire->input_lines[23] = str2ul(ptr);

	ptr = strright(buf, buf_len, "i24:");
	if (ptr) mode_fire->input_lines[24] = str2ul(ptr);
}

void mode_sound_to_str(mode_sound_t * mode_sound, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "ss:%d,tb:%s,ls:%u,fl:%s",  mode_sound->soundsrc, bool2str(mode_sound->tilibom), mode_sound->sound_lines, mode_sound->file);
}



void str_to_mode_sound(char * buf, uint16_t buf_len, mode_sound_t * mode_sound)
{
	char * ptr, * ptr2;


	ptr = strright(buf, buf_len, "ss:");
	if (ptr) mode_sound->soundsrc = str2int(ptr);


	ptr = strright(buf, buf_len, "ls:");
	if (ptr) mode_sound->sound_lines = str2ul(ptr);

	ptr = strright(buf, buf_len, "tb:");
	if (ptr) mode_sound->tilibom = str2bool(ptr);

	ptr = strright(buf, buf_len, "fl:");
	if (ptr) {
		ptr2 = strstr_s(ptr, strlen(ptr), ",");
		if (ptr2) strncpy_s(mode_sound->file, sizeof(mode_sound->file), ptr, ptr2 - ptr);
		else strcpy_s(mode_sound->file, sizeof(mode_sound->file), ptr);
	}

}

void netfunc_fire_to_str(netfunc_fire_t * netfunc_fire, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "a:%d,l:%d,s:%u,f:%s",  netfunc_fire->pu_addr, netfunc_fire->fire_line, netfunc_fire->sound_lines, netfunc_fire->file);
}

void str_to_netfunc_fire(char * buf, uint16_t buf_len, netfunc_fire_t * netfunc_fire)
{
	char * ptr;

	ptr = strright(buf, buf_len, "a:");
	if (ptr) netfunc_fire->pu_addr = str2int(ptr);

	ptr = strright(buf, buf_len, "l:");
	if (ptr) netfunc_fire->fire_line = str2int(ptr);

	ptr = strright(buf, buf_len, "s:");
	if (ptr) netfunc_fire->sound_lines = str2ull(ptr);

	ptr = strright(buf, buf_len, "f:");
	if (ptr) {
		strcpy_s(netfunc_fire->file, sizeof(netfunc_fire->file), ptr);
	}
}

void netfunc_mchs_to_str(netfunc_mchs_t * netfunc_mchs, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "a:%d,l:%u",  netfunc_mchs->pu_addr, netfunc_mchs->sound_lines);
}

void str_to_netfunc_mchs(char * buf, uint16_t buf_len, netfunc_mchs_t * netfunc_mchs)
{
	char * ptr;

	ptr = strright(buf, buf_len, "a:");
	if (ptr) netfunc_mchs->pu_addr = str2int(ptr);

	ptr = strright(buf, buf_len, "l:");
	if (ptr) netfunc_mchs->sound_lines = str2ul(ptr);

}

void user_to_str(user_t * user, char * buf, uint16_t buf_len)
{
	snprintf(buf, buf_len, "nm:%s,pc:%s,lv:%d", user->name, user->pin_code, user->level);
}

void str_to_user(char * buf, uint16_t buf_len, user_t * user)
{
	char * ptr, * ptr2;

	ptr = strright(buf, buf_len, "nm:");
	if (ptr) {
		ptr2 = strstr_s(ptr, strlen(ptr), ",");
		if (ptr2) {
			strncpy_s(user->name, sizeof(user->name), ptr, ptr2 - ptr);
		}
	}

	ptr = strright(buf, buf_len, "pc:");
	if (ptr) {
		ptr2 = strstr_s(ptr, strlen(ptr), ",");
		if (ptr2) {
			strncpy_s(user->pin_code, sizeof(user->pin_code), ptr, ptr2 - ptr);
		}
	}


	ptr = strright(buf, buf_len, "lv:");
	if (ptr) user->level = str2int(ptr);

}
BOOL compare_key(char * s, size_t s_size, const char * key)
{
	if (s_size != strlen(key)) return FALSE;

	return strcmp_s(s, s_size, key);
}

void default_setting(void)
{
	int uid;
	char mac_str[20];
	//uint32_t uids[3];


	memset(&setting, 0, sizeof(setting));
	setting.uart_local_addr = 10;
	setting.sai_volume = 100;
	//strcpy_s(setting.name, sizeof(setting.name), "ТРОМБОН ПУ");
	setting.lines_count = 24;
	setting.mic_volume = 100;
	setting.mchs_volume = 100;

	//uids[0] = HAL_GetUIDw0();
	//uids[1] = HAL_GetUIDw1();
	//uids[2] = HAL_GetUIDw2();

	uid = rand();
	//snprintf(mac_str, sizeof(mac_str), "8A:EF:%02X:%02X:%02X:%02X", (uid >> 24) & 0xFF, (uid >> 16) & 0xFF, (uid >> 8) & 0xFF, uid & 0xFF);
	//ulog_fmt("def mac: %s"CRLF, mac_str);
	memcpy(setting.chipsNetInfo[0].mac, str2mac("8A:EF:77:95:C1:84"), sizeof(setting.chipsNetInfo[0].mac));
	//memcpy(setting.chipsNetInfo[0].mac, str2mac(mac_str), sizeof(setting.chipsNetInfo[0].mac));
	setting.chipsNetInfo[0].dhcp = FALSE; //TRUE;
	//setting.ip = str2ip("192.168.0.84");
	setting.chipsNetInfo[0].ip = str2ip("192.168.0.117");
	setting.chipsNetInfo[0].gw = str2ip("192.168.0.1");
	setting.chipsNetInfo[0].mask =  str2ip("255.255.255.0");
	setting.chipsNetInfo[0].dns =  str2ip("8.8.8.8");


	uid = rand();
	snprintf(mac_str, sizeof(mac_str), "8A:EF:%02X:%02X:%02X:%02X", (uid >> 24) & 0xFF, (uid >> 16) & 0xFF, (uid >> 8) & 0xFF, uid & 0xFF);
	//ulog_fmt("def mac: %s"CRLF, mac_str);
	memcpy(setting.chipsNetInfo[1].mac, str2mac("8A:EF:77:19:C1:47"), sizeof(setting.chipsNetInfo[1].mac));
	//memcpy(setting.chipsNetInfo[1].mac, str2mac(mac_str), sizeof(setting.chipsNetInfo[1].mac));
	setting.chipsNetInfo[1].dhcp = FALSE; //TRUE;
	//setting.ip = str2ip("192.168.0.84");
	setting.chipsNetInfo[1].ip = str2ip("192.168.0.118");
	setting.chipsNetInfo[1].gw = str2ip("192.168.0.1");
	setting.chipsNetInfo[1].mask =  str2ip("255.255.255.0");
	setting.chipsNetInfo[1].dns =  str2ip("8.8.8.8");
	//strcpy_s(setting.pin_code, sizeof(setting.pin_code), "12345678");

	//strcpy_s(setting.web_login, sizeof(setting.web_login), "admin");
	//strcpy_s(setting.web_password, sizeof(setting.web_password), "admin");



	setting.mkk_procent = 20;
	setting.log_file_size = 0;

	setting.mkk_um1 = 0xAAAAAA;
	setting.mkk_um2 = 0x555555;

	setting.int_spk = IS_UM1_UM2;
	setting.control_time_sec = 80;
	setting.mic_is_fix = TRUE;

	setting.users[0].level = 1;
	setting.users[1].level = 1;
	setting.users[2].level = 1;
	setting.users[3].level = 1;
	setting.users[4].level = 1;
	setting.users[5].level = 1;
	setting.users[6].level = 1;
	setting.users[7].level = 1;
	setting.users[8].level = 1;
	setting.users[9].level = 1;
	setting.users[10].level = 1;
}



BOOL load_setting(const char * path)
{
	FIL cfg_file;
	size_t len, key_len;
	char buf[512], * ptr;
	int ival;
	char * val;

	default_setting();

	if (!fatfs_mount) return FALSE;

	if ( f_open(&cfg_file, path, FA_READ) != FR_OK)  return FALSE;

	f_readln_init();

	while (f_readln(buf, sizeof(buf), &cfg_file) == FR_OK) {
		len = strlen(buf);
		if (len == 0) continue;
				ptr = strstr_s(buf, len, "=");
		if (!ptr) continue;
		key_len = ptr - buf;
		if (!key_len) continue;
		*ptr = '\0';          /* buf теперь ровно ключ: "net0_ip" */
		val = ptr + 1;        /* val — значение:          "192.168.0.117" */

		if (compare_key(buf, key_len, KEY_UART_LOCAL_ADDR)) setting.uart_local_addr = str2int(val);
		//else if (compare_key(buf, key_len, KEY_NAME)) strcpy_s(setting.name, sizeof(setting.name), val);
		else if (compare_key(buf, key_len, KEY_SAI_VOLUME)) setting.sai_volume = str2int(val);
		else if (compare_key(buf, key_len, KEY_LINES_COUNT)) setting.lines_count = str2int(val);
		// else if (compare_key(buf, key_len, KEY_MAC)) memcpy(setting.chipsNetInfo[0].mac, str2mac(val), sizeof(setting.chipsNetInfo[0].mac));
		// else if (compare_key(buf, key_len, KEY_DHCP)) setting.chipsNetInfo[0].dhcp = str2bool(val);
		// else if (compare_key(buf, key_len, KEY_IP)) setting.chipsNetInfo[0].ip = str2ip(val);
		// else if (compare_key(buf, key_len, KEY_GW)) setting.chipsNetInfo[0].gw = str2ip(val);
		// else if (compare_key(buf, key_len, KEY_MASK)) setting.chipsNetInfo[0].mask = str2ip(val);
		// else if (compare_key(buf, key_len, KEY_DNS)) setting.chipsNetInfo[0].dns = str2ip(val);
		
		else if (key_len > 4 && buf[4] == '_' && buf[0] == 'n' && buf[1] == 'e' && buf[2] == 't'
		         && buf[3] >= '0' && buf[3] < (char)('0' + _WIZCHIP_CHIP_NUM_)) {
			uint8_t ch = (uint8_t)(buf[3] - '0');
			char * field = &buf[5];

			if      (!strcmp(field, "mac"))  memcpy(setting.chipsNetInfo[ch].mac, str2mac(val), sizeof(setting.chipsNetInfo[ch].mac));
			else if (!strcmp(field, "dhcp")) setting.chipsNetInfo[ch].dhcp = str2bool(val);
			else if (!strcmp(field, "ip"))   setting.chipsNetInfo[ch].ip   = str2ip(val);
			else if (!strcmp(field, "mask")) setting.chipsNetInfo[ch].mask = str2ip(val);
			else if (!strcmp(field, "gw"))   setting.chipsNetInfo[ch].gw   = str2ip(val);
			else if (!strcmp(field, "dns"))  setting.chipsNetInfo[ch].dns  = str2ip(val);
		}
		/* старые файлы (без префикса) — это настройки канала 0 */
		else if (compare_key(buf, key_len, KEY_MAC))  memcpy(setting.chipsNetInfo[0].mac, str2mac(val), sizeof(setting.chipsNetInfo[0].mac));
		else if (compare_key(buf, key_len, KEY_DHCP)) setting.chipsNetInfo[0].dhcp = str2bool(val);
		else if (compare_key(buf, key_len, KEY_IP))   setting.chipsNetInfo[0].ip   = str2ip(val);
		else if (compare_key(buf, key_len, KEY_MASK)) setting.chipsNetInfo[0].mask = str2ip(val);
		else if (compare_key(buf, key_len, KEY_GW))   setting.chipsNetInfo[0].gw   = str2ip(val);
		else if (compare_key(buf, key_len, KEY_DNS))  setting.chipsNetInfo[0].dns  = str2ip(val);


		else if (compare_key(buf, key_len, KEY_MIC_VOLUME)) setting.mic_volume = str2int(val);
		else if (compare_key(buf, key_len, KEY_MCHS_VOLUME)) setting.mchs_volume = str2int(val);
		else if (compare_key(buf, key_len, KEY_PD_COUNT)) setting.pd_count = str2int(val);
		else if (compare_key(buf, key_len, KEY_UM_COUNT)) setting.um_count = str2int(val);
		else if (compare_key(buf, key_len, KEY_MKK_PROCENT)) setting.mkk_procent = str2int(val);
		else if (compare_key(buf, key_len, KEY_MKK_SOUND_LINES)) setting.mkk_sound_lines = str2ul(val);
		else if (compare_key(buf, key_len, KEY_MKK_INSIG_LINES)) setting.mkk_insig_lines = str2ul(val);
		else if (compare_key(buf, key_len, KEY_MKK_UM1)) setting.mkk_um1 = str2ul(val);
		else if (compare_key(buf, key_len, KEY_MKK_UM2)) setting.mkk_um2 = str2ul(val);
		else if (compare_key(buf, key_len, KEY_MKK_START_LINES)) setting.mkk_start_lines = str2ul(val);
		//else if (compare_key(buf, key_len, KEY_ZONE_NUMBERS)) str_to_arr(val, &setting.zone_numbers[1], VAR_TYPE_UINT16, FALSE);
		else if (compare_key(buf, key_len, KEY_INTERNAL_SPEAKER)) setting.int_spk = str2int(val);
		else if (compare_key(buf, key_len, KEY_GROUPS)) str_to_arr(val, &setting.groups[1], VAR_TYPE_UINT32, FALSE);
		else if (compare_key(buf, key_len, KEY_LOGFILE_SIZE)) setting.log_file_size = str2ul(val);
		else if (compare_key(buf, key_len, KEY_MIC_FIX)) setting.mic_is_fix = str2bool(val);
		else if (compare_key(buf, key_len, KEY_CONTROL_TIME)) setting.control_time_sec = str2ul(val);

		else if (compare_key(buf, key_len, KEY_MANUAL_TILIBOM_MIC)) setting.mode_manual_tilibom_mic = str2bool(val);
		else if (compare_key(buf, key_len, KEY_MANUAL_TILIBOM_FILE)) setting.mode_manual_tilibom_file = str2bool(val);



		//else if (compare_key(buf, key_len, KEY_PIN_CODE_LEVEL_1)) {
		//	strcpy_s(setting.pin_code_level_1, sizeof(setting.pin_code_level_1), val);
		//}
		//else if (compare_key(buf, key_len, KEY_PIN_CODE_LEVEL_2)) {
		//	strcpy_s(setting.pin_code_level_2, sizeof(setting.pin_code_level_2), val);
		//}
		//else if (compare_key(buf, key_len, KEY_PIN_CODE_LEVEL_3)) {
		//	strcpy_s(setting.pin_code_level_3, sizeof(setting.pin_code_level_3), val);
		//}
		//else if (compare_key(buf, key_len, KEY_PIN_CODE_LEVEL_4)) {
		//	strcpy_s(setting.pin_code_level_4, sizeof(setting.pin_code_level_4), val);
		//}


		else if (strcmp_s(buf, key_len, "user_")) {
			ptr = buf + strlen("user_");
			ival = str2int(ptr);
			str_to_user(val, strlen(val), &setting.users[ival]);
		}


		//else if (compare_key(buf, key_len, KEY_WEB_LOGIN)) {
		//	strcpy_s(setting.web_login, sizeof(setting.web_login), val);
		//}
		//else if (compare_key(buf, key_len, KEY_WEB_PASSWORD)) {
		//	strcpy_s(setting.web_password, sizeof(setting.web_password), val);
		//}

		else if (compare_key(buf, key_len, "modemchs")) {
			str_to_mode_mchs(val, strlen(val), &setting.mode_mchs);
		}
		else if (compare_key(buf, key_len, "modesound")) {
			str_to_mode_sound(val, strlen(val), &setting.mode_sound);
		}
		else if (strcmp_s(buf, key_len, "modefire_")) {
			ptr = buf + strlen("modefire_");
			ival = str2int(ptr);
			str_to_mode_fire(val, strlen(val), &setting.mode_fire[ival]);
		}
		//else if (strcmp_s(buf, key_len, "modemanual_")) {
		//	ptr = buf + strlen("modemanual_");
		//	ival = str2int(ptr);
		//	str_to_mode_manual(val, strlen(val), &setting.mode_manual[ival]);
		//}
		else if (strcmp_s(buf, key_len, "netfunc_fire_")) {
			ptr = buf + strlen("netfunc_fire_");
			ival = str2int(ptr);
			str_to_netfunc_fire(val, strlen(val), &setting.netfunc_fire[ival]);
		}

		else if (strcmp_s(buf, key_len, KEY_ZONE_DESCRIPTION"_")) {
			ptr = buf + strlen(KEY_ZONE_DESCRIPTION"_");
			ival = str2int(ptr);
			//if ((ival > 0) && (ival < 13)) strcpy_s(setting.zone_descriptions[ival], sizeof(setting.zone_descriptions[ival]), val);
		}

		else if (compare_key(buf, key_len, "netfunc_mchs")) {
			str_to_netfunc_mchs(val, strlen(val), &setting.netfunc_mchs);
		}

	}

	f_close(&cfg_file);

	return TRUE;
}




BOOL save_setting(const char * path)
{
	FIL tmp_file;
	UINT bw;
	char buf[512];
	int i;


	if (!fatfs_mount) return FALSE;


	if ( f_open(&tmp_file, "save_setting.tmp", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return FALSE;

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_NAME, setting.name);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_UART_LOCAL_ADDR, setting.uart_local_addr);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_SAI_VOLUME, setting.sai_volume);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_LINES_COUNT, setting.lines_count);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	for (i = 0; i < _WIZCHIP_CHIP_NUM_; i++) {
		snprintf(buf, sizeof(buf), "net%d_mac=%s"CRLF,  i, mac2str(setting.chipsNetInfo[i].mac));
		f_write(&tmp_file, buf, strlen(buf), &bw);
		snprintf(buf, sizeof(buf), "net%d_dhcp=%s"CRLF, i, bool2str(setting.chipsNetInfo[i].dhcp));
		f_write(&tmp_file, buf, strlen(buf), &bw);
		if (!setting.chipsNetInfo[i].dhcp) {
			snprintf(buf, sizeof(buf), "net%d_ip=%s"CRLF,   i, ip2str(setting.chipsNetInfo[i].ip));
		    f_write(&tmp_file, buf, strlen(buf), &bw);
		    snprintf(buf, sizeof(buf), "net%d_mask=%s"CRLF, i, ip2str(setting.chipsNetInfo[i].mask));
		    f_write(&tmp_file, buf, strlen(buf), &bw);
		    snprintf(buf, sizeof(buf), "net%d_gw=%s"CRLF,   i, ip2str(setting.chipsNetInfo[i].gw));
		    f_write(&tmp_file, buf, strlen(buf), &bw);
		    snprintf(buf, sizeof(buf), "net%d_dns=%s"CRLF,  i, ip2str(setting.chipsNetInfo[i].dns));
		    f_write(&tmp_file, buf, strlen(buf), &bw);
		}
	}

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_MIC_VOLUME, setting.mic_volume);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_MCHS_VOLUME, setting.mchs_volume);
	f_write(&tmp_file, buf, strlen(buf), &bw);


	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_PD_COUNT, setting.pd_count);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_UM_COUNT, setting.um_count);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_MKK_PROCENT, setting.mkk_procent);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_MKK_SOUND_LINES, setting.mkk_sound_lines);
	f_write(&tmp_file, buf, strlen(buf), &bw);



	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_MKK_UM1, setting.mkk_um1);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_MKK_UM2, setting.mkk_um2);
	f_write(&tmp_file, buf, strlen(buf), &bw);


	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_INTERNAL_SPEAKER, setting.int_spk);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_LOGFILE_SIZE, setting.log_file_size);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_MIC_FIX, bool2str(setting.mic_is_fix));
	f_write(&tmp_file, buf, strlen(buf), &bw);


	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_MKK_INSIG_LINES, setting.mkk_insig_lines);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%d"CRLF, KEY_MKK_START_LINES, setting.mkk_start_lines);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%u"CRLF, KEY_CONTROL_TIME, setting.control_time_sec);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_PIN_CODE_LEVEL_1, setting.pin_code_level_1);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_PIN_CODE_LEVEL_2, setting.pin_code_level_2);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_PIN_CODE_LEVEL_3, setting.pin_code_level_3);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_PIN_CODE_LEVEL_4, setting.pin_code_level_4);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_MANUAL_TILIBOM_MIC, bool2str(setting.mode_manual_tilibom_mic));
	f_write(&tmp_file, buf, strlen(buf), &bw);

	snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_MANUAL_TILIBOM_FILE, bool2str(setting.mode_manual_tilibom_file));
	f_write(&tmp_file, buf, strlen(buf), &bw);



	for (i = 1; i <= 10; ++i) {
		if ( (setting.users[i].pin_code[0] == '\0') || (setting.users[i].name[0] == '\0') || (setting.users[i].level == 0)) continue;
		snprintf(buf, sizeof(buf), "user_%d=", i);
		user_to_str(&setting.users[i], buf + strlen(buf), sizeof(buf) - strlen(buf));
		strcat_s(buf, sizeof(buf), CRLF);
		f_write(&tmp_file, buf, strlen(buf), &bw);
	}


	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_WEB_LOGIN, setting.web_login);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	//snprintf(buf, sizeof(buf), "%s=%s"CRLF, KEY_WEB_PASSWORD, setting.web_password);
	//f_write(&tmp_file, buf, strlen(buf), &bw);

	strcpy_s(buf, sizeof(buf), "modemchs=");
	mode_mchs_to_str(&setting.mode_mchs, buf + strlen(buf), sizeof(buf) - strlen(buf));
	strcat_s(buf, sizeof(buf), CRLF);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	strcpy_s(buf, sizeof(buf), "modesound=");
	mode_sound_to_str(&setting.mode_sound, buf + strlen(buf), sizeof(buf) - strlen(buf));
	strcat_s(buf, sizeof(buf), CRLF);
	f_write(&tmp_file, buf, strlen(buf), &bw);


	strcpy_s(buf, sizeof(buf), "netfunc_mchs=");
	netfunc_mchs_to_str(&setting.netfunc_mchs, buf + strlen(buf), sizeof(buf) - strlen(buf));
	strcat_s(buf, sizeof(buf), CRLF);
	f_write(&tmp_file, buf, strlen(buf), &bw);

	for (i = 1; i <= 10; ++i) {
		snprintf(buf, sizeof(buf), "netfunc_fire_%d=", i);
		netfunc_fire_to_str(&setting.netfunc_fire[i], buf + strlen(buf), sizeof(buf) - strlen(buf));
		strcat_s(buf, sizeof(buf), CRLF);
		f_write(&tmp_file, buf, strlen(buf), &bw);
	}


	for (i = 1; i <= 10; ++i) {
		snprintf(buf, sizeof(buf), "modefire_%d=", i);
		mode_fire_to_str(&setting.mode_fire[i], buf + strlen(buf), sizeof(buf) - strlen(buf));
		strcat_s(buf, sizeof(buf), CRLF);
		f_write(&tmp_file, buf, strlen(buf), &bw);
	}

	//for (i = 1; i <= 10; ++i) {
	//	snprintf(buf, sizeof(buf), "modemanual_%d=", i);
	//	mode_manual_to_str(&setting.mode_manual[i], buf + strlen(buf), sizeof(buf) - strlen(buf));
	//	strcat_s(buf, sizeof(buf), CRLF);
	//	f_write(&tmp_file, buf, strlen(buf), &bw);
	//}


	snprintf(buf, sizeof(buf), "%s=", KEY_ZONE_NUMBERS);
	//arr_to_str(&setting.zone_numbers[1], 12, VAR_TYPE_UINT16, buf + strlen(buf), sizeof(buf) - strlen(buf), FALSE);
	strcat_s(buf, sizeof(buf), CRLF);
	f_write(&tmp_file, buf, strlen(buf), &bw);



	for (i = 1 ; i <= 12; i++) {
		//snprintf(buf, sizeof(buf), KEY_ZONE_DESCRIPTION"_%d=%s"CRLF, i, setting.zone_descriptions[i]);
		f_write(&tmp_file, buf, strlen(buf), &bw);
	}


	snprintf(buf, sizeof(buf), "%s=", KEY_GROUPS);
	arr_to_str(&setting.groups[1], 12, VAR_TYPE_UINT32, buf + strlen(buf), sizeof(buf) - strlen(buf), FALSE);
	strcat_s(buf, sizeof(buf), CRLF);
	f_write(&tmp_file, buf, strlen(buf), &bw);



	f_close(&tmp_file);

	f_unlink(path);
	f_rename("save_setting.tmp", path);

	return TRUE;
}

