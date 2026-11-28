/*
 * i2c50.c
 *
 *  Обмен с платой ModulInterface (STM32F103) по I2C2: пакеты по 50 байт с
 *  контролем CRC16 (STM32H747, ядро Cortex-M7, мастер).
 *
 *  Запрос и ответ - фиксированные пакеты I2C50_PACKET_SIZE байт; формат и
 *  типы описаны в i2c50.h. CRC16 CCITT-FALSE считается Crc16() - той же
 *  функцией, что и в PD-протоколе, и совпадает с CalcCRC16() на F103.
 *
 *  Обмен: мастер пишет запрос (50 байт, STOP), затем читает ответ (50 байт).
 *  Слейв формирует ответ в прерывании по STOP; пока он это делает, периферия
 *  F103 растягивает SCL на адресе чтения, поэтому ответ всегда готов.
 *  Неудачный обмен повторяется (I2C50_ATTEMPTS), при серии неудач шина
 *  освобождается (I2C2_BusRecover).
 *
 *  Диагностика - глобальные счётчики i2c50_* (видны в отладчике) и
 *  последние запрос/ответ (i2c50_last_req / i2c50_last_resp).
 */

#include "i2c50.h"

#include "math_util.h"      /* Crc16 */

#include <string.h>

/* ------------------------------------------------------------------------- */
/* Диагностика                                                               */
/* ------------------------------------------------------------------------- */

volatile uint32_t i2c50_tx_count = 0;
volatile uint32_t i2c50_rx_ok = 0;
volatile uint32_t i2c50_err_crc = 0;
volatile uint32_t i2c50_err_hdr = 0;
volatile uint32_t i2c50_err_seq = 0;
volatile uint32_t i2c50_err_type = 0;
volatile uint32_t i2c50_nack = 0;
volatile uint32_t i2c50_recover_count = 0;
volatile uint32_t i2c50_ping_ok = 0;
volatile uint32_t i2c50_echo_ok = 0;
volatile uint32_t i2c50_write_ok = 0;
volatile uint32_t i2c50_read_ok = 0;

volatile I2C2_Result i2c50_last_res = I2C2_RES_OK;
volatile uint8_t i2c50_last_type = 0;
volatile uint8_t i2c50_last_seq = 0;
volatile uint8_t i2c50_last_len = 0;
volatile uint8_t i2c50_last_err = 0;
volatile uint8_t i2c50_slave_rx = 0;
volatile uint8_t i2c50_slave_crc = 0;

uint8_t i2c50_last_req[I2C50_PACKET_SIZE];
uint8_t i2c50_last_resp[I2C50_PACKET_SIZE];

static BOOL     i2c50_ready = FALSE;
static uint8_t  i2c50_seq = 0;
static uint32_t i2c50_fail_streak = 0;
static uint32_t i2c50_last_recover = 0;
static uint32_t i2c50_last_poll = 0;
static uint32_t i2c50_poll_count = 0;
static uint32_t i2c50_poll_ok = 0;
static uint32_t i2c50_poll_fail = 0;

/* ------------------------------------------------------------------------- */
/* Вспомогательные функции                                                   */
/* ------------------------------------------------------------------------- */

static uint16_t i2c50_crc(const uint8_t *data, uint16_t len)
{
	return Crc16((unsigned char *)data, (unsigned short)len);
}

/* Шина должна быть отпущена, периферия - готова */
static BOOL i2c50_needs_recover(void)
{
	I2C_HandleTypeDef *handle = I2C2_Handle();

	if (I2C2_Lines() != 0x03U) {
		return TRUE;
	}
	if ((handle != NULL) &&
	    (handle->State != HAL_I2C_STATE_READY) &&
	    (handle->State != HAL_I2C_STATE_RESET))
	{
		return TRUE;
	}
	return FALSE;
}

