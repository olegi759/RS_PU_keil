/*
 * i2c2.c
 *
 *  Раздел работы с интерфейсом I2C2 (STM32H747IITx, ядро Cortex-M4).
 *
 *  PF0 - I2C2_SDA, PF1 - I2C2_SCL, AF4, открытый сток,
 *  тактирование от D2PCLK1 (120 МГц), ~100 кГц.
 *
 *  Что делает раздел:
 *    - настраивает выводы, тактирование и периферию I2C2;
 *    - выполняет обмен с устройствами на шине (чтение/запись, регистры);
 *    - сканирует шину и находит адреса устройств;
 *    - восстанавливает "залипшую" шину;
 *    - предоставляет консольную команду "I2C2 ..." для отладки на стенде.
 *
 *  Раздел не изменяет работу I2C1 (PB7/PB8, RTC BQ32000) и остальных
 *  периферийных блоков: используются только I2C2, PF0 и PF1.
 */

#include "i2c2.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_util.h"
#include "stm_util.h"
#include "str_util.h"

/* ------------------------------------------------------------------------- */
/* Состояние раздела                                                         */
/* ------------------------------------------------------------------------- */

/* Ответ консоли на неверные параметры команды */
#define I2C2_STR_PARAM  "PARAM"

/* Имя дескриптора отличается от hi2c2, чтобы не конфликтовать с кодом,
   который может сгенерировать CubeMX при добавлении I2C2 в main.c. */
static I2C_HandleTypeDef i2c2_hi2c;

static BOOL         i2c2_initialized = FALSE;
static I2C2_Result  i2c2_last_result = I2C2_RES_NOT_INIT;

/* Буфер сканирования (0x08..0x77) */
static uint8_t      i2c2_scan_devices[I2C2_MAX_DEVICES];

/* Буфер вывода в консоль: собираем строку и отдаём её одним вызовом ulog(),
   чтобы не ждать окончания передачи после каждого printf. */
#define I2C2_OUT_SIZE   256
static char         i2c2_out_buf[I2C2_OUT_SIZE];
static int          i2c2_out_len = 0;

/* ------------------------------------------------------------------------- */
/* Вспомогательные функции                                                   */
/* ------------------------------------------------------------------------- */

static void i2c2_out_flush(void)
{
	if (i2c2_out_len > 0) {
		i2c2_out_buf[i2c2_out_len] = '\0';
		ulog(i2c2_out_buf);
		i2c2_out_len = 0;
		i2c2_out_buf[0] = '\0';
	}
}

static void i2c2_out(const char *fmt, ...)
{
	va_list args;
	int written;

	if (i2c2_out_len >= (I2C2_OUT_SIZE - 1)) {
		i2c2_out_flush();
	}

	va_start(args, fmt);
	written = vsnprintf(&i2c2_out_buf[i2c2_out_len],
	                    (size_t)(I2C2_OUT_SIZE - i2c2_out_len),
	                    fmt, args);
	va_end(args);

	if (written > 0) {
		i2c2_out_len += written;
		if (i2c2_out_len > (I2C2_OUT_SIZE - 1)) {
			i2c2_out_len = I2C2_OUT_SIZE - 1;
		}
	}
}

/* Небольшая задержка (единицы микросекунд) для восстановления шины.
   Точность не нужна: любой интервал 1..100 мкс подходит для 100 кГц. */
static void i2c2_delay_short(void)
{
	volatile uint32_t i;
	for (i = 0; i < 200U; i++) {
		__NOP();
	}
}

static I2C2_Result i2c2_store(I2C2_Result res)
{
	i2c2_last_result = res;
	return res;
}

static BOOL i2c2_addr_valid(uint8_t addr)
{
	if (addr == I2C2_ADDR_INVALID) {
		return FALSE;
	}
	if (addr > 0x7FU) {
		return FALSE;
	}
	return TRUE;
}

