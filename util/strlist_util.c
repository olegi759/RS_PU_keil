/*
 * strlist_util.c
 *
 *  Created on: 8 мая 2026 г.
 *      Author: Rafael Boltachev
 */


#include "strlist_util.h"
#include "def_util.h"
#include "str_util.h"


void strlist_init(strlist_t * strlist, char * buf, int buf_size)
{
	strlist->buf = buf;
	strlist->buf_size = buf_size;
}


void strlist_set_text(strlist_t * strlist, char * text)
{
	strcpy_s(strlist->buf, strlist->buf_size, text);
}

void strlist_add_line(strlist_t * strlist, char * line)
{
	strcat_s(strlist->buf, strlist->buf_size, line);
	strcat_s(strlist->buf, strlist->buf_size, CRLF);
}

int strlist_get_items_count(strlist_t * strlist)
{
	int cnt;
	char * ptr;

	cnt = 0;
	ptr = strlist->buf;
	while ( (ptr = strright(ptr, strlen(ptr), CRLF)) ) cnt++;

	return cnt;
}

char * strlist_get_item(strlist_t * strlist, int index, int * item_len)
{
	char * ptr, * ptr2;
	int cnt;

	ptr = strlist->buf;
	cnt = 0;
	while ( (ptr2 = strright(ptr, strlen(ptr), CRLF)) ) {
		*item_len = (ptr2 - ptr - strlen(CRLF));
		if (cnt == index) return ptr;
		cnt++;
		ptr = ptr2;
	}

	*item_len = 0;
	return NULL;
}

void strlist_clear(strlist_t * strlist)
{
	strlist->buf[0] = '\0';
}

int strlist_get_items_count_from_str(const char * str)
{
	int cnt;
	char * ptr, * ptr2;

	cnt = 0;
	ptr = (char *)str;
	while ( (ptr2 = strright(ptr, strlen(ptr), CRLF)) ) {
          cnt++;
          ptr = ptr2;
        }

        if (strlen(ptr)) cnt++;

	return cnt;
}

char * strlist_get_item_from_str(const char * str, int index, int * item_len)
{
	char * ptr, * ptr2;
	int cnt;

	ptr = (char *)str;
	cnt = 0;
	while ( (ptr2 = strright(ptr, strlen(ptr), CRLF)) ) {
		if (cnt == index) {
  		  *item_len = (ptr2 - ptr - strlen(CRLF));
                  return ptr;
                }
		cnt++;
		ptr = ptr2;
	}

        if (strlen(ptr)) {
          if (cnt == index) {
            *item_len = strlen(ptr);
             return ptr;
          }
        }


	*item_len = 0;
	return NULL;
}


BOOL strlist_remove_item_from_str(const char * str, int index)
{
	char * ptr, * ptr2;
	int  item_len;

	ptr = strlist_get_item_from_str(str, index, &item_len);

	if (!ptr) return FALSE;

	ptr2 = ptr + item_len + strlen(CRLF);

	if (strlen(ptr2)) {
		strcpy(ptr, ptr2);
	} else {
		*ptr = '\0';
	}

	return TRUE;
}
