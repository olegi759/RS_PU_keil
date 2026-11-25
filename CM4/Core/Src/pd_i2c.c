/*
 * pd_i2c.c
 *
 *  Передача кадров PD-протокола по I2C2 (STM32H747, ядро Cortex-M4).
 *
 *  PU (CM4) - master, ModulInterface (STM32F103) - slave (адрес 0x21).
 *
 *  Кадр PD-протокола собирается тем же кодом, что и для UART
 *  (create_piv_packet_cmd / fill_crc16 из Common/common.c), поэтому
 *  содержимое пакета не меняется - меняется только транспорт.
 *
 *  Обмен: master пишет кадр -> slave обрабатывает -> master читает ответ
 *  фиксированным блоком PD_I2C_READ_SIZE. Если ответ ещё не готов, slave
 *  отдаёт нули (CRC не сходится) - master повторяет чтение.
 */

#include "pd_i2c.h"
#include "i2c2.h"
#include "str_util.h"
#include "stm_util.h"

#include <stdio.h>
#include <string.h>

/* Количество попыток чтения ответа (по 1 мс) */
#define PD_I2C_READ_ATTEMPTS    4U

/* Диагностика для отладчика */
volatile uint32_t pd_i2c_tx_count = 0;
volatile uint32_t pd_i2c_rx_ok    = 0;
volatile uint32_t pd_i2c_rx_bad   = 0;
volatile uint32_t pd_i2c_recover_count = 0;
volatile I2C2_Result pd_i2c_last_res = I2C2_RES_OK;

/* Последний отправленный/принятый кадр - для сравнения с приёмной стороной */
static uint8_t  pd_i2c_last_tx[PD_I2C_READ_SIZE];
static uint16_t pd_i2c_last_tx_len = 0;
static uint8_t  pd_i2c_last_rx[PD_I2C_READ_SIZE];
static uint16_t pd_i2c_last_rx_len = 0;

/* Восстановление шины I2C2: если МК-слейв перезапустили посреди обмена,
   шина может "залипнуть" (SDA/SCL удерживаются низко), а периферия - остаться
   в состоянии BUSY. I2C2_BusRecover() делает 9 тактов SCL + STOP и заново
   включает I2C2. Вызываем не чаще, чем раз в 200 мс. */
#define PD_I2C_RECOVER_PERIOD_MS   200U

static uint32_t pd_i2c_last_recover = 0;
/* Подряд идущие неудачные обмены: после нескольких таких подряд освобождаем
   шину принудительно, даже если линии выглядят отпущенными. */
static uint32_t pd_i2c_fail_streak = 0;

#define PD_I2C_FAIL_STREAK_LIMIT   3U

static BOOL pd_i2c_needs_recover(void)
{
	I2C_HandleTypeDef *handle = I2C2_Handle();

	/* линии должны быть отпущены (обе высокие) */
	if (I2C2_Lines() != 0x03u)
	{
		return TRUE;
	}
	/* периферия должна быть в готовности */
	if ((handle != NULL) &&
	    (handle->State != HAL_I2C_STATE_READY) &&
	    (handle->State != HAL_I2C_STATE_RESET))
	{
		return TRUE;
	}
	return FALSE;
}

static void pd_i2c_maybe_recover(BOOL force)
{
	uint32_t now = HAL_GetTick();

	if ((force == FALSE) && (pd_i2c_needs_recover() == FALSE))
	{
		return;
	}
	if ((pd_i2c_last_recover != 0u) && ((now - pd_i2c_last_recover) < PD_I2C_RECOVER_PERIOD_MS))
	{
		return;
	}

	pd_i2c_last_recover = now;
	pd_i2c_recover_count++;
	(void)I2C2_BusRecover();
}

/* ------------------------------------------------------------------------- */
/* Вспомогательные функции                                                   */
/* ------------------------------------------------------------------------- */

