/*
 * http_server.c
 *
 *  Created on: 18 февр. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "http_server.h"
#include "httpserver_util.h"
//#include "w5500.h"
#include "socket.h"
#include "fatfs_util.h"
#include "webpage.h"
#include "str_util.h"
#include "httpUtil.h"
#include "mkk.h"
#include "safety.h"
#include "cm4core.h"
#include "common.h"

#include "virtual_socket_map.h"

#define HTTP_SEND_CHUNK 2048     /* или 3072, если TX-буфер сокета 4 КБ */

static uint32_t httpserver_tick[HTTPSERVER_MAX_SOCKET] = {0};


uint32_t w5500_send_all(uint8_t sn, uint8_t * buf, uint32_t len)
{
	uint32_t size;
	uint32_t send_len;
	int32_t rc;


	size = 0;
	while (size < len) {
        send_len = MIN(len - size, HTTP_SEND_CHUNK);
        int32_t rc = send(sn, &buf[size], send_len);
        if (rc > 0) { size += (uint32_t)rc; continue; }
        if (getSn_SR(sn) != SOCK_ESTABLISHED) break;   /* клиент ушёл */
        HAL_Delay(1);
	}
	return size;
}


uint32_t w5500_recv_all(uint8_t sn, uint8_t * buf, uint32_t len)
{

	uint32_t offset;
	uint16_t size;
	int32_t rc;
	uint16_t available;



	offset = 0;
	while (offset < len) {
		available = getSn_RX_RSR(sn);
		if (available < 1) break;
		size = len - offset;
		if (size > available) size = available;
		rc = recv(sn, &buf[offset], size);
		if (rc > 0)	offset += rc;
		//HAL_Delay(10);
	}


	return offset;

}

// -------------- LOCAL PAGE -----------------------------
typedef struct {
  const char * file_name;
  const char * ptr;
  int size;
} local_page_t;

#define LOCAL_PAGE_COUNT		11

static local_page_t local_pages[LOCAL_PAGE_COUNT] = {
  {"style.css", PAGE_STYLE_CSS, sizeof(PAGE_STYLE_CSS) - 1},
  {"params.js", PAGE_PARAMS_JS, sizeof(PAGE_PARAMS_JS) - 1},
  {"index.html", PAGE_INDEX_HTML, sizeof(PAGE_INDEX_HTML) - 1},
  {"network.html", PAGE_NETWORK_HTML, sizeof(PAGE_NETWORK_HTML) - 1},
  {"setting.html", PAGE_SETTING_HTML, sizeof(PAGE_SETTING_HTML) - 1},
  {"control.html", PAGE_CONTROL_HTML, sizeof(PAGE_CONTROL_HTML) - 1},
  {"mode.html", PAGE_MODE_HTML, sizeof(PAGE_MODE_HTML) - 1},
  {"filemanager.html", PAGE_FILEMANAGER_HTML, sizeof(PAGE_FILEMANAGER_HTML) - 1},
  {"safety.html", PAGE_SAFETY_HTML, sizeof(PAGE_SAFETY_HTML) - 1},
  {"auth.html", PAGE_AUTH_HTML, sizeof(PAGE_AUTH_HTML) - 1},
  {"netfunc.html", PAGE_NETFUNC_HTML, sizeof(PAGE_NETFUNC_HTML) - 1}

};


const char * get_local_page_and_page_size(const char * file_name, int * file_size)
{
   int i;
   uint8_t level;

   *file_size = 0;




   if ( !strstr_s((char *)file_name, strlen(file_name), ".css") &&  !strstr_s((char *)file_name, strlen(file_name), ".js") ) {

	   level = safety_get_level_from_pin_code();

	   if (level < 3) {

		   switch (level) {
		   	   case 1:
				   *file_size = sizeof(PAGE_ERROR_USER_LEVEL_1) - 1;
				   return PAGE_ERROR_USER_LEVEL_1;
			   break;
		   	   case 2:
				   *file_size = sizeof(PAGE_ERROR_USER_LEVEL_2) - 1;
				   return PAGE_ERROR_USER_LEVEL_2;
			   break;
		   	   //case 3:
				//   *file_size = sizeof(PAGE_ERROR_USER_LEVEL_3) - 1;
				//   return PAGE_ERROR_USER_LEVEL_3;
			   //break;
		   	   //case 4:
				  // *file_size = sizeof(PAGE_ERROR_USER_LEVEL_4) - 1;
				   //return PAGE_ERROR_USER_LEVEL_4;
			   //break;
		   	   default:
				   *file_size = sizeof(PAGE_ERROR_USER_LEVEL_1) - 1;
				   return PAGE_ERROR_USER_LEVEL_1;
		   }

	   }

	   //if (!safety_is_login_and_password_good()) {
		//   *file_size = sizeof(PAGE_AUTH_HTML) - 1;
		//   return PAGE_AUTH_HTML;
	   //}

   }


   for (i = 0; i < LOCAL_PAGE_COUNT; i++) {
      if (strequals((char *)file_name, local_pages[i].file_name)) {
          *file_size = local_pages[i].size;
          return local_pages[i].ptr;
      }

   }

   return NULL;
}

