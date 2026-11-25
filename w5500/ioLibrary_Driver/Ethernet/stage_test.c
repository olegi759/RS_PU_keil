#include "stage_test.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "virtual_socket_map.h"
#include "stm_util.h"          /* ulog_fmt, если он у вас там */
#include <string.h>

/* Компиляторные проверки макросов — падают на этапе сборки, если что-то не так */
_Static_assert(_VIRTUAL_SOCK_NUM_ == 16, "expected 16 virtual sockets");
_Static_assert(SN2CHIP(0)  == 0, "SN2CHIP(0)");
_Static_assert(SN2CHIP(7)  == 0, "SN2CHIP(7)");
_Static_assert(SN2CHIP(8)  == 1, "SN2CHIP(8)");
_Static_assert(SN2CHIP(15) == 1, "SN2CHIP(15)");
_Static_assert(SN2LOCAL(0)  == 0, "SN2LOCAL(0)");
_Static_assert(SN2LOCAL(7)  == 7, "SN2LOCAL(7)");
_Static_assert(SN2LOCAL(8)  == 0, "SN2LOCAL(8)");
_Static_assert(SN2LOCAL(15) == 7, "SN2LOCAL(15)");

int stage3_test(void)
{
    uint8_t ver0, ver1, ver0b;
    uint8_t ip0[4], ip1[4], ip0b[4];

    /* 1) Переключились на чип 0, прочитали VERSIONR */
    VMAP_SELECT_CHIP(0);
    ver0 = getVERSIONR(0);
    WIZCHIP_READ_BUF(SIPR, ip0, 4, 0);

    /* 2) Переключились на чип 1 (через виртуальный sn = 8), прочитали */
    VMAP_SELECT_CHIP(8);
    ver1 = getVERSIONR(1);
    WIZCHIP_READ_BUF(SIPR, ip1, 4, 1);

    /* 3) Вернулись на чип 0 — состояние не потерялось */
    VMAP_SELECT_CHIP(0);
    ver0b = getVERSIONR(0);
    WIZCHIP_READ_BUF(SIPR, ip0b, 4, 0);

    ulog_fmt("stage3: ver0=0x%02X ver1=0x%02X ver0b=0x%02X"CRLF,
             ver0, ver1, ver0b);
    ulog_fmt("stage3: ip0=%d.%d.%d.%d ip1=%d.%d.%d.%d"CRLF,
             ip0[0], ip0[1], ip0[2], ip0[3],
             ip1[0], ip1[1], ip1[2], ip1[3]);

    /* W5500 всегда возвращает 0x04 в VERSIONR */
    if (ver0 != 0x04) return 1;
    if (ver1 != 0x04) return 2;
    if (ver0b != 0x04) return 3;

    /* IP разные и не нулевые */
    if (memcmp(ip0, ip1, 4) == 0) return 4;
    if (ip0[0] == 0 || ip1[0] == 0) return 5;

    /* Возврат на chip 0 дал тот же IP */
    if (memcmp(ip0, ip0b, 4) != 0) return 6;

    /* 4) Все 16 виртуальных номеров переключаются без зависания SPI */
    for (uint8_t sn = 0; sn < _VIRTUAL_SOCK_NUM_; sn++) {
        VMAP_SELECT_CHIP(sn);
        if (getVERSIONR(SN2CHIP(sn)) != 0x04) return (int)(10 + sn);
    }

    /* 5) Выход за диапазон не переключает чип (защита в VMAP_SELECT_CHIP) */
    VMAP_SELECT_CHIP(0);          /* явно выбрали 0 */
    VMAP_SELECT_CHIP(16);         /* выход за диапазон — no-op */
    if (getVERSIONR(0) != 0x04) return 40;   /* всё ещё читаем chip 0 */

    return 0;
}