/* Пересчёт результата HAL в результат раздела */
static I2C2_Result i2c2_map_hal(HAL_StatusTypeDef status)
{
	if (status == HAL_OK) {
		return I2C2_RES_OK;
	}
	if (status == HAL_BUSY) {
		return I2C2_RES_BUSY;
	}
	if (status == HAL_TIMEOUT) {
		return I2C2_RES_TIMEOUT;
	}
	if ((i2c2_hi2c.ErrorCode & HAL_I2C_ERROR_AF) != 0U) {
		return I2C2_RES_NACK;
	}
	return I2C2_RES_ERROR;
}

/* ------------------------------------------------------------------------- */
/* Настройка выводов, тактирования и периферии                               */
/* ------------------------------------------------------------------------- */

static void i2c2_gpio_config(void)
{
	GPIO_InitTypeDef gpio = {0};
	RCC_PeriphCLKInitTypeDef clk = {0};

	/* Источник тактирования I2C2 - D2PCLK1, как у I2C1 */
	clk.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
	clk.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
	(void)HAL_RCCEx_PeriphCLKConfig(&clk);

	__HAL_RCC_GPIOF_CLK_ENABLE();

	gpio.Pin       = I2C2_SCL_Pin | I2C2_SDA_Pin;
	gpio.Mode      = GPIO_MODE_AF_OD;
	gpio.Pull      = GPIO_NOPULL;
	gpio.Speed     = GPIO_SPEED_FREQ_LOW;
	gpio.Alternate = I2C2_GPIO_AF;
	HAL_GPIO_Init(GPIOF, &gpio);

	__HAL_RCC_I2C2_CLK_ENABLE();
}

static BOOL i2c2_peripheral_config(void)
{
	i2c2_hi2c.Instance = I2C2;

	i2c2_hi2c.Init.Timing          = I2C2_TIMING;
	i2c2_hi2c.Init.OwnAddress1     = 0;
	i2c2_hi2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	i2c2_hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2c2_hi2c.Init.OwnAddress2     = 0;
	i2c2_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	i2c2_hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2c2_hi2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&i2c2_hi2c) != HAL_OK) {
		return FALSE;
	}
	if (HAL_I2CEx_ConfigAnalogFilter(&i2c2_hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
		return FALSE;
	}
	if (HAL_I2CEx_ConfigDigitalFilter(&i2c2_hi2c, 0) != HAL_OK) {
		return FALSE;
	}
	return TRUE;
}

/* Освобождение "залипшей" шины: до 9 тактов SCL и STOP на выводах.
   Вызывается при выключенной периферии I2C2 (выводы - GPIO, открытый сток). */
static void i2c2_lines_recover(void)
{
	GPIO_InitTypeDef gpio = {0};
	uint8_t i;

	gpio.Pin   = I2C2_SCL_Pin | I2C2_SDA_Pin;
	gpio.Mode  = GPIO_MODE_OUTPUT_OD;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &gpio);

	HAL_GPIO_WritePin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_SET);
	i2c2_delay_short();

	/* Такты SCL - даём устройству вытолкнуть зажатые данные */
	for (i = 0; i < 9U; i++) {
		HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_RESET);
		i2c2_delay_short();
		HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_SET);
		i2c2_delay_short();
	}

	/* Условие STOP: SDA поднимается при высоком SCL */
	HAL_GPIO_WritePin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin, GPIO_PIN_RESET);
	i2c2_delay_short();
	HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_SET);
	i2c2_delay_short();
	HAL_GPIO_WritePin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin, GPIO_PIN_SET);
	i2c2_delay_short();
}

/* ------------------------------------------------------------------------- */
/* Инициализация                                                             */
/* ------------------------------------------------------------------------- */