const char * get_local_page(const char * file_name)
{
    int file_size;

    return get_local_page_and_page_size(file_name, &file_size);
}

int get_local_page_size(const char * file_name)
{
    int file_size;

    get_local_page_and_page_size(file_name, &file_size);

    return file_size;
}

// ----------------------------------- SEND RECV -----------------------------


void httpserver_close(int index)
{
	disconnect(index);
	httpserver_init_socket_data(index);
	httpserver_tick[index] = 0;

	setSn_CR(index, Sn_CR_DISCON);
	/* wait to process the command... */
	while(getSn_CR(index));

	//ulog_fmt("httpserver_close(%d)"CRLF, index);
}

void httpserver_send(int index, char * data, int data_len)
{
	w5500_send_all(index, (uint8_t *)data, data_len);
}

int httpserver_recv(int index, char * data, int data_len)
{
	return w5500_recv_all(index, (uint8_t *)data, data_len);
}


// ----------------------------------- FILE -----------------------------

BOOL httpserver_file_exist(const char * file_name)
{
	if (get_local_page(file_name)) return TRUE;

	return f_exists(file_name) ? TRUE : FALSE;
}

BOOL httpserver_file_type_is_web(const char * file_name)
{
	if (get_local_page(file_name)) return TRUE;
	return FALSE;
}


int httpserver_get_file_size(const char * file_name)
{
	FSIZE_t fsize;
	int fsz;

	fsz = get_local_page_size(file_name);
	if (fsz) return fsz;

	return (f_file_size2(file_name,  &fsize) == FR_OK ? fsize : 0);
}

int httpserver_read_file(const char * file_name, int file_size, int file_pos, char * buf, int buf_len, char * params, int type)
{
	FIL fil;
	UINT br;
	FRESULT fr;
    const char * ptr;
    int len;

    if (file_size && (file_pos >= file_size)) return 0;

	len = MIN(buf_len, (file_size - file_pos));
	if (!len) return 0;

    ptr = get_local_page(file_name);
    if (ptr) {
    	memcpy(buf, &ptr[file_pos], len);
    	return len;
    }


	if (f_open(&fil, file_name, FA_READ) != FR_OK) return 0;
	f_lseek (&fil, file_pos);
    fr = f_read(&fil, buf, len,  &br);
    f_close(&fil);

   return fr == FR_OK ? br : 0 ;
}


// ----------------------------------- UPLOAD FILE ------------------------

void httpserver_create_upload_file(const char * file_name)
{
	f_unlink(file_name);
}

void httpserver_write_upload_file(const char * file_name, char * body, int body_len)
{
	f_append(file_name, body, body_len);
}

// ----------------------------------- CGI -----------------------------

typedef struct {
  char * buf;
  int buf_len;
  uint8_t request;
} httpserver_read_cgi_params_t;


void parse_cgi_params(char * key, char * val, void * params)
{
   httpserver_read_cgi_params_t * p = (httpserver_read_cgi_params_t *)params;


   if (strlen(p->buf) > 1) strcat_s(p->buf, p->buf_len, ",");

   strcat_s(p->buf, p->buf_len, "\"");
   strcat_s(p->buf, p->buf_len, key);
   strcat_s(p->buf, p->buf_len, "\"");
   strcat_s(p->buf, p->buf_len, ":");
   strcat_s(p->buf, p->buf_len, "\"");
   strcat_s(p->buf, p->buf_len, get_or_set_item(key, val, p->request == HTTP_POST));
   strcat_s(p->buf, p->buf_len, "\"");
}


