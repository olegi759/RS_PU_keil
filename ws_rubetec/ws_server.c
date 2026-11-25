#include <stdio.h>
#include <string.h>
#include "wizchip_conf.h"
#include "socket.h"
#include "ws_crypto.h"
#include "ws_server.h"
#include "stm_util.h"
//#include "ulog.h"

#define WS_BUF_SIZE 4096
#define WS_TX_CHUNK 1024
#define WS_CTX_COUNT 2

// �������� WebSocket (���� �� �����)
typedef struct {
    uint8_t  msg_buf[WS_BUF_SIZE];
    uint32_t msg_len;

    uint8_t  ctrl_buf[125];
    uint8_t  ctrl_len;

    uint8_t  header_buf[14];
    uint8_t  header_cnt;
    uint8_t  header_needed;
    uint8_t  in_payload;
    uint8_t  opcode;
    uint8_t  fin;
    uint8_t  mask[4];
    uint8_t  mask_idx;
    uint64_t payload_len;
    uint64_t payload_recv;

    uint8_t  handshake_buf[512];
    uint16_t handshake_len;
    uint8_t  handshake_done;

    uint8_t  got_pong;
} ws_ctx_t;

static ws_ctx_t ws_ctx[WS_CTX_COUNT];


static int8_t ws_ctx_map[_VIRTUAL_SOCK_NUM_] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static uint16_t ws_port_map[_VIRTUAL_SOCK_NUM_] = {0};


static uint8_t ws_rx_buf[2048];
static uint8_t ws_tx_buf[2048];

static uint8_t ws_ctrl_rx_buf[256];   /* отдельный буфер для проверки ping/pong */
static bool    ws_in_parse = false;   /* true, пока работает ws_parse_frames() */

// �������� �������� �� ������ ������
static ws_ctx_t* ws_get_ctx(uint8_t sock) {
    if (sock >= _VIRTUAL_SOCK_NUM_) return NULL;
    int8_t idx = ws_ctx_map[sock];
    if (idx < 0 || idx >= WS_CTX_COUNT) return NULL;
    return &ws_ctx[idx];
}

static void ws_force_reinit(uint8_t sock_num) {
    ws_ctx_t *ctx = ws_get_ctx(sock_num);
    if (!ctx) return;

    uint8_t  ctx_idx = ws_ctx_map[sock_num];
    uint16_t port    = ws_port_map[sock_num];

    close(sock_num);              // RST + ��������� ? SOCK_CLOSED
    setSn_IR(sock_num, 0xFF);     // ����� ���� ������
    memset(ctx, 0, sizeof(ws_ctx_t));

    if (ctx_idx >= 0 && ctx_idx < WS_CTX_COUNT && port != 0) {
        ws_server_init(sock_num, port, ctx_idx);   // ����� ����� LISTEN
    }
}


void ws_server_init(uint8_t ws_sock, uint16_t port, uint8_t ctx_idx) {
    if (ctx_idx >= WS_CTX_COUNT){
#ifdef RUBETEC_LOG
        ulog_fmt("WS: interface number WS( %d)\r\n", ctx_idx);
#endif
        return;
    }
    if (socket(ws_sock, Sn_MR_TCP, port, 0) == ws_sock) {
        ws_ctx_map[ws_sock]  = ctx_idx;
        ws_port_map[ws_sock] = port;   
        memset(&ws_ctx[ctx_idx], 0, sizeof(ws_ctx_t));
        listen(ws_sock);
#ifdef RUBETEC_LOG
        ulog_fmt("WS: listening on port %d (sock %d, ctx %d)\r\n", port, ws_sock, ctx_idx);
#endif
    } else {
        #ifdef RUBETEC_LOG
        ulog_fmt("WS: use port %d (sock %d, ctx %d) - ERROR \r\n", port, ws_sock, ctx_idx);
#endif
    }
}

