/*
 * net.c
 *
 *  Created on: 17 янв. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "net.h"
#include <string.h>
#include "stm_util.h"
#include "dhcp.h"
#include <stdbool.h>
#include "webpage.h"
#include "loopback.h"
#include "setting.h"
#include "str_util.h"
#include "common.h"

#include "ws_server.h"
#include "stage_test.h"

//#define USE_IO_LIBRARY	1

#ifdef USE_IO_LIBRARY
#include "httpServer.h"
#else
#include "http_server.h"
#endif

//#define DNS_SOCKET      1
//#define HTTP_SOCKET     2

//#define SOCK_TCPS       0
//#define SOCK_UDPS       1
//#define PORT_TCPS		5000
//#define PORT_UDPS       3000

//ко-лво http сокетов
#define MAX_HTTPSOCK	8
//#define DATA_BUF_SIZE   1024

//const uint8_t socknumlist[MAX_HTTPSOCK] = {2, 3, 4, 5, 6, 7};
//const uint8_t socknumlist[MAX_HTTPSOCK] = {2, 3, 4, 5};
//const uint8_t socknumlist[MAX_HTTPSOCK] = {0, 1, 2, 3, 4, 5, 6, 7};
//const uint8_t socknumlist[MAX_HTTPSOCK] = {0, 1};

static const uint8_t dhcp_sock[_WIZCHIP_CHIP_NUM_] = { 5, 13 };  /* виртуальные sn: local 5 */
static uint8_t * dhcp_buffer[_WIZCHIP_CHIP_NUM_] = { NULL, NULL };
const uint8_t socknumlist[MAX_HTTPSOCK] = {0, 1, 2, 3, 8, 9, 10, 11};

#ifdef USE_IO_LIBRARY
static uint8_t RX_BUF[DATA_BUF_SIZE];
static uint8_t TX_BUF[DATA_BUF_SIZE];
#endif
// 1K should be enough, see https://forum.wiznet.io/t/topic/1612/2
//static uint8_t dhcp_buffer[DATA_BUF_SIZE];
//#define  dhcp_buffer TX_BUF
//static uint8_t dhcp_buffer[548];


static wiz_NetInfo net_info = {0};


void W5500_Select(void)
{
	//ulog("CS0"CRLF);
	if (g_wizchip_sel == 0){
		HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_SET);
    	HAL_GPIO_WritePin(E_CS_SPI2_GPIO_Port, E_CS_SPI2_Pin, GPIO_PIN_RESET); 
	} else {	
		HAL_GPIO_WritePin(E_CS_SPI2_GPIO_Port, E_CS_SPI2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_RESET);
	}
}

