/*
 * old_qspi.c
 *
 *  Created on: 21 нояб. 2023 г.
 *      Author: Rafael Boltachev
 */

#include "mx25lm51245g.h"
#include <string.h>

/* QUADSPI init function */
uint8_t mx25_QSPI_Init(void)
{

  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 4;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 25;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
    return HAL_ERROR;
  }

  return HAL_OK;
}


void mx25_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  // USER CODE BEGIN QUADSPI_MspInit 0

  // USER CODE END QUADSPI_MspInit 0
     //QUADSPI clock enable
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    //QUADSPI GPIO Configuration
    //PE2     ------> QUADSPI_BK1_IO2
    //PB6     ------> QUADSPI_BK1_NCS
    //PC10     ------> QUADSPI_BK1_IO1
    //PC9     ------> QUADSPI_BK1_IO0
    //PF6     ------> QUADSPI_BK1_IO3
    //PF10     ------> QUADSPI_CLK

    //PE2(QUADSPI_BK1_IO2) -> PE2(FLASH_Q2)
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    //PB6(QUADSPI_BK1_NCS) -> PB6(FLASH_CS)
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //PC10(QUADSPI_BK1_IO1) и PC9(QUADSPI_BK1_IO0) -> PD12(FLASH_Q1) и PD11(FLASH_Q0)
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    //PF6(QUADSPI_BK1_IO3) и PF10(QUADSPI_CLK) -> PD13(FLASH_Q3) и PB2(FLASH_CLK)
    //GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_10;
    //GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //GPIO_InitStruct.Pull = GPIO_NOPULL;
    //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    //HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // USER CODE BEGIN QUADSPI_MspInit 1

  // USER CODE END QUADSPI_MspInit 1
  }
}

/*
void mx25_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  // USER CODE BEGIN QUADSPI_MspInit 0

  // USER CODE END QUADSPI_MspInit 0
     //QUADSPI clock enable
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    //QUADSPI GPIO Configuration
    //PE2     ------> QUADSPI_BK1_IO2
    //PB6     ------> QUADSPI_BK1_NCS
    //PC10     ------> QUADSPI_BK1_IO1
    //PC9     ------> QUADSPI_BK1_IO0
    //PF6     ------> QUADSPI_BK1_IO3
    //PF10     ------> QUADSPI_CLK

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  // USER CODE BEGIN QUADSPI_MspInit 1

  // USER CODE END QUADSPI_MspInit 1
  }
}
*/



void mx25_HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB6     ------> QUADSPI_BK1_NCS
    PC10     ------> QUADSPI_BK1_IO1
    PC9     ------> QUADSPI_BK1_IO0
    PF6     ------> QUADSPI_BK1_IO3
    PF10     ------> QUADSPI_CLK
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_10);

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

