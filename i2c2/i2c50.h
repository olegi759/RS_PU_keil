/*
 * i2c50.h
 *
 *  Обмен с платой ModulInterface (STM32F103) по I2C2 пакетами по 50 байт
 *  с контролем CRC16 (STM32H747, мастер - ядро Cortex-M7).
 *
 *  Выводы I2C2: PF0 - SDA, PF1 - SCL (модуль i2c2). Ведомый - ModulInterface,
 *  I2C1, 7-битный адрес 0x42, буфер 50 байт.
 *
 *  Формат пакета (одинаков для запроса и ответа):
 *
 *    смещение  размер  поле
 *    0         1       magic    = 0xA5
 *    1         1       version  = 0x01
 *    2         1       type     тип запроса / тип ответа (запрос | 0x80)
 *    3         1       seq      номер запроса, в ответе повторяется
 *    4         1       len      длина полезных данных 0..43
 *    5         43      payload  полезные данные
 *    48        2       crc      CRC16 (старший байт, затем младший), байты 0..47
 *
 *  CRC16 - CCITT-FALSE (0x1021, начальное 0xFFFF): Crc16() из math_util.c,
 *  на стороне ModulInterface - CalcCRC16() (та же таблица).
 *
 *  Обмен: мастер пишет запрос (50 байт), затем читает ответ (50 байт).
 *  Пока слейв формирует ответ, он растягивает SCL на адресе чтения, поэтому
 *  ответ всегда готов к моменту чтения.
 *
 *  Периодический опрос (I2C50_Task) выполняется из главного цикла CM7:
 *  раз в секунду PING, каждый 5-й опрос - ECHO 43 байт, каждый 10-й -
 *  WRITE + READ 43 байт (проверка целостности данных в обе стороны).
 */

#ifndef I2C50_H
#define I2C50_H

#include "main.h"
#include "i2c2.h"

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Формат пакета
 * ------------------------------------------------------------------------ */

#define I2C50_PACKET_SIZE       50U     /* пакет целиком, байт                */
#define I2C50_MAX_PAYLOAD       43U     /* 50 - 5 (заголовок) - 2 (CRC)       */

#define I2C50_MAGIC             0xA5U
#define I2C50_VERSION           0x01U

#define I2C50_OFS_MAGIC         0U
#define I2C50_OFS_VERSION       1U
#define I2C50_OFS_TYPE          2U
#define I2C50_OFS_SEQ           3U
#define I2C50_OFS_LEN           4U
#define I2C50_OFS_PAYLOAD       5U
#define I2C50_OFS_CRC           48U

/* Типы запросов */
#define I2C50_TYPE_PING         0x01U
#define I2C50_TYPE_ECHO         0x02U
#define I2C50_TYPE_WRITE        0x03U
#define I2C50_TYPE_READ         0x04U

/* Тип ответа = тип запроса | 0x80; 0x80 - ответ с ошибкой */
#define I2C50_TYPE_REPLY        0x80U
#define I2C50_TYPE_ERROR        0x80U

/* Коды ошибок в payload[0] ответа 0x80 (совпадают с прошивкой F103) */
#define I2C50_ERR_NONE          0U
#define I2C50_ERR_CRC           1U
#define I2C50_ERR_MAGIC         2U
#define I2C50_ERR_VERSION       3U
#define I2C50_ERR_LEN           4U
#define I2C50_ERR_TYPE          5U
#define I2C50_ERR_SIZE          6U

/* ---------------------------------------------------------------------------
 * Настройки обмена
 * ------------------------------------------------------------------------ */

/* 7-битный адрес платы ModulInterface на шине I2C2 */
#define I2C50_SLAVE_ADDR        0x42U

/* Периодический опрос из главного цикла, мс */
#define I2C50_POLL_PERIOD_MS    1000U

/* Сколько раз повторять обмен, прежде чем сдаться */
#define I2C50_ATTEMPTS          3U

/* Восстановление шины - не чаще, чем раз в 200 мс */
#define I2C50_RECOVER_PERIOD_MS 200U

/* Сколько подряд неудачных обменов считается поводом освободить шину */
#define I2C50_FAIL_STREAK_LIMIT 3U

/* Пауза между попытками, мс */
#define I2C50_RETRY_DELAY_MS    1U

/* ---------------------------------------------------------------------------
 * Диагностика (видно в отладчике)
 * ------------------------------------------------------------------------ */

extern volatile uint32_t i2c50_tx_count;      /* отправлено запросов          */
extern volatile uint32_t i2c50_rx_ok;         /* корректных ответов           */
extern volatile uint32_t i2c50_err_crc;       /* ответов с неверной CRC       */
extern volatile uint32_t i2c50_err_hdr;       /* неверный magic/version/len   */
extern volatile uint32_t i2c50_err_seq;       /* не совпал номер запроса      */
extern volatile uint32_t i2c50_err_type;      /* не тот тип ответа/ошибка слейва */
extern volatile uint32_t i2c50_nack;          /* слейв не ответил (NACK)      */
extern volatile uint32_t i2c50_recover_count; /* восстановлений шины          */
extern volatile uint32_t i2c50_ping_ok;       /* удачных PING                 */
extern volatile uint32_t i2c50_echo_ok;       /* удачных ECHO                 */
extern volatile uint32_t i2c50_write_ok;      /* удачных WRITE                */
extern volatile uint32_t i2c50_read_ok;       /* удачных READ                 */

extern volatile I2C2_Result i2c50_last_res;   /* результат последней операции */
extern volatile uint8_t  i2c50_last_type;
extern volatile uint8_t  i2c50_last_seq;
extern volatile uint8_t  i2c50_last_len;
extern volatile uint8_t  i2c50_last_err;      /* код ошибки от слейва         */
extern volatile uint8_t  i2c50_slave_rx;      /* счётчик слейва из PING       */
extern volatile uint8_t  i2c50_slave_crc;     /* ошибки CRC слейва из PING    */

extern uint8_t i2c50_last_req[I2C50_PACKET_SIZE];
extern uint8_t i2c50_last_resp[I2C50_PACKET_SIZE];

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

/* Инициализация I2C2 и модуля. Возвращает TRUE при успехе. */
BOOL I2C50_Init(void);

/* Обмен: запрос type с данными payload (plen <= I2C50_MAX_PAYLOAD) и приём
   ответа. reply/rlen - буфер и длина полезных данных ответа (могут быть NULL,
   если данные не нужны). Возвращает TRUE, если ответ корректен и CRC сошлась. */
BOOL I2C50_Exchange(uint8_t type, const uint8_t *payload, uint8_t plen,
                    uint8_t *reply, uint8_t *rlen);

/* То же для PING (данные не нужны) */
BOOL I2C50_Ping(void);

/* Периодический опрос: вызывать из главного цикла CM7. Внутри - не чаще раза
   в I2C50_POLL_PERIOD_MS, не блокирует цикл дольше одного обмена. */
void I2C50_Task(void);

#endif /* I2C50_H */