void W5500_Unselect(void)
{
	//ulog("CS1"CRLF);
	HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(E_CS_SPI2_GPIO_Port, E_CS_SPI2_Pin, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(E_CS_SPI2_GPIO_Port, E_CS_SPI2_Pin, GPIO_PIN_RESET);
    // HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_SET);
	
	//HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(E_CS_SPI2_GPIO_Port, E_CS_SPI2_Pin, GPIO_PIN_SET);
}

void W5500_ReadBuff(uint8_t* buff, uint16_t len)
{
	//HAL_StatusTypeDef res;
    //char str[MAX_STR_SIZE] = {'\0'};

	//res =
			HAL_SPI_Receive(&hspi6, buff, len, 100);
    //mem2hexstr(buff, len, str, sizeof(str), " ");
	//if (len > 4) ulog_fmt("R=%d"CRLF,  len);

	//while (HAL_SPI_GetState(&hspi6) == HAL_SPI_STATE_BUSY_RX) {};
	// HAL_SPI_Receive_DMA(&hspi6, buff, len);
	 //while(HAL_SPI_GetState(&hspi6) == HAL_SPI_STATE_BUSY_RX);
}

//static uint8_t abuf[4096];
void W5500_WriteBuff(uint8_t* buff, uint16_t len)
{
	//HAL_StatusTypeDef res;
    //char str[MAX_STR_SIZE] = {'\0'};

    //res =
    		HAL_SPI_Transmit(&hspi6, buff, len, 100);
    //mem2hexstr(buff, len, str, sizeof(str), " ");
    //if (len > 4) ulog_fmt("W=%d"CRLF, len);

	//while(HAL_SPI_GetState(&hspi6) != HAL_SPI_STATE_READY) {};
	//memcpy(abuf, buff, MIN(len, sizeof(abuf)));
	//HAL_SPI_Transmit_IT(&hspi6, abuf, MIN(len, sizeof(abuf)));

	//while(HAL_SPI_GetState(&hspi6) == HAL_SPI_STATE_BUSY_TX);

}

uint8_t W5500_ReadByte(void)
{
    uint8_t byte;
    W5500_ReadBuff(&byte, sizeof(byte));
    return byte;
}

void W5500_WriteByte(uint8_t byte)
{
	//ulog("WB"CRLF);
    W5500_WriteBuff(&byte, sizeof(byte));
	//ulog("WE"CRLF);
}




void net_print_net_info(wiz_NetInfo * pnet_info)
{
    ulog_fmt(CONM"MAC: %02X:%02X:%02X:%02X:%02X:%02X"CRLF""CONM"IP:  %d.%d.%d.%d"CRLF""CONM"GW:  %d.%d.%d.%d"CRLF""CONM"Net: %d.%d.%d.%d"CRLF""CONM"DNS: %d.%d.%d.%d"CRLF""CONM"DHCP: %s"CRLF,
        pnet_info->mac[0], pnet_info->mac[1], pnet_info->mac[2], pnet_info->mac[3], pnet_info->mac[4], pnet_info->mac[5],
        pnet_info->ip[0], pnet_info->ip[1], pnet_info->ip[2], pnet_info->ip[3],
        pnet_info->gw[0], pnet_info->gw[1], pnet_info->gw[2], pnet_info->gw[3],
        pnet_info->sn[0], pnet_info->sn[1], pnet_info->sn[2], pnet_info->sn[3],
        pnet_info->dns[0], pnet_info->dns[1], pnet_info->dns[2], pnet_info->dns[3],
		(pnet_info->dhcp == NETINFO_DHCP ? "DYNAMIC" : "STATIC")
    );

}
static uint8_t chip_init_cnt = 0;
void net_init_continue(uint8_t chip_num)
{
    ulog_fmt(CONM"net: W5500 #%u read chip net settings..."CRLF, chip_num);
    wizchip_getnetinfo(&net_info, chip_num);
    net_print_net_info(&net_info);

#ifdef USE_IO_LIBRARY


    httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, (uint8_t *)socknumlist);		// Tx/Rx buffers (1kB) / The number of W5500 chip H/W sockets in use
    reg_httpServer_cbfunc(NVIC_SystemReset, NULL); 					// Callback: NXP MCU Reset


	reg_httpServer_webContent((uint8_t *)"index.html", (uint8_t *)PAGE_INDEX_HTML);				//стартовая
	reg_httpServer_webContent((uint8_t *)"setting.html", (uint8_t *)PAGE_SETTING_HTML);			//общие настройки
	reg_httpServer_webContent((uint8_t *)"network.html", (uint8_t *)PAGE_NETWORK_HTML);			//сеть
	reg_httpServer_webContent((uint8_t *)"style.css", (uint8_t *)PAGE_STYLE_CSS);				//стиль
	reg_httpServer_webContent((uint8_t *)"params.js", (uint8_t *)PAGE_PARAMS_JS);				//AJAX
    reg_httpServer_webContent((uint8_t *)"control.html", (uint8_t *)PAGE_CONTROL_HTML);			//Контроль линий
    reg_httpServer_webContent((uint8_t *)"mode.html", (uint8_t *)PAGE_MODE_HTML);				//Режимы
    reg_httpServer_webContent((uint8_t *)"filemanager.html", (uint8_t *)PAGE_FILEMANAGER_HTML);	//Файл менеджер
    reg_httpServer_webContent((uint8_t *)"safety.html", (uint8_t *)PAGE_SAFETY_HTML);			//Безопасность
    reg_httpServer_webContent((uint8_t *)"auth.html", (uint8_t *)PAGE_AUTH_HTML);				//Вход в WEB
    reg_httpServer_webContent((uint8_t *)"netfunc.html", (uint8_t *)PAGE_NETFUNC_HTML);			//Сетеывые ф-ции



/*

	// Index page and netinfo / base64 image demo
	reg_httpServer_webContent((uint8_t *)"index.html", (uint8_t *)index_page);				// index.html 		: Main page example
	reg_httpServer_webContent((uint8_t *)"netinfo.html", (uint8_t *)netinfo_page);			// netinfo.html 	: Network information example page
	reg_httpServer_webContent((uint8_t *)"netinfo.js", (uint8_t *)wiz550web_netinfo_js);	// netinfo.js 		: JavaScript for Read Network configuration 	(+ ajax.js)
	reg_httpServer_webContent((uint8_t *)"img.html", (uint8_t *)img_page);					// img.html 		: Base64 Image data example page

	// Example #1
	reg_httpServer_webContent((uint8_t *)"dio.html", (uint8_t *)dio_page);					// dio.html 		: Digital I/O control example page
	reg_httpServer_webContent((uint8_t *)"dio.js", (uint8_t *)wiz550web_dio_js);			// dio.js 			: JavaScript for digital I/O control 	(+ ajax.js)

	// Example #2
	reg_httpServer_webContent((uint8_t *)"ain.html", (uint8_t *)ain_page);					// ain.html 		: Analog input monitor example page
	reg_httpServer_webContent((uint8_t *)"ain.js", (uint8_t *)wiz550web_ain_js);			// ain.js 			: JavaScript for Analog input monitor	(+ ajax.js)

	// Example #3
	reg_httpServer_webContent((uint8_t *)"ain_gauge.html", (uint8_t *)ain_gauge_page);		// ain_gauge.html 	: Analog input monitor example page; using Google Gauge chart
	reg_httpServer_webContent((uint8_t *)"ain_gauge.js", (uint8_t *)ain_gauge_js);			// ain_gauge.js 	: JavaScript for Google Gauge chart		(+ ajax.js)

	// AJAX JavaScript functions
	reg_httpServer_webContent((uint8_t *)"ajax.js", (uint8_t *)wiz550web_ajax_js);			// ajax.js			: JavaScript for AJAX request transfer
*/
	display_reg_webContent_list();
#else
	chip_init_cnt++;

	if(chip_init_cnt < _WIZCHIP_CHIP_NUM_) return;

	ulog(CONM"net: http server init..."CRLF);
	http_server_init();
	ulog(CONM"net: web socket servet init..."CRLF);
	ws_server_init(4, 8080, 0); // протокольный WS (ctx 0)
	ws_server_init(12, 8080, 1); // протокольный WS (ctx 1)

	chip_init_cnt = 0;
#endif



}