// --- ��������: � ���������� ������������ � Extended Length ---
void ws_send_data(uint8_t sock_num, uint8_t *data, uint32_t len, uint8_t opcode) {
    uint32_t sent = 0;

    if (len == 0) {
        uint8_t empty[2] = {0x80 | opcode, 0};
        send(sock_num, empty, 2);
        return;
    }

    while (sent < len) {
        uint32_t chunk = len - sent;
        uint8_t fin = 0;

        if (chunk > WS_TX_CHUNK) {
            chunk = WS_TX_CHUNK;
            fin = 0;
        } else {
            fin = 1;
        }

        ws_tx_buf[0] = (fin ? 0x80 : 0x00) | (sent == 0 ? opcode : 0x00);
        uint32_t header_len = 2;

        if (chunk <= 125) {
            ws_tx_buf[1] = (uint8_t)chunk;
        } else if (chunk <= 65535) {
            ws_tx_buf[1] = 126;
            ws_tx_buf[2] = (chunk >> 8) & 0xFF;
            ws_tx_buf[3] = chunk & 0xFF;
            header_len = 4;
        } else {
            ws_tx_buf[1] = 127;
            ws_tx_buf[2] = 0; ws_tx_buf[3] = 0; ws_tx_buf[4] = 0; ws_tx_buf[5] = 0;
            ws_tx_buf[6] = (chunk >> 24) & 0xFF;
            ws_tx_buf[7] = (chunk >> 16) & 0xFF;
            ws_tx_buf[8] = (chunk >> 8) & 0xFF;
            ws_tx_buf[9] = chunk & 0xFF;
            header_len = 10;
        }

        memcpy(&ws_tx_buf[header_len], data + sent, chunk);

        uint32_t timeout = 1000;
        while (getSn_TX_FSR(sock_num) < (header_len + chunk)) {
            if (--timeout == 0) {
#ifdef RUBETEC_LOG
                ulog("WS: TX timeout, connection might be lost\r\n");
#endif
                return;
            }
            HAL_Delay(1);
        }

        send(sock_num, ws_tx_buf, header_len + chunk);
        sent += chunk;
    }
}

// --- ��������: Pong ---
void ws_send_pong(uint8_t sock_num, uint8_t *payload, uint16_t len) {
    if (len > 125) len = 125;
    ws_tx_buf[0] = 0x8A;
    ws_tx_buf[1] = (uint8_t)len;
    if (len > 0 && payload != NULL) {
        memcpy(&ws_tx_buf[2], payload, len);
    }
    send(sock_num, ws_tx_buf, len + 2);
}

static void ws_parse_frames_impl(uint8_t sock_num, uint8_t *data, uint16_t len, ws_data_handler handler) {
    ws_ctx_t *ctx = ws_get_ctx(sock_num);
    if (!ctx) return;

    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = data[i];

        if (!ctx->in_payload) {
            ctx->header_buf[ctx->header_cnt++] = byte;

            if (ctx->header_cnt == 2) {
                ctx->fin = (ctx->header_buf[0] & 0x80) >> 7;
                ctx->opcode = ctx->header_buf[0] & 0x0F;
                uint8_t masked = (ctx->header_buf[1] & 0x80) >> 7;
                ctx->payload_len = ctx->header_buf[1] & 0x7F;

                if (ctx->payload_len == 126)      ctx->header_needed = 4;
                else if (ctx->payload_len == 127) ctx->header_needed = 10;
                else                              ctx->header_needed = masked ? 6 : 2;
            }

            if (ctx->header_cnt >= 4 && ctx->header_needed == 4) {
                ctx->payload_len = (ctx->header_buf[2] << 8) | ctx->header_buf[3];
                ctx->header_needed = (ctx->header_buf[1] & 0x80) ? 8 : 4;
            }

            if (ctx->header_cnt >= 10 && ctx->header_needed == 10) {
                ctx->payload_len = 0;
                for (int j = 0; j < 8; j++) ctx->payload_len = (ctx->payload_len << 8) | ctx->header_buf[2 + j];
                ctx->header_needed = (ctx->header_buf[1] & 0x80) ? 14 : 10;
            }

            if (ctx->header_cnt == ctx->header_needed) {
                if (ctx->header_buf[1] & 0x80) {
                    memcpy(ctx->mask, &ctx->header_buf[ctx->header_cnt - 4], 4);
                }
                ctx->mask_idx = 0;
                ctx->payload_recv = 0;

                if (ctx->payload_len == 0) {
                    ctx->in_payload = 0;
                    ctx->header_cnt = 0;

                    if (ctx->opcode == 0x9) {
                        ws_send_pong(sock_num, NULL, 0);
                    } else if (ctx->opcode == 0xA) {
                        ctx->got_pong = 1;
                    } else if (ctx->opcode == 0x8) {
#ifdef RUBETEC_LOG
                        ulog("WS: Close received\r\n");
#endif
                        uint8_t close_frame[2] = {0x88, 0x00};
                        send(sock_num, close_frame, 2);
                        disconnect(sock_num);
                        return;
                    }
                } else {
                    ctx->in_payload = 1;
                    if (ctx->opcode == 0x1 || ctx->opcode == 0x2) {
                        ctx->msg_len = 0;
                    }
                }
            }
        } else {
            if (ctx->header_buf[1] & 0x80) {
                byte ^= ctx->mask[ctx->mask_idx++ % 4];
            }

            if (ctx->opcode & 0x08) {
                if (ctx->ctrl_len < sizeof(ctx->ctrl_buf)) ctx->ctrl_buf[ctx->ctrl_len++] = byte;
            } else {
                if (ctx->msg_len < WS_BUF_SIZE) {
                    ctx->msg_buf[ctx->msg_len++] = byte;
                }
            }

            ctx->payload_recv++;

            if (ctx->payload_recv >= ctx->payload_len) {
                ctx->in_payload = 0;
                ctx->header_cnt = 0;

                if (ctx->opcode & 0x08) {
                    if (ctx->opcode == 0x9) {
                        ws_send_pong(sock_num, ctx->ctrl_buf, ctx->ctrl_len);
                    } else if (ctx->opcode == 0xA) {
                        ctx->got_pong = 1;
                    } else if (ctx->opcode == 0x8) {
#ifdef RUBETEC_LOG
                        ulog("WS: Close received\r\n");
#endif
                        uint8_t close_frame[2] = {0x88, 0x00};
                        send(sock_num, close_frame, 2);
                        disconnect(sock_num);
                        return;
                    }
                    ctx->ctrl_len = 0;
                } else {
                    if (ctx->fin) {
                        if (ctx->msg_len < WS_BUF_SIZE) {
                            ctx->msg_buf[ctx->msg_len] = '\0';
                        }
                        if (handler != NULL) {
                            handler(sock_num, ctx->msg_buf, ctx->msg_len);
                        }
                        ctx->msg_len = 0; // ����� ��� ���������� ���������
                    }
                }
            }
        }
    }
}


