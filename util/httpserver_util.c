/*
 * httpserver_util.c
 *
 *  Created on: 12 февр. 2026 г.
 *      Author: Rafael Boltachev
 */

#include "httpserver_util.h"
#include "str_util.h"
#include <stdio.h>


static httpserver_socket_data_t hs_sock_data[HTTPSERVER_MAX_SOCKET] = {0};



void httpserver_init_socket_data(int index)
{
	//ulog("d1"CRLF);
	if (hs_sock_data[index].buf) {
		free(hs_sock_data[index].buf);
		hs_sock_data[index].buf = NULL;
	}
	//ulog("d2"CRLF);
	if (hs_sock_data[index].file_name) {
		free(hs_sock_data[index].file_name);
		hs_sock_data[index].file_name = NULL;
	}
	//ulog("d3"CRLF);
	if (hs_sock_data[index].file_params) {
		free(hs_sock_data[index].file_params);
		hs_sock_data[index].file_params = NULL;
	}
	//ulog("d4"CRLF);
	if (hs_sock_data[index].boundary) {
		free(hs_sock_data[index].boundary);
		hs_sock_data[index].boundary = NULL;
	}
	//ulog("d5"CRLF);
	if (hs_sock_data[index].upload_file_name) {
		free(hs_sock_data[index].upload_file_name);
		hs_sock_data[index].upload_file_name = NULL;
	}
	//ulog("d6"CRLF);
	memset(&hs_sock_data[index], 0, sizeof(hs_sock_data[index]));
}


void httpserver_on_accept(int index)
{
	httpserver_init_socket_data(index);
}


void httpserver_close_and_init(int index)
{
	httpserver_close(index);
	httpserver_init_socket_data(index);
}


void httpserver_send_404(httpserver_socket_data_t * sock_data, int index, const char * uri_type)
{
	http_create_answer(sock_data->buf, HTTPSERVER_MAX_BUF_SIZE, http_code_404,  HTTPSERVER_NAME, http_delimetr_rn, 0, uri_type);
	httpserver_send(index, sock_data->buf, strlen(sock_data->buf));
}

void httpserver_send_200(int index, int content_length, const char * uri_type)
{
	char buf[256];

	http_create_answer(buf, sizeof(buf), http_code_200,  HTTPSERVER_NAME, http_delimetr_rn, content_length, uri_type);
	httpserver_send(index, buf, strlen(buf));
}

#define RES_HTTP_200_FILE_DOWNLOAD ""\
"HTTP/1.1 200 OK\r\n"\
"Server: "HTTPSERVER_NAME"\r\n"\
"Content-Encoding: identity\r\n"\
"Content-Type: application/force-download; charset=utf-8\r\n"\
"Content-Disposition: attachment; filename=\"%s\"\r\n"\
"Content-Length: %d\r\n"\
"Connection: Closed\r\n"\
"\r\n"

void httpserver_send_200_file(int index, int content_length,  const char * file_name)
{
	char buf[300];

	snprintf(buf, sizeof(buf), RES_HTTP_200_FILE_DOWNLOAD, file_name, content_length);
	httpserver_send(index, buf, strlen(buf));
}


void httpserver_send_answer(httpserver_socket_data_t * sock_data, int index)
{
	int rs;
	const char * uri_type;
	int file_pos, file_size;

	file_pos = 0;
	uri_type = http_type_to_uri_type(sock_data->type);

	if (sock_data->type == HTTP_TYPE_CGI)  {
		rs = httpserver_read_cgi(sock_data->file_name, sock_data->file_params, sock_data->request, sock_data->buf, HTTPSERVER_MAX_BUF_SIZE);
		if (rs) {
			httpserver_send_200(index, rs, uri_type);
			httpserver_send(index, sock_data->buf, rs);
		} else {
			httpserver_send_404(sock_data, index, uri_type);
		}
	} else {
		if (httpserver_file_exist(sock_data->file_name)) {
			file_size = httpserver_get_file_size(sock_data->file_name);

			if (httpserver_file_type_is_web(sock_data->file_name)) {

				httpserver_send_200(index, file_size, uri_type);
			} else {
				httpserver_send_200_file(index, file_size, sock_data->file_name);
			}


			while (file_pos < file_size) {
				rs = httpserver_read_file(sock_data->file_name, file_size, file_pos, sock_data->buf, HTTPSERVER_MAX_BUF_SIZE, sock_data->file_params, sock_data->type);
				if (rs < 1) break;
				httpserver_send(index, sock_data->buf, rs);
				file_pos += rs;
			}

		} else {
			httpserver_send_404(sock_data, index, uri_type);
		}
	}

	httpserver_close_and_init(index);
}