void Callback_IPAssigned(void)
{
    uint8_t ch = getDHCPCurrentChip();
    uint8_t ip[4], gw[4], sn[4], dns[4];

    getIPfromDHCP(ip,  ch);
    getGWfromDHCP(gw,  ch);
    getSNfromDHCP(sn,  ch);
    getDNSfromDHCP(dns, ch);

    ulog_fmt(CONM"net: DHCP lease (chip %u): %u.%u.%u.%u/%u.%u.%u.%u gw %u.%u.%u.%u"CRLF,
             ch, ip[0],ip[1],ip[2],ip[3], sn[0],sn[1],sn[2],sn[3],
             gw[0],gw[1],gw[2],gw[3]);

	ulog_fmt(CONM"net: DHCP lease time = %u s (chip %d)"CRLF, getDHCPLeasetime(ch), ch);

    /* мы заменили default_ip_assign() — поэтому применяем настройки сами */
    getSHAR(net_info.mac, ch);            /* MAC из чипа, чтобы не затереть нулями */
    memcpy(net_info.ip,  ip,  sizeof(net_info.ip));
    memcpy(net_info.gw,  gw,  sizeof(net_info.gw));
    memcpy(net_info.sn,  sn,  sizeof(net_info.sn));
    memcpy(net_info.dns, dns, sizeof(net_info.dns));
    net_info.dhcp = NETINFO_DHCP;

    wizchip_setnetinfo(&net_info, ch);    /* IP/маска/шлюз в чип + теневые _DNS_/_DHCP_ */

	//копируем в setting что-бы web страница отображала новые значения
	memcpy(&setting.chipsNetInfo[ch].ip,   ip,  4);
    memcpy(&setting.chipsNetInfo[ch].gw,   gw,  4);
    memcpy(&setting.chipsNetInfo[ch].mask, sn,  4);
    memcpy(&setting.chipsNetInfo[ch].dns,  dns, 4);

    net_init_continue(ch);                /* запуск HTTP/WS, когда оба канала готовы */
}

