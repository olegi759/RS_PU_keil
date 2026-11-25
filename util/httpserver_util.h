/*
 * httpserver_util.h
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef UTIL_HTTPSERVER_UTIL_H_
#define UTIL_HTTPSERVER_UTIL_H_

#include "def_util.h"
#include <stdint.h>
#include "http_util.h"

#include "httpserver_util_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HTTPSERVER_MAX_SOCKET
#define HTTPSERVER_MAX_SOCKET	        8
#endif

#ifndef HTTPSERVER_MAX_BUF_SIZE
#define HTTPSERVER_MAX_BUF_SIZE         (4 * 1024)
#endif

#ifndef HTTPSERVER_BASE_FILE
#define HTTPSERVER_BASE_FILE            "index.html"
#endif

#ifndef HTTPSERVER_NAME
#define HTTPSERVER_NAME                 "httpserv"
#endif

#ifndef HTTPSERVER_UPLOAD_FILE_KEY
#define HTTPSERVER_UPLOAD_FILE_KEY      "filename"
#endif


#ifndef HTTPSERVER_FILE_NAME_SIZE
#define HTTPSERVER_FILE_NAME_SIZE       128
#endif

#ifndef HTTPSERVER_FILE_PARAMS_SIZE
#define HTTPSERVER_FILE_PARAMS_SIZE     1024
#endif

#ifndef HTTPSERVER_BOUNDARY_SIZE
#define HTTPSERVER_BOUNDARY_SIZE        256
#endif

typedef enum {
    HS_STATE_IDLE = 0,     //
    HS_STATE_RECV_REQUEST_HEADER, //recv header
    HS_STATE_RECV_REQUEST_BODY, //recv body
} httpserver_state_t;

typedef struct _httpserver_socket_data {
        uint8_t state;
        uint8_t request;
        uint8_t type;
        int request_offset;
        int content_len;
        int body_len;
        char * file_name;
        char * file_params;
        char * boundary;
        char * upload_file_name;
        char * buf;
} httpserver_socket_data_t;


void httpserver_init(void); //httpserver init
void httpserver_on_accept(int index); //httpserver on accept socket
void httpserver_on_recv(int index); //httpserver on recv socket

//!NEED
void httpserver_close(int index); //httpserver closed socket
void httpserver_send(int index, char * data, int data_len);  //send data to socket
int httpserver_recv(int index, char * data, int data_len); //recv data from socket
void httpserver_init_socket_data(int index); //init socket httpserver_socket_data_t

BOOL httpserver_file_exist(const char * file_name); //file exist
int httpserver_get_file_size(const char * file_name); //file size
BOOL httpserver_file_type_is_web(const char * file_name);
int httpserver_read_file(const char * file_name, int file_size, int file_pos, char * buf, int buf_len, char * params, int type); //read file

int httpserver_read_cgi(const char * file_name, const char * file_params, uint8_t request, char * buf, int buf_len); //read cgi


void httpserver_create_upload_file(const char * file_name); //create upload file
void httpserver_write_upload_file(const char * file_name, char * body, int body_len); // write to upload file

#ifdef __cplusplus
}
#endif

#endif /* UTIL_HTTPSERVER_UTIL_H_ */