static void ws_parse_frames(uint8_t sock_num, uint8_t *data, uint16_t len, ws_data_handler handler)
{
    if (ws_in_parse) return;          /* реентерабельный вызов (из handler) игнорируем */
    ws_in_parse = true;
    ws_parse_frames_impl(sock_num, data, len, handler);
    ws_in_parse = false;
}

void ws_server_poll(uint8_t sock_num, ws_data_handler handler) {
    ws_ctx_t *ctx = ws_get_ctx(sock_num);
    if (!ctx) return;

    uint8_t sr = getSn_SR(sock_num);

    static uint8_t last_sr[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    if (sock_num < 8 && sr != last_sr[sock_num]) {
        ulog_fmt("WS: sock %d state 0x%02X\r\n", sock_num, sr);
        last_sr[sock_num] = sr;
    }
    switch (sr) {
        case SOCK_ESTABLISHED:
            if (getSn_IR(sock_num) & Sn_IR_CON) {
                setSn_IR(sock_num, Sn_IR_CON);
#ifdef RUBETEC_LOG
                ulog_fmt("WS: Client connected on sock %d\r\n", sock_num);
#endif
            }

            uint16_t rx_len = getSn_RX_RSR(sock_num);
            if (rx_len > 0) {
                int32_t ret = recv(sock_num, ws_rx_buf, rx_len);
                if (ret > 0) {
                    if (!ctx->handshake_done) {
                        if (ctx->handshake_len + ret < sizeof(ctx->handshake_buf)) {
                            memcpy(&ctx->handshake_buf[ctx->handshake_len], ws_rx_buf, ret);
                            ctx->handshake_len += ret;
                            ctx->handshake_buf[ctx->handshake_len] = '\0';

                            char *hdr_end = strstr((char*)ctx->handshake_buf, "\r\n\r\n");
                            if (hdr_end != NULL) {
                                char* key_ptr = strstr((char*)ctx->handshake_buf, "Sec-WebSocket-Key:");
                                if (key_ptr) {
                                    key_ptr += 18;
                                    while (*key_ptr == ' ') key_ptr++;
                                    char client_key[32] = {0};
                                    int i = 0;
                                    while (key_ptr[i] != '\r' && key_ptr[i] != '\0' && i < 31) {
                                        client_key[i] = key_ptr[i];
                                        i++;
                                    }
                                    if (i != 24) {
#ifdef RUBETEC_LOG
                                        ulog("WS: Invalid Sec-WebSocket-Key length!\r\n");
#endif
                                        sprintf((char*)ws_tx_buf, "HTTP/1.1 400 Bad Request\r\n\r\n");
                                        send(sock_num, ws_tx_buf, strlen((char*)ws_tx_buf));
                                        disconnect(sock_num);
                                        return;
                                    }

                                    char magic_key[80];
                                    sprintf(magic_key, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", client_key);
                                    uint8_t sha1_res[20];
                                    char accept_key[32];
                                    sha1_hash((uint8_t*)magic_key, strlen(magic_key), sha1_res);
                                    base64_encode(sha1_res, 20, accept_key);

                                    sprintf((char*)ws_tx_buf,
                                        "HTTP/1.1 101 Switching Protocols\r\n"
                                        "Upgrade: websocket\r\n"
                                        "Connection: Upgrade\r\n"
                                        "Sec-WebSocket-Accept: %s\r\n\r\n", accept_key);

                                    send(sock_num, ws_tx_buf, strlen((char*)ws_tx_buf));
#ifdef RUBETEC_LOG
                                    ulog_fmt("WS: Handshake complete on sock %d!\r\n", sock_num);
#endif
                                    ctx->handshake_done = 1;

                                    /* ������� ����� �� TCP-�������� ����� ���������� � ��� WS-����� */
                                    {
                                        uint32_t hdr_len  = (uint32_t)((hdr_end + 4) - (char*)ctx->handshake_buf);
                                        uint32_t rest_len = (uint32_t)ctx->handshake_len - hdr_len;

                                        if (rest_len > 0) {
                                            ws_parse_frames(sock_num, (uint8_t*)hdr_end + 4,
                                                            (uint16_t)rest_len, handler);
                                        }
                                    }  
                                     
                                } else {
                                    ctx->handshake_len = 0;
                                }
                            }
                        } else {
                            ctx->handshake_len = 0;
                        }
                    } else {
                        ws_parse_frames(sock_num, ws_rx_buf, ret, handler);
                    }
                }
            }
            break;

        case SOCK_CLOSE_WAIT:
            // ������ ������� RX-������, ����� �� ����������� TCP-�������
            {
                uint16_t rx_left = getSn_RX_RSR(sock_num);
                if (rx_left > 0) {
                    uint16_t to_read = rx_left > sizeof(ws_rx_buf) ? sizeof(ws_rx_buf) : rx_left;
                    recv(sock_num, ws_rx_buf, to_read);
                }
            }
            disconnect(sock_num);
            setSn_IR(sock_num, 0xFF); // ����� ���� ������ ����������
            memset(ctx, 0, sizeof(ws_ctx_t));
            break;

        // ������������ ��������� ������������� ��������� ��������
        case SOCK_FIN_WAIT:    // 0x18
        case SOCK_CLOSING:     // 0x1A
        case SOCK_TIME_WAIT:   // 0x1B
        case SOCK_LAST_ACK:    // 0x1D
            ws_force_reinit(sock_num);
            break;

        case SOCK_CLOSED: {
            uint8_t  ctx_idx = ws_ctx_map[sock_num];
            uint16_t port    = ws_port_map[sock_num];
            if (ctx_idx >= 0 && ctx_idx < WS_CTX_COUNT && port != 0) {
                ws_server_init(sock_num, port, ctx_idx);
            }
            break;
        }
        
        default:
            break;
    }
}

// --- PING/PONG (на уровне самого протокола веб сокета)---
bool ws_try_ping_pong(uint8_t sock_num, ws_data_handler handler, uint32_t timeout)
{
    ws_ctx_t *ctx = ws_get_ctx(sock_num);
    if (!ctx || !ctx->handshake_done) return false;
    if (ws_in_parse) return false;                                  /* нельзя из handler */
    if (getSn_SR(sock_num) != SOCK_ESTABLISHED) return false;

    ctx->got_pong = 0;
    ws_send_data(sock_num, NULL, 0, 0x9);                           /* Ping */

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout) {
        uint16_t rx = getSn_RX_RSR(sock_num);
        if (rx > 0) {
            if (rx > sizeof(ws_ctrl_rx_buf)) rx = sizeof(ws_ctrl_rx_buf);
            int32_t ret = recv(sock_num, ws_ctrl_rx_buf, rx);
            if (ret > 0) {
                ws_parse_frames(sock_num, ws_ctrl_rx_buf, (uint16_t)ret, handler);
            }
        }
        if (ctx->got_pong) return true;
        if (getSn_SR(sock_num) != SOCK_ESTABLISHED) return false;
        HAL_Delay(1);
    }
    return false;
}

void ws_server_close(uint8_t sock_num)
{
    ws_force_reinit(sock_num);
}