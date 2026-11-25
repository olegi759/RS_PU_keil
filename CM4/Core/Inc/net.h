/*
 * net.h
 *
 *  Created on: 17 янв. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_NET_H_
#define CORE_INC_NET_H_

#include "main.h"
#include "wizchip_conf.h"

void net_init(void);
void net_task(void);
void net_int_change(void);

extern uint32_t http_session_tick;

#endif /* CORE_INC_NET_H_ */
