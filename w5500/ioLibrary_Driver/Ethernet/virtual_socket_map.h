//virtual_socket_map.h

#ifndef VIRTUAL_SOCKET_MAP_H
#define VIRTUAL_SOCKET_MAP_H

#include "wizchip_conf.h"
#include "w5500.h"

/*
 * Виртуальные сокеты.
 *
 * SN2CHIP(), SN2LOCAL(), _VIRTUAL_SOCK_NUM_(), _LS() и макросы доступа
 * к регистрам сокетов getSn_xxx()/setSn_xxx() теперь определены прямо в w5500.h
 * (они сразу выбирают нужный физический чип по виртуальному номеру сокета),
 * поэтому #undef-переопределения в этом файле больше не нужны.
 *
 * Доступ к общим регистрам (getMR/setGAR/getVERSIONR/...) требует явного
 * номера чипа csw — он передаётся последним параметром, см. w5500.h.
 */

/* Запасные определения на случай сборки не под W5500: w5500.h тогда их не даёт. */
#ifndef _VIRTUAL_SOCK_NUM_
#define _VIRTUAL_SOCK_NUM_   ((uint8_t)((_WIZCHIP_CHIP_NUM_) * (_WIZCHIP_SOCK_NUM_)))

/* virt sn -> chip index */
#define SN2CHIP(sn)          ((uint8_t)((uint8_t)(sn) / _WIZCHIP_SOCK_NUM_))

/* virt sn -> local socket number */
#define SN2LOCAL(sn)         ((uint8_t)((uint8_t)(sn) % _WIZCHIP_SOCK_NUM_))
#endif

/*
//virt sn -> {chip, local_sn} — если очень нужна структура
#define VMAP_GET(sn, m)      do {                     \
    (m).chip     = SN2CHIP(sn);                       \
    (m).local_sn = SN2LOCAL(sn);                      \
} while (0)
*/

/* select physical chip by virt sn (side effect) */
#define VMAP_SELECT_CHIP(sn)      do {                     \
    if ((uint8_t)(sn) < _VIRTUAL_SOCK_NUM_)           \
        wizchip_select(SN2CHIP(sn));                  \
} while (0)

#endif