BOOL I2C2_Init(void)
{
	i2c2_initialized = FALSE;
	i2c2_last_result = I2C2_RES_NOT_INIT;

	if (i2c2_hi2c.Instance != NULL) {
		(void)HAL_I2C_DeInit(&i2c2_hi2c);
	}

	i2c2_gpio_config();

	/* Если линии прижаты к нулю (обмен был прерван), сначала освобождаем шину */
	if ((I2C2_Lines() & 0x03U) != 0x03U) {
		i2c2_lines_recover();
		i2c2_gpio_config();
	}

	if (!i2c2_peripheral_config()) {
		i2c2_last_result = I2C2_RES_ERROR;
		return FALSE;
	}

	i2c2_initialized  = TRUE;
	i2c2_last_result  = I2C2_RES_OK;
	return TRUE;
}

BOOL I2C2_IsReady(void)
{
	return i2c2_initialized;
}

I2C_HandleTypeDef *I2C2_Handle(void)
{
	return &i2c2_hi2c;
}

I2C2_Result I2C2_LastResult(void)
{
	return i2c2_last_result;
}

const char *I2C2_ResultStr(I2C2_Result res)
{
	switch (res) {
		case I2C2_RES_OK:       return "OK";
		case I2C2_RES_ERROR:    return "ERROR";
		case I2C2_RES_TIMEOUT:  return "TIMEOUT";
		case I2C2_RES_NACK:     return "NACK";
		case I2C2_RES_BUSY:     return "BUSY";
		case I2C2_RES_PARAM:    return "PARAM";
		case I2C2_RES_NOT_INIT: return "NOT_INIT";
		default:                return "UNKNOWN";
	}
}

uint8_t I2C2_Lines(void)
{
	uint8_t lines = 0;

	if (HAL_GPIO_ReadPin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin) == GPIO_PIN_SET) {
		lines |= 0x01U;
	}
	if (HAL_GPIO_ReadPin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin) == GPIO_PIN_SET) {
		lines |= 0x02U;
	}
	return lines;
}

/* ------------------------------------------------------------------------- */
/* Обмен данными                                                             */
/* ------------------------------------------------------------------------- */

static I2C2_Result i2c2_probe_raw(uint8_t addr)
{
	return i2c2_map_hal(HAL_I2C_IsDeviceReady(&i2c2_hi2c,
	                                          (uint16_t)((uint16_t)addr << 1),
	                                          2U,
	                                          I2C2_PROBE_TIMEOUT_MS));
}

I2C2_Result I2C2_Probe(uint8_t addr)
{
	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (!i2c2_addr_valid(addr)) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return i2c2_store(i2c2_probe_raw(addr));
}

I2C2_Result I2C2_Scan(uint8_t *devices, uint16_t capacity, uint16_t *count)
{
	uint16_t found = 0;
	uint8_t addr;

	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (count == NULL) {
		return i2c2_store(I2C2_RES_PARAM);
	}

	for (addr = I2C2_ADDR_FIRST; addr <= I2C2_ADDR_LAST; addr++) {
		if (i2c2_probe_raw(addr) == I2C2_RES_OK) {
			if ((devices != NULL) && (found < capacity)) {
				devices[found] = addr;
			}
			found++;
		}
	}

	*count = found;
	return i2c2_store(I2C2_RES_OK);
}

I2C2_Result I2C2_Write(uint8_t addr, const uint8_t *data, uint16_t size)
{
	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (!i2c2_addr_valid(addr) || (data == NULL) || (size == 0U)) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return i2c2_store(i2c2_map_hal(HAL_I2C_Master_Transmit(&i2c2_hi2c,
	                                                      (uint16_t)((uint16_t)addr << 1),
	                                                      (uint8_t *)data,
	                                                      size,
	                                                      I2C2_TIMEOUT_MS)));
}

I2C2_Result I2C2_Read(uint8_t addr, uint8_t *data, uint16_t size)
{
	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (!i2c2_addr_valid(addr) || (data == NULL) || (size == 0U)) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return i2c2_store(i2c2_map_hal(HAL_I2C_Master_Receive(&i2c2_hi2c,
	                                                     (uint16_t)((uint16_t)addr << 1),
	                                                     data,
	                                                     size,
	                                                     I2C2_TIMEOUT_MS)));
}