uint8_t mx25_QSPI_ResetChip()
{
	QSPI_CommandTypeDef sCommand;
	uint32_t temp = 0;
	/* Erasing Sequence ---------------------------------*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= RESET_ENABLE_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.Address= 0;
	sCommand.DataMode= QSPI_DATA_NONE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	for(temp = 0; temp < 0x2f; temp++)
	{
		__NOP();
	}
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= RESET_EXECUTE_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.Address= 0;
	sCommand.DataMode= QSPI_DATA_NONE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t mx25_QSPI_AutoPollingMemReady(void)
{
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	memset(&sCommand, 0, sizeof(sCommand));
	memset(&sConfig, 0, sizeof(sConfig));
	/* Configure automatic polling mode to wait for memory ready ------*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= READ_STATUS_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sConfig.Match= 0x00;
	sConfig.Mask= 0x01;
	sConfig.MatchMode= QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize= 1;
	sConfig.Interval= 0x10;
	sConfig.AutomaticStop= QSPI_AUTOMATIC_STOP_ENABLE;
	if(HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t mx25_QSPI_WriteEnable(void)
{
	QSPI_CommandTypeDef sCommand;
	QSPI_AutoPollingTypeDef sConfig;
	memset(&sCommand, 0, sizeof(sCommand));
	memset(&sConfig, 0, sizeof(sConfig));
	/* Enable write operations ------------------------------------------*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= WRITE_ENABLE_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_NONE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* Configure automatic polling mode to wait for write enabling ----*/
	sConfig.Match= 0x02;
	sConfig.Mask= 0x02;
	sConfig.MatchMode= QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize= 1;
	sConfig.Interval= 0x10;
	sConfig.AutomaticStop= QSPI_AUTOMATIC_STOP_ENABLE;
	sCommand.Instruction= READ_STATUS_REG_CMD;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	if(HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_5, GPIO_PIN_SET);
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t mx25_QSPI_Configuration()
{
	QSPI_CommandTypeDef sCommand;
	uint8_t test_buffer[4] = { 0 };
	/*read status register*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= READ_STATUS_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData= 1;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	if(HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	/*read configuration register*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= READ_CONFIGURATION_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData= 1;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}

	if(HAL_QSPI_Receive(&hqspi, &(test_buffer[1]),	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	/*modify buffer to enable quad mode*/
	test_buffer[0] |= 0x40;
	/*set dummy cycles*/
	test_buffer[1] |= 0xC0;
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= WRITE_STATUS_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	sCommand.NbData= 2;
	if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	if(HAL_QSPI_Transmit(&hqspi, test_buffer,
	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t mx25_QSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
	QSPI_CommandTypeDef sCommand;
	EraseStartAddress= EraseStartAddress - EraseStartAddress % MEMORY_SECTOR_SIZE;
	/* Erasing Sequence --------------------------------------------*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= SECTOR_ERASE_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_1_LINE;
	sCommand.DataMode= QSPI_DATA_NONE;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;
	while(EraseEndAddress>= EraseStartAddress)
	{
		sCommand.Address= (EraseStartAddress& 0x0FFFFFFF);
		if(mx25_QSPI_WriteEnable() != QSPI_OK)
		{
			return QSPI_ERROR;
		}
		if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
		{
			return QSPI_ERROR;
		}
		EraseStartAddress+= MEMORY_SECTOR_SIZE;
		if(mx25_QSPI_AutoPollingMemReady() != QSPI_OK)
		{
			return QSPI_ERROR;
		}
	}
	return QSPI_OK;
}



uint8_t mx25_QSPI_ReadMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  //s_command.Instruction       = QUAD_INOUT_FAST_READ_CMD;
  s_command.Instruction       = QUAD_OUT_FAST_READ_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = QSPI_DUMMY_CYCLES;
  s_command.NbData            = buffer_size;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Set S# timing for Read command: Min 20ns for N25Q128A memory */
  //MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_2_CYCLE);

  /* Reception of the data */
  if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Restore S# timing for nonRead commands */
  //MODIFY_REG(hqspi.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_5_CYCLE);

  return QSPI_OK;
}


uint8_t mx25_QSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size)
{
	QSPI_CommandTypeDef sCommand;
	uint32_t end_addr, current_size, current_addr;
	/* Calculation of the size between the write address and the end of the page */
	current_addr= 0;
	//
	while(current_addr<= address)
	{
		current_addr+= MEMORY_PAGE_SIZE;
	}
	current_size= current_addr-address;
	/* Check if the size of the data is less than the remaining place in the page */
	if(current_size> buffer_size)
	{
		current_size= buffer_size;
	}
	/* Initialize the adressvariables */
	current_addr= address;
	end_addr= address + buffer_size;
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= QUAD_IN_FAST_PROG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_4_LINES;
	sCommand.DataMode= QSPI_DATA_4_LINES;
	sCommand.NbData= buffer_size;
	sCommand.Address= address;
	sCommand.DummyCycles= QSPI_DUMMY_CYCLES;

	/* Perform the write page by page */
	do
	{
		sCommand.Address= current_addr;
		sCommand.NbData= current_size;
		if(current_size== 0)
		{
			return QSPI_OK;
		}
		/* Enable write operations */
		if(mx25_QSPI_WriteEnable() != QSPI_OK)
		{
			return QSPI_ERROR;
		}
		/* Configure the command */
		if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		{
			return QSPI_ERROR;
		}
		/* Transmission of the data */
		if(HAL_QSPI_Transmit(&hqspi, buffer, buffer_size) != HAL_OK)
		{
			return QSPI_ERROR;
		}
		/* Configure automatic polling mode to wait for end of program */
		if(mx25_QSPI_AutoPollingMemReady() != QSPI_OK)
		{
			return QSPI_ERROR;
		}
		/* Update the address and size variables for next page programming */
		current_addr+= current_size;
		buffer += current_size;
		current_size=
		((current_addr+ MEMORY_PAGE_SIZE) > end_addr) ? (end_addr-current_addr) : MEMORY_PAGE_SIZE;
	} while(current_addr<= end_addr);

	return QSPI_OK;
}

uint8_t mx25_QSPI_EnableMemoryMappedMode()
{
	QSPI_CommandTypeDef sCommand;
	QSPI_MemoryMappedTypeDef sMemMappedCfg;
	/* Enable Memory-Mapped mode------------------------------------------*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_1_LINE;
	sCommand.DataMode= QSPI_DATA_4_LINES;
	sCommand.NbData= 0;
	sCommand.Address= 0;
	sCommand.Instruction= QUAD_OUT_FAST_READ_CMD;
	sCommand.DummyCycles= DUMMY_CLOCK_CYCLES_READ_QUAD;
	sMemMappedCfg.TimeOutActivation= QSPI_TIMEOUT_COUNTER_DISABLE;
	if(HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

