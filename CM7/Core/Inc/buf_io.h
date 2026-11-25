/*
 * buf_io.h
 *
 *  Created on: 23 июн. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef CM7_CORE_INC_BUF_IO_H_
#define CM7_CORE_INC_BUF_IO_H_


#include "main.h"


#ifdef USE_RINGBUF


#include "ringbuff_util.h"

#define BUFIO_PUT_PACKET(abuf, data, data_len)	ringbuff_write_packet(abuf, data, data_len)
#define BUFIO_PUT(abuf, data, data_len)			ringbuff_write(abuf, data, data_len)
#define BUFIO_POP_PACKET(abuf, buf, buf_size)	ringbuff_read_packet(abuf,  buf, buf_size)
#define BUFIO_POP(abuf, buf, buf_size)			ringbuff_read(abuf,  buf, buf_size)
#define BUFIO_IS_EMPTY(abuf)					(ringbuff_get_full(abuf) == 0)
#define BUFIO_IS_BUSY(abuf)						ringbuff_get_busy(abuf)
#define BUFIO_POP_LINE(abuf, buf, buf_size)		ringbuff_read(abuf, buf, buf_size)
#define BUFIO_CLEAR(abuf)						ringbuff_reset(abuf)
#define BUFIO_FREE_SIZE(abuf)					ringbuff_get_free(abuf)
#define BUFIO_FULL_SIZE(abuf)					ringbuff_get_full(abuf)
#define BUFIO_SET_BUSY(abuf, val)				ringbuff_set_busy(abuf, val)
#define BUFIO_GET_MAX_SIZE(abuf)				ringbuff_get_max_size(abuf)
#define BUFIO_SET_MAX_SIZE(abuf, buf, size)		ringbuff_init(abuf, buf, size)

#endif


#ifdef USE_DYNBUF


#include "dynbuf_util.h"

#define BUFIO_PUT_PACKET(abuf, data, data_len)	dynamic_buf_push_io(abuf, data, data_len)
#define BUFIO_PUT(abuf, data, data_len)			dynamic_buf_push_io(abuf, data, data_len)
#define BUFIO_POP_PACKET(abuf, buf, buf_size)	dynamic_buf_pop_io(abuf,  buf, buf_size)
#define BUFIO_POP(abuf, buf, buf_size)			dynamic_buf_pop_line_io(abuf, buf, buf_size)
#define BUFIO_IS_EMPTY(abuf)					dynamic_buf_is_empty(abuf)
#define BUFIO_IS_BUSY(abuf)						dynamic_buf_get_busy(abuf)
#define BUFIO_CLEAR(abuf)						dynamic_buf_clear(abuf);
#define BUFIO_FREE_SIZE(abuf)					dynamic_buf_get_free_size(abuf)
#define BUFIO_FULL_SIZE(abuf)					dynamic_buf_get_data_size(abuf)
#define BUFIO_SET_BUSY(abuf, val)				dynamic_buf_set_busy(abuf, val)
#define BUFIO_GET_MAX_SIZE(abuf)				dynamic_buf_get_max_alloc_size(abuf)
#define BUFIO_SET_MAX_SIZE(abuf, buf, size)		dynamic_buf_set_max_alloc_size(abuf, size)

#endif


#define	BUFIO_FULL_PERCENT(buf)			((BUFIO_FULL_SIZE(buf) * 100) / BUFIO_GET_MAX_SIZE(buf))

#endif /* CM7_CORE_INC_BUF_IO_H_ */
