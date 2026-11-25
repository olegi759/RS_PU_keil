/*
 * cfg_util.h
 *
 *  Created on: 10 мар. 2023 г.
 *      Author: Rafael Boltachev
 */

#ifndef _CFG_UTIL_H_
#define _CFG_UTIL_H_

#include "def_util.h"
#include "ff.h"



//Must be defined
void cfg_on_read_key_value(char * key, char * value);
BOOL cfg_read(const char * cfg_file_name);
BOOL cfg_write_key_value(FIL * cfg_file, char * key, char * value);

char * cfg_read_string(const char * cfg_file_name, const char * key);
BOOL cfg_write_string(const char * cfg_file_name, char * key, char * value);

/*
//#include "map_util.h"
 *
BOOL cfg_read(map_t * map, const char * file_name);
BOOL cfg_write( map_t * map, const char * file_name);
const char * cfg_get(map_t * map, const char * key);
BOOL cfg_set(map_t * map, const char * key, const char * value);
*/

#endif /* UTIL_CFG_UTIL_H_ */
