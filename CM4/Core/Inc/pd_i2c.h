/*
 * pd_i2c.h
 *
 *  Передача кадров PD-протокола по I2C2 (STM32H747, ядро Cortex-M4).
 *
 *  Раньше PD-протокол ходил по UART; на этой плате он вынесен на I2C2:
 *  PU (CM4) - master, Plaта ModulInterface (STM32F103) - slave с адресом 0x21.
 *
 *  Кадр PD-протокола передаётся как есть (без изменений):
 *    [dest][src][len_hi][len_lo][cmd][data ...][crc_hi][crc_lo],
 *  где len = 1 + число байт данных, CRC16 по всем байтам, кроме двух CRC.
 *  Ответ slave читается фиксированным блоком PD_I2C_READ_SIZE: длина кадра
 *  определяется по полю len, лишние байты игнорируются.
 */

#ifndef PD_I2C_H
#define PD_I2C_H

#include "main.h"
#include "common.h"

/* Адрес платы ModulInterface на шине I2C1 (7-битный) */
#define PD_I2C_SLAVE_ADDR       0x21U

/* Фиксированный размер чтения ответа (кадр самоописывающийся) */
#define PD_I2C_READ_SIZE        128U

/* Команда опроса, которую понимает PD-протокол ModulInterface
   (в PD_protocol она называется PD_CMD_PING) */
#define PD_I2C_CMD_POLL         0x55U

/* Адреса по умолчанию: ПД - 0x01, ПУ - 0x0A */
#define PD_I2C_DEF_PD_ADDR      0x01U
#define PD_I2C_DEF_PU_ADDR      0x0AU

/* Отправка кадра и приём ответа. Возвращает TRUE, если получен корректный
   кадр (совпала CRC). rx_len - длина принятого кадра. */
BOOL PD_I2C_SendReceive(const uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t *rx_len);

/* Сформировать и отправить кадр опроса (cmd = PD_I2C_CMD_POLL) */
BOOL PD_I2C_Poll(uint8_t dest, uint8_t src, uint8_t *rx, uint16_t *rx_len);

/* Обработчик консольной команды "PDI2C ..." (PING/SEND/STRESS/INFO) */
void PD_I2C_Console(char *msg);

#endif /* PD_I2C_H */
