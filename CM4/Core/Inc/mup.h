/*
 * mup.h
 *
 *  Created on: 12 мар. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MUP_H_
#define CORE_INC_MUP_H_

#include "main.h"
#include "mup_data.h"


void mup_send_query(void);
void mup_rcv(uint16_t * data, uint16_t data_len);


extern const uint16_t mup_query[12];

extern uint8_t mup_errrors;




#endif /* CORE_INC_MUP_H_ */