static uint8_t *pd_next_arg(char **cursor)
{
	char *p = *cursor;
	char *start;

	if (p == NULL) return NULL;

	while (*p == ' ') p++;
	if (*p == '\0') return NULL;

	start = p;
	while (*p != '\0' && *p != ' ') p++;
	if (*p != '\0')
	{
		*p = '\0';
		p++;
	}
	*cursor = p;
	return (uint8_t *)start;
}

static BOOL pd_parse_hex_byte(const char *s, uint8_t *out)
{
	uint32_t value = 0;
	uint8_t  digits = 0;

	if (s == NULL) return FALSE;

	/* допускаем префикс 0x / 0X */
	if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) s += 2;

	while (*s != '\0')
	{
		char c = *s++;
		uint8_t d;

		if (c >= '0' && c <= '9') d = (uint8_t)(c - '0');
		else if (c >= 'a' && c <= 'f') d = (uint8_t)(c - 'a' + 10);
		else if (c >= 'A' && c <= 'F') d = (uint8_t)(c - 'A' + 10);
		else return FALSE;

		value = (value << 4) | d;
		digits++;
		if (digits > 2U) return FALSE;
	}

	if (digits == 0U) return FALSE;
	*out = (uint8_t)value;
	return TRUE;
}

static void pd_print_hex(const char *prefix, const uint8_t *data, uint16_t len)
{
	char buf[MAX_STR_SIZE];
	int  pos = 0;
	uint16_t i;

	pos += snprintf(&buf[pos], sizeof(buf) - (size_t)pos, CONM "%s %u:", prefix, (unsigned int)len);
	for (i = 0; (i < len) && (pos < (int)sizeof(buf) - 4); i++)
	{
		pos += snprintf(&buf[pos], sizeof(buf) - (size_t)pos, " %02X", (unsigned int)data[i]);
	}
	snprintf(&buf[pos], sizeof(buf) - (size_t)pos, CRLF);
	ulog(buf);
}

/* ------------------------------------------------------------------------- */
/* Обмен по I2C2                                                             */
/* ------------------------------------------------------------------------- */

BOOL PD_I2C_SendReceive(const uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t *rx_len)
{
	uint16_t attempt;
	I2C2_Result res;

	if ((tx == NULL) || (rx == NULL) || (rx_len == NULL) || (tx_len == 0U))
	{
		return FALSE;
	}
	if (!I2C2_IsReady())
	{
		pd_i2c_maybe_recover(TRUE);
		if (!I2C2_IsReady())
		{
			pd_i2c_last_res = I2C2_RES_NOT_INIT;
			return FALSE;
		}
	}

	/* Залипшую шину не трогаем: сначала освобождаем её, иначе каждый обмен
	   будет ждать таймаут HAL (50 мс) и блокировать главный цикл CM4. */
	if (I2C2_Lines() != 0x03u)
	{
		pd_i2c_maybe_recover(TRUE);
		if (I2C2_Lines() != 0x03u)
		{
			pd_i2c_last_res = I2C2_RES_ERROR;
			return FALSE;
		}
	}

	pd_i2c_tx_count++;

	if (tx_len > (uint16_t)sizeof(pd_i2c_last_tx))
	{
		tx_len = (uint16_t)sizeof(pd_i2c_last_tx);
	}
	(void)memcpy(pd_i2c_last_tx, tx, tx_len);
	pd_i2c_last_tx_len = tx_len;

	res = I2C2_Write(PD_I2C_SLAVE_ADDR, tx, tx_len);
	pd_i2c_last_res = res;
	if (res != I2C2_RES_OK)
	{
		pd_i2c_fail_streak++;
		if (pd_i2c_fail_streak >= PD_I2C_FAIL_STREAK_LIMIT)
		{
			/* подряд идут неудачи - освобождаем шину принудительно */
			pd_i2c_fail_streak = 0;
			pd_i2c_maybe_recover(TRUE);
		}
		else
		{
			pd_i2c_maybe_recover((res == I2C2_RES_BUSY) || (res == I2C2_RES_TIMEOUT));
		}
		return FALSE;
	}

	for (attempt = 0; attempt < PD_I2C_READ_ATTEMPTS; attempt++)
	{
		uint16_t field;
		uint16_t len;

		res = I2C2_Read(PD_I2C_SLAVE_ADDR, rx, PD_I2C_READ_SIZE);
		pd_i2c_last_res = res;

		if (res == I2C2_RES_OK)
		{
			/* len = 1 + число байт данных */
			field = ((uint16_t)rx[2] << 8) | (uint16_t)rx[3];
			len = (uint16_t)(6U + field);

			if ((field >= 1U) && (len <= PD_I2C_READ_SIZE) && is_correct_crc16(rx, len))
			{
				*rx_len = len;
				pd_i2c_rx_ok++;
				pd_i2c_fail_streak = 0;
				(void)memcpy(pd_i2c_last_rx, rx, len);
				pd_i2c_last_rx_len = len;
				return TRUE;
			}
		}

		/* шина залипла - дальше ждать бессмысленно */
		if (I2C2_Lines() != 0x03u)
		{
			break;
		}

		HAL_Delay(1);
	}

	pd_i2c_rx_bad++;
	pd_i2c_fail_streak++;
	if (pd_i2c_fail_streak >= PD_I2C_FAIL_STREAK_LIMIT)
	{
		pd_i2c_fail_streak = 0;
		pd_i2c_maybe_recover(TRUE);
	}
	else
	{
		pd_i2c_maybe_recover((pd_i2c_last_res == I2C2_RES_BUSY) || (pd_i2c_last_res == I2C2_RES_TIMEOUT));
	}
	return FALSE;
}