void Callback_IPUpdated(void) { 
	uint8_t ch = getDHCPCurrentChip();
    uint8_t ip[4], gw[4], sn[4], dns[4];

    getIPfromDHCP(ip,  ch);
    getGWfromDHCP(gw,  ch);
    getSNfromDHCP(sn,  ch);
    getDNSfromDHCP(dns, ch);

	/* мы заменили default_ip_assign() — поэтому применяем настройки сами */
    getSHAR(net_info.mac, ch);            /* MAC из чипа, чтобы не затереть нулями */
    memcpy(net_info.ip,  ip,  sizeof(net_info.ip));
    memcpy(net_info.gw,  gw,  sizeof(net_info.gw));
    memcpy(net_info.sn,  sn,  sizeof(net_info.sn));
    memcpy(net_info.dns, dns, sizeof(net_info.dns));
    net_info.dhcp = NETINFO_DHCP;

    wizchip_setnetinfo(&net_info, ch);    /* IP/маска/шлюз в чип + теневые _DNS_/_DHCP_ */

	//копируем в setting что-бы web страница отображала новые значения
	memcpy(&setting.chipsNetInfo[ch].ip,   ip,  4);
    memcpy(&setting.chipsNetInfo[ch].gw,   gw,  4);
    memcpy(&setting.chipsNetInfo[ch].mask, sn,  4);
    memcpy(&setting.chipsNetInfo[ch].dns,  dns, 4);

	ulog_fmt(CONM"net: W5500 #%d DHCP IP Update ip(%u.%u.%u.%u) "CRLF, ch, ip[0],ip[1],ip[2],ip[3]);
}

void Callback_IPConflict(void)
{
    uint8_t ch = getDHCPCurrentChip();
    ulog_fmt(CONM"net: DHCP IP conflict (chip %d)"CRLF, ch);
    if (dhcp_buffer[ch]) {
        free(dhcp_buffer[ch]);
        dhcp_buffer[ch] = NULL;
    }
}


void net_task_execute(void)
{
	uint8_t i;

#ifdef USE_IO_LIBRARY
	for (i = 0; i < MAX_HTTPSOCK; i++) {
		httpServer_run(i); 	// HTTP Server handler
	}
#else
	for (i = 0; i < MAX_HTTPSOCK; i++) {
		http_server_run(socknumlist[i]);
	}
#endif

}

void net_task(void)
{
	net_task_execute();
	
	for (uint8_t i = 0; i < _WIZCHIP_CHIP_NUM_; i++) {
		if (setting.chipsNetInfo[i].dhcp) DHCP_run(i);
	}
}

//void W5500_Critical_Enter(void) {}

//void W5500_Critical_Exit(void) {}

void net_init(void)
{
	ulog(CONM"net: Register W5500 callbacks..."CRLF);
	reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
	reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
	reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

	uint8_t rx_tx_buff_sizes[] = {2, 2, 4, 4, 2, 2, 0, 0};

	ulog(CONM"net: W5500 #0 Calling wizchip_init()..."CRLF);
	if (wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes, 0) < 0) {
		ulog(CONM"net: W5500 #0 Calling wizchip_init() error"CRLF);
	}

	ulog(CONM"net: W5500 #1 Calling wizchip_init()..."CRLF);
	if (wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes, 1) < 0) {
		ulog(CONM"net: W5500 #1 Calling wizchip_init() error"CRLF);
	}


	ulog(CONM"net: Register DHCP callbacks..."CRLF);
	reg_dhcp_cbfunc(Callback_IPAssigned, Callback_IPAssigned, Callback_IPConflict);

	for (uint8_t i = 0; i < _WIZCHIP_CHIP_NUM_; i++) {
		uint8_t ver = getVERSIONR(i);
		if (ver != 0x04){ 
			ulog_fmt(CONM"net: WIZCHIP version ERROR (W5500 #%d, version = %u)"CRLF, i, ver);
			return;
		}

		if (setting.chipsNetInfo[i].dhcp) {
			/* MAC должен быть в чипе до DHCP — DHCP_init читает SHAR */
			setSHAR(setting.chipsNetInfo[i].mac, (uint8_t)i);

			dhcp_buffer[i] = malloc(548);
			if (!dhcp_buffer[i]) {
				ulog_fmt(CONM"net: W5500 #%d DHCP buffer alloc failed"CRLF, i);
				continue;
			}
			DHCP_init(dhcp_sock[i], dhcp_buffer[i], (uint8_t)i);
			ulog_fmt(CONM"net: DHCP started (W5500 #%d, sock %d)"CRLF, i, dhcp_sock[i]);
		} else {
			memcpy(net_info.mac, setting.chipsNetInfo[i].mac, sizeof(net_info.mac));
			memcpy(net_info.ip,  &setting.chipsNetInfo[i].ip,  sizeof(net_info.ip));
			memcpy(net_info.gw,  &setting.chipsNetInfo[i].gw,  sizeof(net_info.gw));
			memcpy(net_info.sn,  &setting.chipsNetInfo[i].mask, sizeof(net_info.sn));
			memcpy(net_info.dns, &setting.chipsNetInfo[i].dns, sizeof(net_info.dns));
			net_info.dhcp = NETINFO_STATIC;
			wizchip_setnetinfo(&net_info, (uint8_t)i);

			net_init_continue(i);
		}
	}
	
}






