/*
 * setting.h
 *
 *  Created on: 24 нояб. 2023 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_SETTING_H_
#define CORE_INC_SETTING_H_

//#include "main.h"
#include "fatfs_util.h"
#include "setting_data.h"
#include "shared_memory.h"


#ifdef __cplusplus
extern "C" {
#endif


//#define setting sm->setting
//extern FATFS fatfs;
//extern BOOL fatfs_mount;

void default_setting(void);

BOOL load_setting(const char * path);
BOOL save_setting(const char * path);


#define SETTING_FILE_NAME		"pu.cfg"


#define KEY_VERSION 			"version"
#define KEY_CM7_VERSION			"cm7version"
#define KEY_NETINFO 			"netinfo"
#define KEY_MKI_VERSION			"mkiversion"
#define KEY_MKK1_VERSION		"mkk1version"
#define KEY_MKK2_VERSION		"mkk2version"

#define KEY_NAME 				"name"
#define KEY_UART_LOCAL_ADDR  	"uartlocaladdr"
#define KEY_SAI_VOLUME 	 		"saivolume"
#define KEY_LINES_COUNT			"linescount"

#define KEY_MAC 				"mac"
#define KEY_DHCP				"dhcp"
#define KEY_IP					"ip"
#define KEY_MASK				"mask"
#define KEY_GW					"gw"
#define KEY_DNS 				"dns"

#define KEY_NET0_MAC			"net0_mac"
#define KEY_NET0_DHCP			"net0_dhcp"
#define KEY_NET0_IP				"net0_ip"
#define KEY_NET0_MASK			"net0_mask"
#define KEY_NET0_GW				"net0_gw"
#define KEY_NET0_DNS			"net0_dns"

#define KEY_NET1_MAC			"net1_mac"
#define KEY_NET1_DHCP			"net1_dhcp"
#define KEY_NET1_IP				"net1_ip"
#define KEY_NET1_MASK			"net1_mask"
#define KEY_NET1_GW				"net1_gw"
#define KEY_NET1_DNS			"net1_dns"

#define KEY_FIRE_LINES			"firelines"
#define KEY_PD_COUNT			"pdcount"
#define KEY_MCHS_LINES 			"mchslines"
#define KEY_MIC_VOLUME 			"micvolume"
#define KEY_MCHS_VOLUME 		"mchsvolume"
#define KEY_DATE_TIME 			"datetime"
#define KEY_UM_COUNT 			"umcount"
#define KEY_MODE_NOW 			"modenow"
#define KEY_LEVEL_NOW 			"levelnow"
#define KEY_STM32_UID			"uidstm32"

#define KEY_ZONE_NUMBERS			"zonenums"
#define KEY_ZONE_DESCRIPTION		"zonedesc"
#define KEY_MIC_FIX					"micfix"


#define KEY_NETINFO_MAC 			KEY_NETINFO""KEY_MAC
#define KEY_NETINFO_IP 				KEY_NETINFO""KEY_IP
#define KEY_NETINFO_MASK			KEY_NETINFO""KEY_MASK
#define KEY_NETINFO_GW				KEY_NETINFO""KEY_GW
#define KEY_NETINFO_DNS				KEY_NETINFO""KEY_DNS
#define KEY_NETINFO_DHCP			KEY_NETINFO""KEY_DHCP

#define KEY_MKK_PROCENT				"mkkprocent"
#define KEY_MKK_SOUND_LINES			"mkksoundlines"
#define KEY_MKK_INSIG_LINES			"mkkinsiglines"
#define KEY_MKK_START_LINES			"mkkstartlines"
#define KEY_CONTROL_TIME			"controltime"

#define KEY_MANUAL_TILIBOM			"manualtilibom"
#define KEY_MANUAL_TILIBOM_MIC		KEY_MANUAL_TILIBOM"mic"
#define KEY_MANUAL_TILIBOM_FILE		KEY_MANUAL_TILIBOM"file"

#define KEY_MKK_UM1					"mkkum1"
#define KEY_MKK_UM2					"mkkum2"
#define KEY_INTERNAL_SPEAKER		"intspk"

#define KEY_GROUPS					"groups"

#define KEY_PIN_CODE_LEVEL_1		"pincode1"
#define KEY_PIN_CODE_LEVEL_2		"pincode2"
#define KEY_PIN_CODE_LEVEL_3		"pincode3"
#define KEY_PIN_CODE_LEVEL_4		"pincode4"
#define KEY_WEB_LOGIN				"login"
#define KEY_WEB_PASSWORD			"password"

#define KEY_INPUT_WEB_LOGIN			"ilogin"
#define KEY_INPUT_WEB_PASSWORD		"ipassword"

#define KEY_INFO_FAULTS				"ifaults"

#define KEY_LOGFILE_SIZE			"lfsize"

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_SETTING_H_ */
