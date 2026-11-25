/*
 * mp3play2_util.h
 *
 *  Created on: 10 мар. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef UTIL_MP3PLAY2_UTIL_H_
#define UTIL_MP3PLAY2_UTIL_H_

#include "main.h"
#include "dacplaybuf_util.h"

void mp3play2_stop(void);
void mp3play2_task(void);
BOOL mp3play2_from_file(const char * file_name, dacplaybuf_data_t * dacplaybuf_data, uint16_t volume);
BOOL mp3play2_from_mem(void * mem, uint32_t mem_size, dacplaybuf_data_t * dacplaybuf_data, uint16_t volume);





#endif /* UTIL_MP3PLAY2_UTIL_H_ */
