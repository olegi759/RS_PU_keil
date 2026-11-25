/*
 * external_flash.h
 *
 *  Created on: 14 февр. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_EXTERNAL_FLASH_H_
#define CORE_INC_EXTERNAL_FLASH_H_

#include "main.h"

//extern BOOL ch376_is_mount;


void external_flash_init(void);
void external_flash_task(void);


BOOL external_flash_get_file_list_item_func_log(int index, char * file_name, unsigned long file_size, void * param);
uint16_t external_flash_enum_files(const char * path, on_enum_files_func_t on_enum_files_func, void * param);


BOOL external_flash_open_file(const char * path);
uint16_t external_flash_read_file(uint8_t * buf, uint16_t cnt);

#define EXTERNAL_FLASH_CLOSE_FORBID_UPDATE	0
#define EXTERNAL_FLASH_CLOSE_AUTO_UPDATE	1

BOOL external_flash_close_file(uint8_t update);
BOOL external_flash_file_exist(const char * path);

unsigned int external_flash_stream_read(void * afile, void * data, unsigned int data_len);
BOOL external_flash_copy_to_local(const char * file_name);
BOOL external_flash_copy_from_local(const char * file_name);

uint8_t external_flash_file_delete(const char * file_name);


extern BOOL external_flash_is_mount;

uint32_t  external_flash_test(const char * path);

#endif /* CORE_INC_EXTERNAL_FLASH_H_ */