static void i2c50_maybe_recover(BOOL force)
{
	uint32_t now = HAL_GetTick();

	if ((force == FALSE) && (i2c50_needs_recover() == FALSE)) {
		return;
	}
	if ((i2c50_last_recover != 0U) &&
	    ((now - i2c50_last_recover) < I2C50_RECOVER_PERIOD_MS))
	{
		return;
	}

	i2c50_last_recover = now;
	i2c50_recover_count++;
	(void)I2C2_BusRecover();
}

/* Учёт неудачной попытки: серия неудач - освобождаем шину */
static void i2c50_failed(I2C2_Result res)
{
	i2c50_fail_streak++;

	if (i2c50_fail_streak >= I2C50_FAIL_STREAK_LIMIT) {
		i2c50_fail_streak = 0;
		i2c50_maybe_recover(TRUE);
	}
	else {
		i2c50_maybe_recover((res == I2C2_RES_BUSY) || (res == I2C2_RES_TIMEOUT));
	}

	HAL_Delay(I2C50_RETRY_DELAY_MS);
}

static void i2c50_build_request(uint8_t *buf, uint8_t type, uint8_t seq,
                                const uint8_t *payload, uint8_t plen)
{
	uint16_t crc;
	uint8_t i;

	(void)memset(buf, 0, I2C50_PACKET_SIZE);

	buf[I2C50_OFS_MAGIC]   = I2C50_MAGIC;
	buf[I2C50_OFS_VERSION] = I2C50_VERSION;
	buf[I2C50_OFS_TYPE]    = type;
	buf[I2C50_OFS_SEQ]     = seq;
	buf[I2C50_OFS_LEN]     = plen;

	for (i = 0U; (i < plen) && (payload != NULL); i++) {
		buf[I2C50_OFS_PAYLOAD + i] = payload[i];
	}

	crc = i2c50_crc(buf, I2C50_OFS_CRC);
	buf[I2C50_OFS_CRC]     = (uint8_t)(crc >> 8);
	buf[I2C50_OFS_CRC + 1] = (uint8_t)(crc & 0xFFU);
}

/* Проверка ответа. TRUE - ответ корректен (magic/version/len/CRC/seq/тип). */
static BOOL i2c50_parse_reply(const uint8_t *buf, uint8_t exp_type, uint8_t exp_seq,
                              uint8_t *reply, uint8_t *rlen)
{
	uint16_t crc_rx;
	uint16_t crc_calc;
	uint8_t  len;
	uint8_t  type;
	uint8_t  seq;

	if (buf[I2C50_OFS_MAGIC] != I2C50_MAGIC) {
		i2c50_err_hdr++;
		return FALSE;
	}
	if (buf[I2C50_OFS_VERSION] != I2C50_VERSION) {
		i2c50_err_hdr++;
		return FALSE;
	}

	len = buf[I2C50_OFS_LEN];
	if (len > I2C50_MAX_PAYLOAD) {
		i2c50_err_hdr++;
		return FALSE;
	}

	crc_rx   = ((uint16_t)buf[I2C50_OFS_CRC] << 8) | (uint16_t)buf[I2C50_OFS_CRC + 1U];
	crc_calc = i2c50_crc(buf, I2C50_OFS_CRC);

	if (crc_rx != crc_calc) {
		i2c50_err_crc++;
		return FALSE;
	}

	type = buf[I2C50_OFS_TYPE];
	seq  = buf[I2C50_OFS_SEQ];

	if (seq != exp_seq) {
		i2c50_err_seq++;
		return FALSE;
	}

	if (type == I2C50_TYPE_ERROR) {
		/* слейв сообщил об ошибке (CRC/заголовок/тип) */
		i2c50_last_err = (len > 0U) ? buf[I2C50_OFS_PAYLOAD] : 0xFFU;
		i2c50_err_type++;
		return FALSE;
	}

	if (type != (uint8_t)(exp_type | I2C50_TYPE_REPLY)) {
		i2c50_err_type++;
		return FALSE;
	}

	if ((reply != NULL) && (rlen != NULL)) {
		(void)memcpy(reply, &buf[I2C50_OFS_PAYLOAD], len);
		*rlen = len;
	}

	return TRUE;
}

/* ------------------------------------------------------------------------- */
/* Обмен                                                                     */
/* ------------------------------------------------------------------------- */