BOOL PD_I2C_Poll(uint8_t dest, uint8_t src, uint8_t *rx, uint16_t *rx_len)
{
	uint8_t tx[8];
	uint16_t tx_len;

	tx_len = create_piv_packet_cmd(tx, dest, src, PD_I2C_CMD_POLL, NULL, 0);
	return PD_I2C_SendReceive(tx, tx_len, rx, rx_len);
}

/* ------------------------------------------------------------------------- */
/* Консоль: PDI2C PING | SEND | STRESS | INFO                                */
/* ------------------------------------------------------------------------- */

static void pd_console_ping(char *cursor)
{
	uint8_t dest = PD_I2C_DEF_PD_ADDR;
	uint8_t src  = PD_I2C_DEF_PU_ADDR;
	uint8_t tx[8];
	uint8_t rx[PD_I2C_READ_SIZE];
	uint16_t tx_len;
	uint16_t rx_len = 0;
	uint8_t *arg;

	arg = pd_next_arg(&cursor);
	if (arg != NULL) (void)pd_parse_hex_byte((const char *)arg, &dest);
	arg = pd_next_arg(&cursor);
	if (arg != NULL) (void)pd_parse_hex_byte((const char *)arg, &src);

	tx_len = create_piv_packet_cmd(tx, dest, src, PD_I2C_CMD_POLL, NULL, 0);
	pd_print_hex("PDI2C_TX", tx, tx_len);

	if (PD_I2C_SendReceive(tx, tx_len, rx, &rx_len))
	{
		pd_print_hex("PDI2C_RX", rx, rx_len);
		ulog_fmt(CONM "PDI2C_OK dest=0x%02X src=0x%02X cmd=0x%02X" CRLF,
		         (unsigned int)dest, (unsigned int)src, (unsigned int)rx[4]);
	}
	else
	{
		ulog_fmt(CONM "PDI2C_ERROR %s" CRLF, I2C2_ResultStr(pd_i2c_last_res));
	}
}