void httpserver_fill_sock_data(httpserver_socket_data_t * sock_data, http_request_data_t * request_data)
{
	char * ptr;
	int len;


	sock_data->request = request_data->request;
	sock_data->content_len = request_data->content_length;

	if (request_data->boundary) {
		len = MIN(HTTPSERVER_BOUNDARY_SIZE, request_data->boundary_len);
		if (!sock_data->boundary) sock_data->boundary = malloc(len + 1);
		strncpy_s(sock_data->boundary, len + 1, request_data->boundary, len);
	}


	if (request_data->path_len <= 1) {
		len = strlen(HTTPSERVER_BASE_FILE) + 1;
		if (!sock_data->file_name) sock_data->file_name = malloc(len);
		strcpy_s(sock_data->file_name, len, HTTPSERVER_BASE_FILE);
	} else {

	   ptr = strstr_s(request_data->path, request_data->path_len, "?");
	   if (ptr) {
		   len = MIN(HTTPSERVER_FILE_NAME_SIZE, ptr - request_data->path - 1);
		   if (!sock_data->file_name) sock_data->file_name = malloc(len + 1);
		   strncpy_s(sock_data->file_name, len + 1, &request_data->path[1], len);
		   ptr++;
		   len = MIN(HTTPSERVER_FILE_PARAMS_SIZE, request_data->path_len - (ptr - request_data->path));
		   if (!sock_data->file_params) sock_data->file_params = malloc(len + 1);
		   strncpy_s(sock_data->file_params, len + 1, ptr, len);
	   } else {
		   len = MIN(HTTPSERVER_FILE_NAME_SIZE, request_data->path_len - 1);
		   if (!sock_data->file_name) sock_data->file_name = malloc(len + 1);
		   strncpy_s(sock_data->file_name, len + 1, &request_data->path[1], len);
	   }
	}

	sock_data->type = http_type_from_file_name(sock_data->file_name);

	//ulog_fmt("%s %s %s"CRLF, (sock_data->request == HTTP_POST ? "POST" : "GET"), sock_data->file_name, (sock_data->file_params ? sock_data->file_params : "NULL"));
}

BOOL httpserver_find_upload_file(httpserver_socket_data_t * sock_data, char * str, int str_len)
{
    char * ptr, * ptr2;
    int len;


    ptr = strstr_s(str, str_len, sock_data->boundary);
    if (!ptr) return FALSE;

    ptr = strright(str, str_len, HTTPSERVER_UPLOAD_FILE_KEY"=\"");
    if (!ptr) return FALSE;

    ptr2 = strstr_s(ptr, str_len - (ptr - str), "\"");
    if (!ptr2) return FALSE;

    len = ptr2 - ptr;
    sock_data->upload_file_name = malloc(len + 1);
    strncpy_s(sock_data->upload_file_name, len + 1, ptr, len);

    return TRUE;
}

void httpserver_upload_file_finish(httpserver_socket_data_t * sock_data, int index, char * body, int body_len)
{
   char * ptr;
   int len;

   ptr = memrfind(body, body_len, sock_data->boundary, strlen(sock_data->boundary));
   if (ptr) {
      len = (ptr - body) - 4;
      httpserver_write_upload_file(sock_data->upload_file_name, body, len);
   } else {
       httpserver_write_upload_file(sock_data->upload_file_name, body, body_len);
   }
 }