I2C2_Result I2C2_WriteReg8(uint8_t addr, uint8_t reg, const uint8_t *data, uint16_t size)
{
	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (!i2c2_addr_valid(addr) || (data == NULL) || (size == 0U)) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return i2c2_store(i2c2_map_hal(HAL_I2C_Mem_Write(&i2c2_hi2c,
	                                                 (uint16_t)((uint16_t)addr << 1),
	                                                 (uint16_t)reg,
	                                                 I2C_MEMADD_SIZE_8BIT,
	                                                 (uint8_t *)data,
	                                                 size,
	                                                 I2C2_TIMEOUT_MS)));
}

I2C2_Result I2C2_ReadReg8(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t size)
{
	if (!i2c2_initialized) {
		return i2c2_store(I2C2_RES_NOT_INIT);
	}
	if (!i2c2_addr_valid(addr) || (data == NULL) || (size == 0U)) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return i2c2_store(i2c2_map_hal(HAL_I2C_Mem_Read(&i2c2_hi2c,
	                                                (uint16_t)((uint16_t)addr << 1),
	                                                (uint16_t)reg,
	                                                I2C_MEMADD_SIZE_8BIT,
	                                                data,
	                                                size,
	                                                I2C2_TIMEOUT_MS)));
}

I2C2_Result I2C2_WriteReg8Byte(uint8_t addr, uint8_t reg, uint8_t value)
{
	return I2C2_WriteReg8(addr, reg, &value, 1U);
}

I2C2_Result I2C2_ReadReg8Byte(uint8_t addr, uint8_t reg, uint8_t *value)
{
	if (value == NULL) {
		return i2c2_store(I2C2_RES_PARAM);
	}
	return I2C2_ReadReg8(addr, reg, value, 1U);
}

/* ------------------------------------------------------------------------- */
/* Восстановление шины                                                       */
/* ------------------------------------------------------------------------- */

I2C2_Result I2C2_BusRecover(void)
{
	I2C2_Result res;

	i2c2_initialized = FALSE;

	if (i2c2_hi2c.Instance != NULL) {
		(void)HAL_I2C_DeInit(&i2c2_hi2c);
	}
	else {
		i2c2_hi2c.Instance = I2C2;
	}

	/* Выводы переводим в GPIO и "выталкиваем" шину */
	i2c2_gpio_config();
	i2c2_lines_recover();

	/* Возвращаем периферию в рабочее состояние */
	i2c2_gpio_config();
	res = i2c2_peripheral_config() ? I2C2_RES_OK : I2C2_RES_ERROR;

	i2c2_initialized = (res == I2C2_RES_OK);
	return i2c2_store(res);
}

/* ------------------------------------------------------------------------- */
/* Консольная команда "I2C2 ..."                                             */
/* ------------------------------------------------------------------------- */

/* Достаёт из строки очередной параметр, разделённый пробелами.
   Разделитель заменяется на '\0', поэтому строка изменяется. */
static char *i2c2_next_arg(char **cursor)
{
	char *ptr = *cursor;
	char *start;

	while (*ptr == ' ') {
		ptr++;
	}
	if (*ptr == '\0') {
		*cursor = ptr;
		return NULL;
	}

	start = ptr;
	while ((*ptr != ' ') && (*ptr != '\0')) {
		ptr++;
	}
	if (*ptr == ' ') {
		*ptr = '\0';
		ptr++;
	}

	*cursor = ptr;
	return start;
}

static BOOL i2c2_parse_ulong(char *str, unsigned long *value)
{
	char *end = NULL;
	unsigned long parsed;

	if ((str == NULL) || (value == NULL)) {
		return FALSE;
	}
	if (*str == '\0') {
		return FALSE;
	}
	if (*str == '-') {
		return FALSE;
	}

	parsed = strtoul(str, &end, 0);
	if (end == str) {
		return FALSE;
	}
	if ((end != NULL) && (*end != '\0')) {
		return FALSE;
	}

	*value = parsed;
	return TRUE;
}

