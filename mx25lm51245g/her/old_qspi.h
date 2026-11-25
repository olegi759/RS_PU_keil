/*
 * old_qspi.h
 *
 *  Created on: 21 нояб. 2023 г.
 *      Author: Rafael Boltachev
 */

#ifndef MX25LM51245G_H
#define MX25LM51245G_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

uint8_t mx25_QSPI_Init(void);
void mx25_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle);
uint8_t mx25_QSPI_WriteEnable(void);
uint8_t mx25_QSPI_EraseSector(uint32_t EraseStartAddress,uint32_t EraseEndAddress);
uint8_t mx25_QSPI_AutoPollingMemReady(void);
uint8_t mx25_QSPI_Configuration(void);
uint8_t mx25_QSPI_ReadMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size);
uint8_t mx25_QSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size);
uint8_t mx25_QSPI_EnableMemoryMappedMode(void);
uint8_t mx25_QSPI_ResetChip(void);

//memory page and sector size -verify inside datasheet
#define MEMORY_SECTOR_SIZE 0x1000
#define MEMORY_PAGE_SIZE 0x100

#define QSPI_DUMMY_CYCLES	6

#define WRITE_ENABLE_CMD 0x06
#define READ_STATUS_REG_CMD 0x05
#define WRITE_STATUS_REG_CMD 0x01
#define SECTOR_ERASE_CMD 0x20
#define QUAD_IN_FAST_PROG_CMD 0x38
#define READ_CONFIGURATION_REG_CMD 0x15
#define QUAD_OUT_FAST_READ_CMD 0x6B
#define QPI_ENABLE_CMD 0x35
#define DUMMY_CLOCK_CYCLES_READ_QUAD 10
#define RESET_ENABLE_CMD 0x66
#define RESET_EXECUTE_CMD 0x99
#define QSPI_OK ((uint8_t)0x00)
#define QSPI_ERROR ((uint8_t)0x01)
#define QSPI_BUSY ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED ((uint8_t)0x08)



#ifdef __cplusplus
}
#endif

#endif
