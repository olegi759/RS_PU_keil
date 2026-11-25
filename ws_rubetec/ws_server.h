#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Обработчик прикладных WebSocket-сообщений.
 *
 * Парсер вызывает handler СИНХРОННО, когда собрано полное сообщение (FIN=1).
 * Указатель data валиден ТОЛЬКО во время вызова handler.
 * Если handler хочет использовать данные после возврата (асинхронная обработка),
 * он ОБЯЗАН скопировать их в свой буфер.
 * Буфер парсера будет перезаписан при приёме следующего сообщения.
 *
 * sock_num — номер сокета W5500, по которому пришло сообщение (нужен,
 *            чтобы handler мог отправить ответ через ws_send_data).
 */
typedef void (*ws_data_handler)(uint8_t sock_num, uint8_t *data, uint32_t len);

void ws_server_init(uint8_t ws_sock, uint16_t port, uint8_t ctx_idx);
void ws_server_poll(uint8_t sock_num, ws_data_handler handler);
void ws_send_data(uint8_t sock_num, uint8_t *data, uint32_t len, uint8_t opcode);
void ws_send_pong(uint8_t sock_num, uint8_t *payload, uint16_t len);
bool ws_try_ping_pong(uint8_t sock_num, ws_data_handler handler, uint32_t timeout);
void ws_server_close(uint8_t sock_num);

#endif /* WS_SERVER_H */