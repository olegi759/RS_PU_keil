/*
 * w25qx_io.c
 *
 *  Created on: 24 апр. 2024 г.
 *      Author: Rafael Boltachev
 */


#include "flash_io.h"
#include "mx25lm51245g_conf.h"
#include "mx25lm51245g.h"
#include "diskio.h"
#include "stm_util.h"


BOOL flash_init(void)
{
	return TRUE;
}



BOOL flash_read(uint8_t *buf, uint32_t sector, uint32_t count)
{
	uint32_t ReadAddr, Size;
	uint8_t mx25_res;

	ReadAddr = sector * MX25L512_SUBSECTOR_SIZE;
	Size = count * MX25L512_SUBSECTOR_SIZE;
	//fulog_fmt("disk_read(%lu[%lu] %lu)"CRLF, sector, ReadAddr, count);
	mx25_res = mx25_QSPI_Read(&hqspi, buf, ReadAddr, Size);
	//ulog_fmt("disk_read(%lu[%lu] %lu)=%d"CRLF, sector, ReadAddr, count, mx25_res);
    return mx25_res == RES_OK ? TRUE : FALSE;
}


BOOL flash_write(uint8_t *buf,	uint32_t sector, uint32_t count)
{
	uint32_t WriteAddr, Size;
	uint8_t mx25_res;
	UINT i;

	for (i = 0; i < count; ++i) {
		WriteAddr = sector * MX25L512_SUBSECTOR_SIZE;
		mx25_res = mx25_QSPI_Erase_SubSector(&hqspi, WriteAddr);
	}

	WriteAddr = sector * MX25L512_SUBSECTOR_SIZE;
	Size = count * MX25L512_SUBSECTOR_SIZE;
	//ulog_fmt("disk_write(%lu[%lu] %lu)"CRLF, sector, WriteAddr, count);
	mx25_res = mx25_QSPI_Write(&hqspi, buf, WriteAddr, Size);
	//ulog_fmt("disk_write(%lu[%lu] %lu)=%d"CRLF, sector, WriteAddr, count, mx25_res);
    return mx25_res == RES_OK ? TRUE: FALSE;
}

uint32_t flash_sector_size(void)
{
	return MX25L512_SUBSECTOR_SIZE;
}

uint32_t flash_sector_count(void)
{
	return MX25L512_FLASH_SIZE / MX25L512_SUBSECTOR_SIZE;
}

uint32_t flash_block_size(void)
{
	return MX25L512_SECTOR_SIZE;
}


uint64_t flash_full_size(void)
{
	return MX25L512_FLASH_SIZE;
}

uint32_t flash_block_count(void)
{
	return MX25L512_FLASH_SIZE / MX25L512_SECTOR_SIZE;
}

uint16_t flash_device_id(void)
{
	uint8_t identification_data[128];

	mx25_QSPI_GetIdentification(&hqspi, identification_data);
	return *(uint16_t *)identification_data;
}