int httpserver_read_cgi(const char * file_name, const char * file_params, uint8_t request, char * buf, int buf_len)
{
	   httpserver_read_cgi_params_t cgi_params;

	   cgi_params.buf = buf;
	   cgi_params.buf_len = buf_len;
	   cgi_params.request = request;

	   cgi_page_num = get_cgi_num((char *)file_name);
	   if (!cgi_page_num) return 0;


	   buf[0] = '{';
	   buf[1] = '\0';
	   parse_http_params((char *)file_params, parse_cgi_params, &cgi_params);
	   if (request == HTTP_POST) {

			if (strlen(buf) < 4) {
				strcat_s(buf, buf_len, "\"result\":\"Ошибка парсинга входных данных\"}");
				return strlen(buf);
			}

	    	strcat_s(buf, buf_len, ", \"result\":\"Данные сохранены\"}");


			if (change_csl) { //Контроль исправности линий звукового оповещения - включение
				change_csl = FALSE;
				mkk_send_lin_on_from_setting(UART2_MUX_MKKL_1);
				if (setting.lines_count > 12) {
					HAL_Delay(50);
					mkk_send_lin_on_from_setting(UART2_MUX_MKKL_2);
				}
			    mkk_change();
			}

			if (change_cis) { //Контроль исправности входов сигнализации - включение
				change_cis = FALSE;

				mkk_send_insig_on_from_setting(UART2_MUX_MKKL_1);
				if (setting.lines_count > 12) {
					HAL_Delay(50);
					mkk_send_insig_on_from_setting(UART2_MUX_MKKL_2);
				}
			    mkk_change();
			}

			if (change_um) { //Распределение линий вещания между усилителями мощности (УМ)
				change_um = FALSE;
				mkk_send_lin_from_setting(UART2_MUX_MKKL_1);
				if (setting.lines_count > 12) {
					HAL_Delay(50);
					mkk_send_lin_from_setting(UART2_MUX_MKKL_2);
				}
			}

	    	need_save_setting = TRUE;

	    	cm4core_change_setting();

	   }
	   else strcat_s(buf, buf_len, "}");

	   return strlen(buf);
}

// ----------------------------------- RUN -----------------------------
//0x13 (SOCK_LISTEN) – the server is waiting for a client connection.
//0x17 (SOCK_ESTABLISHED) – a client has successfully connected.
//0x1C (SOCK_CLOSE_WAIT) – the client has closed its side of the connection.
//0x00 (SOCK_CLOSED) – the socket is fully closed.



//static uint8_t http_server_buf[HTTPSERVER_MAX_BUF_SIZE];





void http_server_run(uint8_t sn)
{

	if (httpserver_tick[sn] && (HAL_DiffTick(httpserver_tick[sn]) > 60000)) {
		disconnect(sn);
		httpserver_init_socket_data(sn);
		httpserver_tick[sn] = 0;
	}



	switch (getSn_SR(sn))
	{
		case SOCK_ESTABLISHED:
			//ulog_fmt("SOCK  ESTABLISHED(%d) "CRLF, s);


			if (getSn_RX_RSR(sn)) {
				http_client_active_tick = HAL_GetTickNoZero();
				httpserver_tick[sn] = HAL_GetTickNoZero();
				httpserver_on_recv(sn);
			}

			/*
			//Работает ОК!!!
			size = getSn_RX_RSR(sn);

			if (size) {
				if (size > sizeof(http_server_buf)) size = sizeof(http_server_buf);
				rc = recv(sn, http_server_buf, size);
				if (rc > 0) {
					http_server_buf[rc] = 0;
					http_server_buf[50] = 0;
					ulog_fmt("recv(%d)=%d %s"CRLF, sn, rc, http_server_buf);

					//send(sn, (uint8_t *)PAGE_INDEX_HTML, sizeof(PAGE_INDEX_HTML) - 1);
					w5500_send_all(sn, (uint8_t *)PAGE_INDEX_HTML, sizeof(PAGE_INDEX_HTML) - 1);
					disconnect(sn);


				}
			}
			*/



		break;
		case SOCK_CLOSE_WAIT:
			//ulog_fmt("SOCK CLOSE_WAIT(%d)"CRLF, sn);
			disconnect(sn);
			httpserver_init_socket_data(sn);
			httpserver_tick[sn] = 0;
			break;

		case SOCK_CLOSED:
			//ulog_fmt("SOCK CLOSED(%d)"CRLF, sn);
			socket(sn, Sn_MR_TCP, HTTP_SERVER_PORT, 0x00);
			break;

		case SOCK_INIT:
			//ulog_fmt("SOCK INIT(%d)"CRLF, sn);
			listen(sn);
			httpserver_init_socket_data(sn);
			httpserver_tick[sn] = 0;
			break;

		case SOCK_LISTEN:
			//ulog_fmt("LISTEN(%d)"CRLF, sn);
			break;

		default :
			break;

	} // end of switch

}


void http_server_init(void)
{
	httpserver_init();
}


/*
void parse_cgi_params(char * key, char * val, void * params)
{
   httpserver_read_cgi_params_t * p;
   int len;

   p = (httpserver_read_cgi_params_t *)params;

   len = strlen(p->buf);

   snprintf(&p->buf[len], p->buf_len - len, "%s\"%s\":\"%s\"", (len > 1 ? "," : ""), key,  get_or_set_item(key, val, p->request == HTTP_POST));
}


void  httpserver_on_request_data(size_t request, char * path, int path_len)
{
	char buf[MAX_STR_SIZE];

	strncpy_s(buf, sizeof(buf), path, path_len);
	ulog_fmt(CONM"%s %s"CRLF, http_request_str[request],  buf);

}

*/