BOOL I2C50_Exchange(uint8_t type, const uint8_t *payload, uint8_t plen,
                    uint8_t *reply, uint8_t *rlen)
{
	uint8_t      req[I2C50_PACKET_SIZE];
	uint8_t      resp[I2C50_PACKET_SIZE];
	uint8_t      attempt;
	I2C2_Result  res;

	if (plen > I2C50_MAX_PAYLOAD) {
		i2c50_last_res = I2C2_RES_PARAM;
		return FALSE;
	}

	if (!I2C2_IsReady()) {
		i2c50_maybe_recover(TRUE);
		if (!I2C2_IsReady()) {
			i2c50_last_res = I2C2_RES_NOT_INIT;
			return FALSE;
		}
	}

	/* Залипшую шину не трогаем: сначала освобождаем, иначе каждый обмен
	   будет ждать таймаут HAL и блокировать главный цикл CM4. */
	if (I2C2_Lines() != 0x03U) {
		i2c50_maybe_recover(TRUE);
		if (I2C2_Lines() != 0x03U) {
			i2c50_last_res = I2C2_RES_ERROR;
			return FALSE;
		}
	}

	i2c50_seq++;
	i2c50_build_request(req, type, i2c50_seq, payload, plen);

	(void)memcpy(i2c50_last_req, req, I2C50_PACKET_SIZE);
	i2c50_last_type = type;
	i2c50_last_seq  = i2c50_seq;
	i2c50_last_len  = plen;
	i2c50_last_err  = I2C50_ERR_NONE;

	for (attempt = 0U; attempt < I2C50_ATTEMPTS; attempt++) {
		i2c50_tx_count++;

		res = I2C2_Write(I2C50_SLAVE_ADDR, req, I2C50_PACKET_SIZE);
		i2c50_last_res = res;
		if (res != I2C2_RES_OK) {
			if (res == I2C2_RES_NACK) {
				i2c50_nack++;
			}
			i2c50_failed(res);
			continue;
		}

		res = I2C2_Read(I2C50_SLAVE_ADDR, resp, I2C50_PACKET_SIZE);
		i2c50_last_res = res;
		if (res != I2C2_RES_OK) {
			if (res == I2C2_RES_NACK) {
				i2c50_nack++;
			}
			i2c50_failed(res);
			continue;
		}

		(void)memcpy(i2c50_last_resp, resp, I2C50_PACKET_SIZE);

		if (i2c50_parse_reply(resp, type, i2c50_seq, reply, rlen)) {
			i2c50_rx_ok++;
			i2c50_fail_streak = 0;
			return TRUE;
		}

		i2c50_failed(I2C2_RES_ERROR);
	}

	return FALSE;
}

BOOL I2C50_Ping(void)
{
	uint8_t reply[I2C50_MAX_PAYLOAD];
	uint8_t rlen = 0U;

	if (!I2C50_Exchange(I2C50_TYPE_PING, NULL, 0U, reply, &rlen)) {
		return FALSE;
	}
	if ((rlen < 7U) || (reply[0] != I2C50_ERR_NONE)) {
		return FALSE;
	}

	/* [1..2] - принято пакетов, [3..4] - ошибок CRC на стороне слейва,
	   [5] - последний тип, [6] - длина записанных данных */
	i2c50_slave_rx  = reply[1];
	i2c50_slave_crc = reply[3];
	i2c50_ping_ok++;

	return TRUE;
}

/* ------------------------------------------------------------------------- */
/* Периодический опрос                                                       */
/* ------------------------------------------------------------------------- */

/* Проверка целостности канала: мастер отправляет 43 байта, слейв возвращает
   их же (ECHO). */