static void pd_console_send(char *cursor)
{
	uint8_t tx[PD_I2C_READ_SIZE];
	uint8_t rx[PD_I2C_READ_SIZE];
	uint16_t tx_len = 0;
	uint16_t rx_len = 0;
	uint8_t *arg;

	while ((tx_len < sizeof(tx)) && ((arg = pd_next_arg(&cursor)) != NULL))
	{
		if (!pd_parse_hex_byte((const char *)arg, &tx[tx_len]))
		{
			ulog_fmt(CONM "PDI2C_ERROR bad byte: %s" CRLF, (char *)arg);
			return;
		}
		tx_len++;
	}

	if (tx_len < 7U)
	{
		ulog_fmt(CONM "PDI2C_ERROR frame too short (%u)" CRLF, (unsigned int)tx_len);
		return;
	}

	pd_print_hex("PDI2C_TX", tx, tx_len);

	if (PD_I2C_SendReceive(tx, tx_len, rx, &rx_len))
	{
		pd_print_hex("PDI2C_RX", rx, rx_len);
		ulog_fmt(CONM "PDI2C_OK" CRLF);
	}
	else
	{
		ulog_fmt(CONM "PDI2C_ERROR %s rx_bad=%u" CRLF,
		         I2C2_ResultStr(pd_i2c_last_res), (unsigned int)pd_i2c_rx_bad);
	}
}

static void pd_console_stress(char *cursor)
{
	uint8_t *arg;
	uint32_t count = 100;
	uint32_t i;
	uint32_t ok = 0;
	uint32_t fail = 0;
	uint8_t rx[PD_I2C_READ_SIZE];
	uint16_t rx_len = 0;

	arg = pd_next_arg(&cursor);
	if (arg != NULL)
	{
		count = (uint32_t)str2int((char *)arg);
		if (count == 0U) count = 1;
		if (count > 10000U) count = 10000U;
	}

	ulog_fmt(CONM "PDI2C_STRESS start count=%u" CRLF, (unsigned int)count);

	for (i = 0; i < count; i++)
	{
		/* длинный прогон не должен ронять МК по сторожевому таймеру */
		HAL_IWDG_Refresh(&hiwdg2);

		if (PD_I2C_Poll(PD_I2C_DEF_PD_ADDR, PD_I2C_DEF_PU_ADDR, rx, &rx_len)) ok++;
		else fail++;
	}

	ulog_fmt(CONM "PDI2C_STRESS done ok=%u fail=%u last=%s" CRLF,
	         (unsigned int)ok, (unsigned int)fail, I2C2_ResultStr(pd_i2c_last_res));
}

static void pd_console_info(void)
{
	if (pd_i2c_last_tx_len > 0U) pd_print_hex("PDI2C_LAST_TX", pd_i2c_last_tx, pd_i2c_last_tx_len);
	if (pd_i2c_last_rx_len > 0U) pd_print_hex("PDI2C_LAST_RX", pd_i2c_last_rx, pd_i2c_last_rx_len);
	ulog_fmt(CONM "PDI2C addr=0x%02X tx=%u rx_ok=%u rx_bad=%u recover=%u last=%s" CRLF,
	         (unsigned int)PD_I2C_SLAVE_ADDR,
	         (unsigned int)pd_i2c_tx_count,
	         (unsigned int)pd_i2c_rx_ok,
	         (unsigned int)pd_i2c_rx_bad,
	         (unsigned int)pd_i2c_recover_count,
	         I2C2_ResultStr(pd_i2c_last_res));
}

void PD_I2C_Console(char *msg)
{
	char *cursor;
	uint8_t *cmd;

	if (msg == NULL) return;
	if ((strlen(msg) < 5U) || (strncmp(msg, "PDI2C", 5U) != 0)) return;

	cursor = msg + 5;
	cmd = pd_next_arg(&cursor);

	if (cmd == NULL)
	{
		pd_console_info();
		ulog_fmt(CONM "PDI2C: PING [dest] [src] | SEND <hex...> | STRESS [n] | INFO" CRLF);
		return;
	}

	if (strcaseequals((char *)cmd, "PING"))   { pd_console_ping(cursor); return; }
	if (strcaseequals((char *)cmd, "SEND"))   { pd_console_send(cursor); return; }
	if (strcaseequals((char *)cmd, "STRESS")) { pd_console_stress(cursor); return; }
	if (strcaseequals((char *)cmd, "INFO"))   { pd_console_info(); return; }

	ulog_fmt(CONM "PDI2C_ERROR unknown command: %s" CRLF, (char *)cmd);
}