void httpserver_on_recv_body(httpserver_socket_data_t * sock_data, int index, char * body, int body_len)
{
	char * ptr;
	BOOL finish;
	int len;


	sock_data->body_len += body_len;
	finish = sock_data->body_len >= sock_data->content_len;


	if (body_len) {

		if  (sock_data->request == HTTP_POST) {

			if (sock_data->boundary) {

				if (sock_data->upload_file_name) {
					if (finish) {
						httpserver_upload_file_finish(sock_data, index, body, body_len);
					} else {
						httpserver_write_upload_file(sock_data->upload_file_name, body, body_len);
					}
				} else {
					ptr = strright(body, body_len, http_delimetr_rnrn);
					if (!ptr) {
						//httpserver_close_and_init(index);
						httpserver_send_answer(sock_data, index);
						return;
					}
					if (!httpserver_find_upload_file(sock_data, body, ptr - body)) {
						//httpserver_close_and_init(index);
						httpserver_send_answer(sock_data, index);
						return;
					}

					httpserver_create_upload_file(sock_data->upload_file_name);

					len = body_len - (ptr - body);
					if (len) {
						if (finish) {
							httpserver_upload_file_finish(sock_data, index, ptr, len);
						} else {
							httpserver_write_upload_file(sock_data->upload_file_name, ptr, len);
						}
					}
				}
			}
			//if  (sock_data->type == HTTP_TYPE_CGI) {
				if (!sock_data->file_params) {
					len = MIN(body_len, HTTPSERVER_FILE_PARAMS_SIZE);
					sock_data->file_params = malloc(len + 1);
					strncpy_s(sock_data->file_params, len + 1, body, len);
				}
			//}

		} //if  (sock_data->request == HTTP_POST) {
	} //if (body_len)



	if (finish) {
		httpserver_send_answer(sock_data, index);
	}
}

void httpserver_on_recv(int index)
{
  http_request_data_t request_data;
  int rc;
  BOOL res;
  httpserver_socket_data_t * sock_data;

  sock_data = &hs_sock_data[index];


  if (!sock_data->buf) sock_data->buf = malloc(HTTPSERVER_MAX_BUF_SIZE);

  switch (sock_data->state) {
        case HS_STATE_IDLE:
           rc = httpserver_recv(index, sock_data->buf, HTTPSERVER_MAX_BUF_SIZE);
           if (rc < 1) {
               httpserver_close_and_init(index);
               return;
            }
            sock_data->request_offset = rc;
            res = http_parse_request(sock_data->buf, sock_data->request_offset, &request_data);
            if (!res) {
                 httpserver_init_socket_data(index);
                 return;
            }


            if (request_data.body) {
                sock_data->state = HS_STATE_RECV_REQUEST_BODY;
                httpserver_fill_sock_data(sock_data, &request_data);
                httpserver_on_recv_body(sock_data, index, request_data.body, request_data.body_len);
            }
            else sock_data->state = HS_STATE_RECV_REQUEST_HEADER;

//#ifdef HTTP_SERVER_DEBUG
//            httpserver_on_request_data(request_data.request , request_data.path, request_data.path_len);
//#endif

        break;
        case HS_STATE_RECV_REQUEST_HEADER:
           rc = httpserver_recv(index, &sock_data->buf[sock_data->request_offset], HTTPSERVER_MAX_BUF_SIZE - sock_data->request_offset);
           if (rc < 1) {
               httpserver_close_and_init(index);
               return;
            }
            sock_data->request_offset += rc;
            res = http_parse_request(sock_data->buf, sock_data->request_offset, &request_data);
            if (!res) {
                 httpserver_init_socket_data(index);
                	//ulog("7"CRLF);
                 return;
            }
            if (!request_data.body) return;
            sock_data->state = HS_STATE_RECV_REQUEST_BODY;
            httpserver_fill_sock_data(sock_data, &request_data);
            httpserver_on_recv_body(sock_data, index, request_data.body, request_data.body_len);

        break;
        case HS_STATE_RECV_REQUEST_BODY:
           rc = httpserver_recv(index, sock_data->buf, HTTPSERVER_MAX_BUF_SIZE);
           if (rc < 1) {
               httpserver_close_and_init(index);
               return;
            }
            httpserver_on_recv_body(sock_data, index, sock_data->buf, rc);
        break;
  }

}



void httpserver_init(void)
{
	memset(hs_sock_data, 0, sizeof(hs_sock_data));
}

