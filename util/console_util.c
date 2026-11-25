/*
 * cli_util.c
 *
 *  Created on: 6 февр. 2026 г.
 *      Author: Rafael Boltachev
 */


#include "console_util.h"
#include <string.h>
#ifdef CONSOLE_USE_CLI
#include <str_util.h>
#endif


static volatile char console_rcv_char;
static char console_rcv_buf[CONSOLE_MAX_CMD_SIZE] = {0};
static char console_cmd_buf[CONSOLE_MAX_CMD_SIZE] = {0};
static int console_index = 0;
extern UART_HandleTypeDef CONSOLE_HUART;
#ifdef CONSOLE_USE_ECHO
static volatile char console_snd_char;
#endif
#ifdef CONSOLE_USE_CLI
const console_command_t console_commands[CONSOLE_COMMANDS_SIZE] = 	{ CONSOLE_COMMANDS };
#endif

void console_RxCpltCallback(void)
{
#ifdef CONSOLE_USE_ECHO
	console_snd_char  = console_rcv_char;
	HAL_UART_Transmit_IT(&CONSOLE_HUART, (uint8_t *)&console_snd_char, 1);
#endif

	if ((console_rcv_char == '\r') || (console_rcv_char == '\n')) {
		if (console_index > 0) {
			memcpy(console_cmd_buf, console_rcv_buf, console_index);
			console_cmd_buf[console_index] = '\0';
		}
		console_index = 0;
	} else {
		console_rcv_buf[console_index] = console_rcv_char;
		if (console_index < CONSOLE_MAX_CMD_SIZE - 1) {
			console_index++;
		}	else{
			console_index = 0;
		}
	}

	HAL_UART_Receive_IT(&CONSOLE_HUART, (uint8_t *)&console_rcv_char, 1);
}

#ifdef CONSOLE_USE_CLI
void console_task_cmd(void)
{
	int i, len;


	for (i = 0; i < CONSOLE_COMMANDS_SIZE; i++) {

		len = strlen(console_commands[i].name);
		if ( (memcmp(console_cmd_buf, console_commands[i].name, len) == 0) && ((console_cmd_buf[len] == ' ') || (console_cmd_buf[len] == '\0')) ) {
			console_commands[i].console_command_rcv_func(&console_cmd_buf[len + 1]);
			break;
		}
	}

	if (strcmp_s(console_cmd_buf, strlen(console_cmd_buf), "HELP")) {
		for (i = 0; i < CONSOLE_COMMANDS_SIZE; i++) {
			HAL_UART_Transmit(&CONSOLE_HUART, (uint8_t *)console_commands[i].name, strlen(console_commands[i].name), 100);
			HAL_UART_Transmit(&CONSOLE_HUART, (uint8_t *)"\r\n", 2, 100);
		}
	}
}
#endif

void console_task(void)
{
	if (console_cmd_buf[0] != '\0') {
#ifdef CONSOLE_USE_CLI
		console_task_cmd();
#else
		console_rcv(console_cmd_buf);
#endif
		console_cmd_buf[0] = '\0';
	}
}


void console_init(void)
{
	HAL_UART_Receive_IT(&CONSOLE_HUART, (uint8_t *)&console_rcv_char, 1);
}