static BOOL i2c50_task_echo(uint32_t tag)
{
	uint8_t tx[I2C50_MAX_PAYLOAD];
	uint8_t rx[I2C50_MAX_PAYLOAD];
	uint8_t rlen = 0U;
	uint8_t i;

	for (i = 0U; i < I2C50_MAX_PAYLOAD; i++) {
		tx[i] = (uint8_t)(tag + i);
	}

	if (!I2C50_Exchange(I2C50_TYPE_ECHO, tx, I2C50_MAX_PAYLOAD, rx, &rlen)) {
		return FALSE;
	}
	if ((rlen != I2C50_MAX_PAYLOAD) || (memcmp(tx, rx, I2C50_MAX_PAYLOAD) != 0)) {
		return FALSE;
	}

	i2c50_echo_ok++;
	return TRUE;
}

/* Запись и обратное чтение данных (WRITE + READ). */
static BOOL i2c50_task_write_read(uint32_t tag)
{
	uint8_t tx[I2C50_MAX_PAYLOAD];
	uint8_t rx[I2C50_MAX_PAYLOAD];
	uint8_t ack[I2C50_MAX_PAYLOAD];
	uint8_t alen = 0U;
	uint8_t rlen = 0U;
	uint8_t i;

	for (i = 0U; i < I2C50_MAX_PAYLOAD; i++) {
		tx[i] = (uint8_t)(tag ^ (uint32_t)(i * 7U));
	}

	if (!I2C50_Exchange(I2C50_TYPE_WRITE, tx, I2C50_MAX_PAYLOAD, ack, &alen)) {
		return FALSE;
	}
	if (alen < 2U) {
		return FALSE;
	}
	i2c50_write_ok++;

	if (!I2C50_Exchange(I2C50_TYPE_READ, NULL, 0U, rx, &rlen)) {
		return FALSE;
	}
	if ((rlen != I2C50_MAX_PAYLOAD) || (memcmp(tx, rx, I2C50_MAX_PAYLOAD) != 0)) {
		return FALSE;
	}

	i2c50_read_ok++;
	return TRUE;
}

void I2C50_Task(void)
{
	uint32_t now = HAL_GetTick();

	if (i2c50_ready == FALSE) {
		return;
	}
	if ((now - i2c50_last_poll) < I2C50_POLL_PERIOD_MS) {
		return;
	}
	i2c50_last_poll = now;
	i2c50_poll_count++;


	if (I2C50_Ping()) {
		i2c50_poll_ok++;
	}
	else {
		i2c50_poll_fail++;
	}

	if ((i2c50_poll_count % 5U) == 0U) {
		(void)i2c50_task_echo(i2c50_poll_count);
	}
	if ((i2c50_poll_count % 10U) == 0U) {
		(void)i2c50_task_write_read(i2c50_poll_count);
	}
}

/* ------------------------------------------------------------------------- */
/* Инициализация                                                             */
/* ------------------------------------------------------------------------- */

BOOL I2C50_Init(void)
{
	i2c50_tx_count = 0;
	i2c50_rx_ok = 0;
	i2c50_err_crc = 0;
	i2c50_err_hdr = 0;
	i2c50_err_seq = 0;
	i2c50_err_type = 0;
	i2c50_nack = 0;
	i2c50_recover_count = 0;
	i2c50_ping_ok = 0;
	i2c50_echo_ok = 0;
	i2c50_write_ok = 0;
	i2c50_read_ok = 0;
	i2c50_last_res = I2C2_RES_OK;
	i2c50_last_type = 0;
	i2c50_last_seq = 0;
	i2c50_last_len = 0;
	i2c50_last_err = 0;
	i2c50_slave_rx = 0;
	i2c50_slave_crc = 0;

	(void)memset(i2c50_last_req, 0, sizeof(i2c50_last_req));
	(void)memset(i2c50_last_resp, 0, sizeof(i2c50_last_resp));

	i2c50_seq = 0;
	i2c50_fail_streak = 0;
	i2c50_last_recover = 0;
	i2c50_poll_count = 0;
	i2c50_poll_ok = 0;
	i2c50_poll_fail = 0;
	i2c50_last_poll = HAL_GetTick();

	i2c50_ready = I2C2_Init();
	if (i2c50_ready == FALSE) {
		i2c50_last_res = I2C2_RES_ERROR;
	}

	return i2c50_ready;
}

