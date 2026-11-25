/*
 * cli_util.h
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef UTIL_CONSOLE_UTIL_H_
#define UTIL_CONSOLE_UTIL_H_

#include "console_util_cfg.h"

void console_RxCpltCallback(void);
void console_task(void);
void console_init(void);

#ifdef CONSOLE_USE_CLI

typedef void console_command_rcv_func_t(char * params);

typedef struct _console_command {
	const char * name;
	console_command_rcv_func_t * console_command_rcv_func;
} console_command_t;

#else

void console_rcv(char * cmd);

#endif

#endif /* UTIL_CONSOLE_UTIL_H_ */