static void i2c2_out_help(void)
{
	i2c2_out(CONM "I2C2_INFO    - state of the interface" CRLF);
	i2c2_out(CONM "I2C2_SCAN    - scan the bus, print found addresses" CRLF);
	i2c2_out(CONM "I2C2_PING 0xNN - check one device" CRLF);
	i2c2_out(CONM "I2C2_RD 0xNN 0xRR [len] - read registers" CRLF);
	i2c2_out(CONM "I2C2_WR 0xNN 0xRR 0xVV - write one register" CRLF);
	i2c2_out(CONM "I2C2_RECOVER - release a stuck bus" CRLF);
}

static void i2c2_out_info(void)
{
	uint8_t lines = I2C2_Lines();

	i2c2_out(CONM "I2C2_INFO pins: PF0=SDA, PF1=SCL, AF4, open-drain" CRLF);
	i2c2_out(CONM "I2C2_INFO timing=0x%08lX ready=%d" CRLF,
	         (unsigned long)I2C2_TIMING, (int)(i2c2_initialized ? 1 : 0));
	i2c2_out(CONM "I2C2_INFO SCL=%d SDA=%d last=%s" CRLF,
	         (int)((lines & 0x02U) ? 1 : 0),
	         (int)((lines & 0x01U) ? 1 : 0),
	         I2C2_ResultStr(i2c2_last_result));
}

static void i2c2_console_scan(void)
{
	uint16_t found = 0;
	uint16_t i;
	I2C2_Result res;

	res = I2C2_Scan(i2c2_scan_devices, I2C2_MAX_DEVICES, &found);

	if (res != I2C2_RES_OK) {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_SCAN") " %s" CRLF, I2C2_ResultStr(res));
		return;
	}

	if (found == 0U) {
		i2c2_out(CONM "I2C2_INFO no devices on the bus" CRLF);
	}
	else {
		for (i = 0; i < found; i++) {
			i2c2_out(CONM "I2C2_FOUND 0x%02X" CRLF, (unsigned int)i2c2_scan_devices[i]);
		}
	}

	i2c2_out(CONM STR_OK_CREATE("I2C2_SCAN") " count=%d" CRLF, (int)found);
}

static void i2c2_console_ping(char *arg)
{
	unsigned long addr;
	I2C2_Result res;

	if (!i2c2_parse_ulong(arg, &addr) || (addr > 0x7FU)) {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_PING") " " I2C2_STR_PARAM CRLF);
		return;
	}

	res = I2C2_Probe((uint8_t)addr);
	if (res == I2C2_RES_OK) {
		i2c2_out(CONM STR_OK_CREATE("I2C2_PING") " 0x%02X" CRLF, (unsigned int)addr);
	}
	else {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_PING") " 0x%02X %s" CRLF,
		         (unsigned int)addr, I2C2_ResultStr(res));
	}
}

static void i2c2_console_read(char *addr_arg, char *reg_arg, char *len_arg)
{
	unsigned long addr;
	unsigned long reg;
	unsigned long len = 1;
	uint8_t data[32];
	uint16_t i;
	I2C2_Result res;

	if (!i2c2_parse_ulong(addr_arg, &addr) || (addr > 0x7FU) ||
	    !i2c2_parse_ulong(reg_arg, &reg) || (reg > 0xFFU)) {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_RD") " " I2C2_STR_PARAM CRLF);
		return;
	}

	if (len_arg != NULL) {
		if (!i2c2_parse_ulong(len_arg, &len) || (len == 0U) || (len > sizeof(data))) {
			i2c2_out(CONM STR_ERROR_CREATE("I2C2_RD") " " I2C2_STR_PARAM CRLF);
			return;
		}
	}

	res = I2C2_ReadReg8((uint8_t)addr, (uint8_t)reg, data, (uint16_t)len);
	if (res != I2C2_RES_OK) {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_RD") " 0x%02X 0x%02X %s" CRLF,
		         (unsigned int)addr, (unsigned int)reg, I2C2_ResultStr(res));
		return;
	}

	i2c2_out(CONM "I2C2_DATA 0x%02X 0x%02X:", (unsigned int)addr, (unsigned int)reg);
	for (i = 0; i < (uint16_t)len; i++) {
		i2c2_out(" %02X", (unsigned int)data[i]);
	}
	i2c2_out(CRLF);
	i2c2_out(CONM STR_OK_CREATE("I2C2_RD") CRLF);
}