int stage4_test_basic(void)
{
    uint8_t st;

    /* --- 1) Открываем TCP-сервер на chip 0, локальный сокет 0 --- */
    if (socket(0, Sn_MR_TCP, 5000, 0) != 0)        return 1;
    if (listen(0) != SOCK_OK)                       return 2;
    if (getsockopt(0, SO_STATUS, &st) != SOCK_OK)   return 3;
    if (st != SOCK_LISTEN)                          return 4;

    /* --- 2) Открываем TCP-сервер на chip 1, локальный сокет 0 --- */
    /*     виртуальный номер = SN_MAKE(1,0) = 8                          */
    if (socket(8, Sn_MR_TCP, 5001, 0) != 8)         return 5;
    if (listen(8) != SOCK_OK)                       return 6;
    if (getsockopt(8, SO_STATUS, &st) != SOCK_OK)   return 7;
    if (st != SOCK_LISTEN)                          return 8;

    /* --- 3) Статусы независимы: chip 0 всё ещё слушает --- */
    if (getsockopt(0, SO_STATUS, &st) != SOCK_OK)   return 9;
    if (st != SOCK_LISTEN)                          return 10;

    /* --- 4) Закрываем сокет на chip 1, chip 0 не должен пострадать --- */
    if (close(8) != SOCK_OK)                        return 11;
    if (getsockopt(0, SO_STATUS, &st) != SOCK_OK)   return 12;
    if (st != SOCK_LISTEN)                          return 13;

    /* --- 5) Открываем сокет на chip 1 с локальным номером 5 --- */
    /*     виртуальный = SN_MAKE(1,5) = 13                              */
    if (socket(13, Sn_MR_TCP, 5002, 0) != 13)       return 14;
    if (listen(13) != SOCK_OK)                      return 15;
    if (getsockopt(13, SO_STATUS, &st) != SOCK_OK)  return 16;
    if (st != SOCK_LISTEN)                          return 17;

    /* --- 6) Два сокета на chip 1 независимы друг от друга --- */
    /*     (8 уже закрыт, 13 слушает)                                   */
    if (getsockopt(8, SO_STATUS, &st) != SOCK_OK)   return 18;
    if (st != SOCK_CLOSED)                          return 19;   /* 8 закрыт */
    if (getsockopt(13, SO_STATUS, &st) != SOCK_OK)  return 20;
    if (st != SOCK_LISTEN)                          return 21;   /* 13 слушает */

    /* --- 7) Закрываем всё --- */
    if (close(13) != SOCK_OK)                       return 22;
    if (close(0)  != SOCK_OK)                       return 23;

    /* --- 8) Проверка границ: sn = 16 должен вернуть SOCKERR_SOCKNUM --- */
    if (socket(16, Sn_MR_TCP, 5003, 0) != SOCKERR_SOCKNUM) return 24;
    if (close(16) != SOCKERR_SOCKNUM)                       return 25;
    if (getsockopt(16, SO_STATUS, &st) != SOCKERR_SOCKNUM)  return 26;

    return 0;
}

int stage4_test_isolation(void)
{
    /*
     * Сценарий:
     *   sn=8  (chip 1, local 0)  — TCP-сервер, слушает 6000
     *   sn=0  (chip 0, local 0)  — TCP-клиент, connect на IP chip 1:6000
     *   Клиент отправляет "PING", сервер отвечает "PONG".
     *
     * Требует, чтобы оба чипа были в одной подсети и видели друг друга.
     */
    uint8_t  ip1[4];
    uint8_t  st;
    uint8_t  buf[8];
    int32_t  n;

    /* Узнаём IP второго чипа */
    VMAP_SELECT_CHIP(8);
    WIZCHIP_READ_BUF(SIPR, ip1, 4, 1);
    ulog_fmt("stage4: server ip = %d.%d.%d.%d"CRLF,
             ip1[0], ip1[1], ip1[2], ip1[3]);

    /* --- 1) Сервер на chip 1 --- */
    if (socket(8, Sn_MR_TCP, 6000, 0) != 8)         return 1;
    if (listen(8) != SOCK_OK)                        return 2;

    /* --- 2) Клиент на chip 0 --- */
    if (socket(0, Sn_MR_TCP, 6001, 0) != 0)         return 3;
    if (connect(0, ip1, 6000) != SOCK_OK)            return 4;

    /* --- 3) Сервер должен увидеть ESTABLISHED (не мгновенно, polling) --- */
    for (int i = 0; i < 1000; i++) {
        getsockopt(8, SO_STATUS, &st);
        if (st == SOCK_ESTABLISHED) break;
    }
    if (st != SOCK_ESTABLISHED)                      return 5;

    /* --- 4) Клиент отправляет "PING" --- */
    memcpy(buf, "PING", 4);
    n = send(0, buf, 4);
    if (n != 4)                                      return 6;

    /* --- 5) Сервер принимает --- */
    for (int i = 0; i < 1000; i++) {
        n = recv(8, buf, 4);
        if (n == 4) break;
    }
    if (n != 4)                                      return 7;
    if (memcmp(buf, "PING", 4) != 0)                 return 8;

    /* --- 6) Сервер отвечает "PONG" --- */
    memcpy(buf, "PONG", 4);
    n = send(8, buf, 4);
    if (n != 4)                                      return 9;

    /* --- 7) Клиент принимает --- */
    for (int i = 0; i < 1000; i++) {
        n = recv(0, buf, 4);
        if (n == 4) break;
    }
    if (n != 4)                                      return 10;
    if (memcmp(buf, "PONG", 4) != 0)                 return 11;

    /* --- 8) Закрываем оба --- */
    disconnect(0);
    close(0);
    disconnect(8);
    close(8);

    return 0;
}

