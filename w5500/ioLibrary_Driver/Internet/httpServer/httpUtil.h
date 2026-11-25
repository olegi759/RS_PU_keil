/**
 * @file	httpUtil.h
 * @brief	Header File for HTTP Server Utilities
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "httpServer.h"
#include "httpParser.h"
#include "main.h"

uint8_t http_get_cgi_handler(uint8_t * uri_name, st_http_request * p_http_request, uint8_t * buf, uint32_t * file_len);
uint8_t http_post_cgi_handler(uint8_t * uri_name, st_http_request * p_http_request, uint8_t * buf, uint32_t * file_len);

char * get_or_set_item(char * key, char * val, BOOL set);


extern BOOL change_cis;
extern BOOL change_csl;
extern BOOL change_um;


extern uint8_t cgi_page_num;
uint8_t get_cgi_num(char * path);

#ifdef __cplusplus
}
#endif

#endif