static void i2c2_console_write(char *addr_arg, char *reg_arg, char *val_arg)
{
	unsigned long addr;
	unsigned long reg;
	unsigned long val;
	I2C2_Result res;

	if (!i2c2_parse_ulong(addr_arg, &addr) || (addr > 0x7FU) ||
	    !i2c2_parse_ulong(reg_arg, &reg) || (reg > 0xFFU) ||
	    !i2c2_parse_ulong(val_arg, &val) || (val > 0xFFU)) {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_WR") " " I2C2_STR_PARAM CRLF);
		return;
	}

	res = I2C2_WriteReg8Byte((uint8_t)addr, (uint8_t)reg, (uint8_t)val);
	if (res == I2C2_RES_OK) {
		i2c2_out(CONM STR_OK_CREATE("I2C2_WR") " 0x%02X 0x%02X=0x%02X" CRLF,
		         (unsigned int)addr, (unsigned int)reg, (unsigned int)val);
	}
	else {
		i2c2_out(CONM STR_ERROR_CREATE("I2C2_WR") " 0x%02X 0x%02X %s" CRLF,
		         (unsigned int)addr, (unsigned int)reg, I2C2_ResultStr(res));
	}
}

void I2C2_Console(char *msg)
{
	char *cursor;
	char *cmd;

	if (msg == NULL) {
		return;
	}
	if ((strlen(msg) < 4U) || (strncmp(msg, "I2C2", 4U) != 0)) {
		return;
	}

	i2c2_out_len = 0;

	cursor = msg + 4;
	cmd = i2c2_next_arg(&cursor);

	if (cmd == NULL) {
		i2c2_out_info();
		i2c2_out_help();
		i2c2_out_flush();
		return;
	}

	if (strcaseequals(cmd, "INFO") || strcaseequals(cmd, "STATUS")) {
		i2c2_out_info();
		i2c2_out_flush();
		return;
	}
	if (strcaseequals(cmd, "SCAN")) {
		i2c2_console_scan();
		i2c2_out_flush();
		return;
	}
	if (strcaseequals(cmd, "PING")) {
		i2c2_console_ping(i2c2_next_arg(&cursor));
		i2c2_out_flush();
		return;
	}
	if (strcaseequals(cmd, "RD")) {
		char *addr_arg = i2c2_next_arg(&cursor);
		char *reg_arg  = i2c2_next_arg(&cursor);
		char *len_arg  = i2c2_next_arg(&cursor);
		i2c2_console_read(addr_arg, reg_arg, len_arg);
		i2c2_out_flush();
		return;
	}
	if (strcaseequals(cmd, "WR")) {
		char *addr_arg = i2c2_next_arg(&cursor);
		char *reg_arg  = i2c2_next_arg(&cursor);
		char *val_arg  = i2c2_next_arg(&cursor);
		i2c2_console_write(addr_arg, reg_arg, val_arg);
		i2c2_out_flush();
		return;
	}
	if (strcaseequals(cmd, "RECOVER")) {
		I2C2_Result res = I2C2_BusRecover();
		if (res == I2C2_RES_OK) {
			i2c2_out(CONM STR_OK_CREATE("I2C2_RECOVER") CRLF);
		}
		else {
			i2c2_out(CONM STR_ERROR_CREATE("I2C2_RECOVER") " %s" CRLF,
			         I2C2_ResultStr(res));
		}
		i2c2_out_flush();
		return;
	}

	i2c2_out(CONM STR_ERROR_CREATE("I2C2") " unknown command: %s" CRLF, cmd);
	i2c2_out_help();
	i2c2_out_flush();
}