#define TCP_ECHO_BUF_LEN 512

static uint8_t  tcp_buf[TCP_ECHO_BUF_LEN];
// static uint16_t tcp_port = TCP_ECHO_PORT;
// static uint8_t  tcp_sock = TCP_ECHO_SOCK;

/* Открыть сокет в режиме TCP-сервера и перейти в LISTEN */
void tcp_echo_open_stage(uint8_t vms, uint16_t port)
{
    if (socket(vms, Sn_MR_TCP, port, 0) != vms) {
        ulog("tcp: socket() failed\r\n");
        return;
    }
    if (listen(vms) != SOCK_OK) {
        ulog("tcp: listen() failed\r\n");
        return;
    }
    ulog_fmt("tcp: listening on port %u(vms %u)\r\n", port, vms);
}


/* Периодически вызывать из main loop */
void tcp_echo_poll_stage(uint8_t vms)
{
    uint8_t sr = getSn_SR(vms);

    switch (sr) {
    case SOCK_ESTABLISHED:
        /* Обработка флага нового подключения */
        if (getSn_IR(vms) & Sn_IR_CON) {
            setSn_IR(vms, Sn_IR_CON);
            ulog("tcp: client connected\r\n");
        }

        /* Есть ли данные в RX-буфере сокета? */
        uint16_t rx_len = getSn_RX_RSR(vms);
        if (rx_len > 0) {
            ulog_fmt("tcp: amaunt of data = %u\r\n", rx_len);
            if (rx_len > TCP_ECHO_BUF_LEN) {
                rx_len = TCP_ECHO_BUF_LEN;   /* не переполняем локальный буфер */
            }
            int32_t got = recv(vms, tcp_buf, rx_len);
            if (got > 0) {
                /* Эхо: отправляем обратно */
                int32_t sent = send(vms, tcp_buf, (uint16_t)got);
                if (sent < 0) {
                    ulog_fmt("tcp: send() error %ld\r\n", (long)sent);
                }
            } else if (got < 0) {
                ulog_fmt("tcp: recv() error %ld\r\n", (long)got);
            }
        }
        break;

    case SOCK_CLOSE_WAIT:
        ulog("tcp: client disconnecting...\r\n");
        disconnect(vms);
        break;

    case SOCK_CLOSED:
        ulog("tcp: socket closed, reopening\r\n");
        tcp_echo_open_stage(vms, get_port_by_sock(vms));
        break;

    case SOCK_INIT:
        /* уже слушаем, ничего не делаем */
        break;

    case SOCK_LISTEN:
        /* ждём подключения */
        break;

    default:
        /* SOCK_SYNRECV, SOCK_SYNSENT, SOCK_FIN_WAIT и т.п. — ждём */
        break;
    }
}