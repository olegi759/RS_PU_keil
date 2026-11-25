#include "mx25lm51245g.h"
#include <string.h>


#define QSPI_CLK_ENABLE()           __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_FORCE_RESET()          __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()        __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_D0_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOE_CLK_ENABLE()
#define QSPI_D3_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()


#define GPIO_AF10_QSPI  			GPIO_AF10_QUADSPI
#define GPIO_AF9_QSPI  				GPIO_AF9_QUADSPI

/*
void mx25_QSPI_MspInit(QSPI_HandleTypeDef *QSPIHandle)
{
  GPIO_InitTypeDef gpio_init_structure;

  //##-1- Enable peripherals and GPIO Clocks #################################
  // Enable the QuadSPI memory interface clock
  QSPI_CLK_ENABLE();
  // Reset the QuadSPI memory interface
  //QSPI_FORCE_RESET();
  //QSPI_RELEASE_RESET();
  // Enable GPIO clocks
  QSPI_CS_GPIO_CLK_ENABLE();
  QSPI_CLK_GPIO_CLK_ENABLE();
  QSPI_D0_GPIO_CLK_ENABLE();
  QSPI_D1_GPIO_CLK_ENABLE();
  QSPI_D2_GPIO_CLK_ENABLE();
  QSPI_D3_GPIO_CLK_ENABLE();

  //##-2- Configure peripheral GPIO ##########################################
  // QSPI CS GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_CS_Pin;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = GPIO_AF10_QSPI;
  HAL_GPIO_Init(FLASH_CS_GPIO_Port, &gpio_init_structure);

  // QSPI CLK GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_CLK_Pin;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Alternate = GPIO_AF9_QSPI;
  HAL_GPIO_Init(FLASH_CLK_GPIO_Port, &gpio_init_structure);

  // QSPI D0 GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_Q0_Pin;
  gpio_init_structure.Alternate = GPIO_AF10_QSPI;
  HAL_GPIO_Init(FLASH_Q0_GPIO_Port, &gpio_init_structure);

  // QSPI D1 GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_Q1_Pin;
  gpio_init_structure.Alternate = GPIO_AF10_QSPI;
  HAL_GPIO_Init(FLASH_Q1_GPIO_Port, &gpio_init_structure);

  // QSPI D2 GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_Q2_Pin;
  gpio_init_structure.Alternate = GPIO_AF9_QSPI;
  HAL_GPIO_Init(FLASH_Q2_GPIO_Port, &gpio_init_structure);

  // QSPI D3 GPIO pin configuration
  gpio_init_structure.Pin       = FLASH_Q3_Pin;
  gpio_init_structure.Alternate = GPIO_AF9_QSPI;
  HAL_GPIO_Init(FLASH_Q3_GPIO_Port, &gpio_init_structure);

  //##-3- Configure the NVIC for QSPI #########################################
  // NVIC configuration for QSPI interrupt
  HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0x00);
  HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}



void mx25_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  // USER CODE BEGIN QUADSPI_MspInit 0

  // USER CODE END QUADSPI_MspInit 0

	  // Enable the QuadSPI memory interface clock
	  __HAL_RCC_QSPI_CLK_ENABLE();
	  // Reset the QuadSPI memory interface
	  //__HAL_RCC_QSPI_FORCE_RESET();
	  //__HAL_RCC_QSPI_RELEASE_RESET();
	  // Enable GPIO clocks

	  //QUADSPI clock enable
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
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

    //PC10(QUADSPI_BK1_IO1) и PC9(QUADSPI_BK1_IO0) -> PD12(FLASH_Q1) и PD11(FLASH_Q0) и PD13(FLASH_Q3)
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

    //PF10(QUADSPI_CLK) -> PB2(FLASH_CLK)
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

*/


void mx25_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  // USER CODE BEGIN QUADSPI_MspInit 0

  // USER CODE END QUADSPI_MspInit 0

	  // Enable the QuadSPI memory interface clock
	  __HAL_RCC_QSPI_CLK_ENABLE();
	  // Reset the QuadSPI memory interface
	  //__HAL_RCC_QSPI_FORCE_RESET();
	  //__HAL_RCC_QSPI_RELEASE_RESET();
	  // Enable GPIO clocks

	  //QUADSPI clock enable
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
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

    //PC10(QUADSPI_BK1_IO1) и PC9(QUADSPI_BK1_IO0) -> PD12(FLASH_Q1) и PD11(FLASH_Q0) и PD13(FLASH_Q3)
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

    //PF10(QUADSPI_CLK) -> PB2(FLASH_CLK)
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

uint8_t mx25_QSPI_GetStatus(QSPI_HandleTypeDef *QSPIHandle, uint8_t * reg)
{
  QSPI_CommandTypeDef s_command;
  //uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(QSPIHandle, reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}


uint8_t mx25_QSPI_Read(QSPI_HandleTypeDef *QSPIHandle, uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = QUAD_INOUT_FAST_READ_CMD;

  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  //s_command.AddressMode       = QSPI_ADDRESS_1_LINE;

  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  //s_command.AddressSize       = QSPI_ADDRESS_32_BITS;


  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

  s_command.DataMode          = QSPI_DATA_4_LINES;
  //s_command.DataMode          = QSPI_DATA_1_LINE;

  s_command.DummyCycles       = MX25L512_DUMMY_CYCLES_READ_QUAD;
  s_command.NbData            = Size;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Set S# timing for Read command: Min 20ns for N25Q128A memory */
  //MODIFY_REG(QSPIHandle->Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_2_CYCLE);

  /* Reception of the data */
  if (HAL_QSPI_Receive(QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Restore S# timing for nonRead commands */
  //MODIFY_REG(QSPIHandle->Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_5_CYCLE);

  return QSPI_OK;
}

uint8_t  mx25_QSPI_WriteEnable(QSPI_HandleTypeDef *QSPIHandle)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = MX25L512_SR_WREN;
  s_config.Mask            = MX25L512_SR_WREN;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(QSPIHandle, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *QSPIHandle, uint32_t Timeout)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.Mask            = MX25L512_SR_WIP;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(QSPIHandle, &s_command, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t mx25_QSPI_Write(QSPI_HandleTypeDef *QSPIHandle, uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = MX25L512_PAGE_SIZE - (WriteAddr % MX25L512_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

  /* Initialize the program command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = EXT_QUAD_IN_FAST_PROG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size;

    /* Enable write operations */
    if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure the command */
    if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Transmission of the data */
    if (HAL_QSPI_Transmit(QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady(QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + MX25L512_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25L512_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}


uint8_t  mx25_QSPI_ResetMemory(QSPI_HandleTypeDef *QSPIHandle)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reset enable command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}



//команда стирания сектора (SE) предназначена для стирания данных выбранного сектора до значения «1». Инструкция используется для
//любой сектор размером 4 КБ. Для установки бита фиксации разрешения записи (WEL) необходимо выполнить команду разрешения записи (WREN).
//перед отправкой команды «Стереть сектор» (SE). Любой адрес сектора (см. «Таблица 4. Организация памяти»)
//является допустимым адресом для инструкции стирания сектора (SE). CS# должен иметь высокий уровень точно на границе байта (наименьшее значение).
//значимый бит адресного байта заперт); в противном случае инструкция будет отклонена и не выполнена.

uint8_t mx25_QSPI_Erase_Sector(QSPI_HandleTypeDef *QSPIHandle, uint32_t SectorAddress)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = SECTOR_ERASE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = SectorAddress;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(QSPIHandle, MX25L512_SECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}


uint8_t mx25_QSPI_Erase_SubSector(QSPI_HandleTypeDef *QSPIHandle, uint32_t SubSectorAddress)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = SUBSECTOR_ERASE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = SubSectorAddress;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(QSPIHandle, MX25L512_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t mx25_QSPI_Erase_Chip(QSPI_HandleTypeDef *QSPIHandle)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = BULK_ERASE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(QSPIHandle, MX25L512_BULK_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}





uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg;

  // Initialize the read volatile configuration register command
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /// Configure the command
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /// Reception of the data
  if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /// Enable write operations
  if (mx25_QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  // Update volatile configuration register (with new dummy cycles)
  s_command.Instruction = WRITE_STATUS_CFG_REG_CMD;

  reg = 64;
  //MODIFY_REG(reg, N25Q128A_VCR_NB_DUMMY, (N25Q128A_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(N25Q128A_VCR_NB_DUMMY)));

  // Configure the write volatile configuration register command
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  // Transmission of the data
  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}


uint8_t mx25_QSPI_GetConfiguration(QSPI_HandleTypeDef *QSPIHandle, uint8_t * reg)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read flag status register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(QSPIHandle, reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

//1 байт Manufacture ID, 2 и 3 байт Device ID (C2 20 1A) (194 32 26)
uint8_t mx25_QSPI_GetIdentification(QSPI_HandleTypeDef *QSPIHandle, uint8_t * identification_data)
{
  QSPI_CommandTypeDef s_command;

  // Initialize the read flag status register command
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_ID_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 3;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  // Configure the command
  if (HAL_QSPI_Command(QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  // Reception of the data
  if (HAL_QSPI_Receive(QSPIHandle, identification_data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}


uint8_t mx25_QSPI_SetStatusAndConfiguration(QSPI_HandleTypeDef *QSPIHandle, uint8_t status_reg, uint8_t configuration_reg)
{
	QSPI_CommandTypeDef sCommand;
	uint8_t data[2] = { status_reg, configuration_reg };


	 /* Enable write operations */
	if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
	{
		return QSPI_ERROR;
	}



	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= WRITE_STATUS_CFG_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	sCommand.NbData= 2;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;

	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	if(HAL_QSPI_Transmit(QSPIHandle, data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}






uint8_t mx25_QSPI_Configuration(QSPI_HandleTypeDef *QSPIHandle)
{
	QSPI_CommandTypeDef sCommand;
	uint8_t test_buffer[4] = { 0 };
	/*read status register*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= READ_STATUS_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData= 1;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	if(HAL_QSPI_Receive(&hqspi, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	/*read configuration register*/
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= READ_CFG_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData= 1;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}

	if(HAL_QSPI_Receive(QSPIHandle, &(test_buffer[1]),	HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	  /* Enable write operations */
	  if (mx25_QSPI_WriteEnable(QSPIHandle) != QSPI_OK)
	  {
	    return QSPI_ERROR;
	  }

	/*modify buffer to enable quad mode*/
	test_buffer[0] = 0;//|= 0x40;
	/*set dummy cycles*/
	//test_buffer[1] |= 0xC0;
	test_buffer[1] = 3;
	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressSize= QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction= WRITE_STATUS_CFG_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	sCommand.NbData= 2;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	if(HAL_QSPI_Transmit(QSPIHandle, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t mx25_QSPI_ResetChip(QSPI_HandleTypeDef *QSPIHandle)
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
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
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
	sCommand.Instruction= RESET_MEMORY_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.Address= 0;
	sCommand.DataMode= QSPI_DATA_NONE;
	sCommand.DummyCycles= MX25L512_DUMMY_CYCLES_READ_QUAD;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}


uint8_t mx25_QSPI_EnableQPImode(QSPI_HandleTypeDef *QSPIHandle)
{
	QSPI_CommandTypeDef sCommand;

	sCommand.InstructionMode= QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction= ENTER_QUAD_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode= QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode= QSPI_DATA_NONE; //QSPI_DATA_1_LINE;
	sCommand.DummyCycles= 0;//MX25L512_DUMMY_CYCLES_READ_QUAD;
	sCommand.DdrMode= QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle= QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode= QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData= 1;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}


uint8_t mx25_QSPI_EnterMemory_QPI(QSPI_HandleTypeDef *_hqspi)
{
    QSPI_CommandTypeDef sCommand;
    QSPI_AutoPollingTypeDef sConfig;
    memset(&sCommand, 0, sizeof(QSPI_CommandTypeDef));
    memset(&sConfig, 0, sizeof(QSPI_AutoPollingTypeDef));

    /* Enable QPI operations ------------------------------------------ */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction = ENTER_QUAD_CMD; //QPI_ENABLE_CMD;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(_hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
            != HAL_OK) {
        return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for QPI enabling ---- */
    sConfig.Match = MX25L512_SR_QUADEN;
    sConfig.Mask = MX25L512_SR_QUADEN | MX25L512_SR_WIP;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1;
    sConfig.Interval = 0x10;
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction = READ_STATUS_REG_CMD;
    sCommand.DataMode = QSPI_DATA_4_LINES;

    if (HAL_QSPI_AutoPolling(_hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

uint8_t  mx25_EnableQuadMode(QSPI_HandleTypeDef *QSPIHandle)
{
	uint8_t test_buffer[4] = { 0 };
	QSPI_CommandTypeDef sCommand;

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
	sCommand.Instruction= WRITE_STATUS_CFG_REG_CMD;
	sCommand.AddressMode= QSPI_ADDRESS_NONE;
	sCommand.DataMode= QSPI_DATA_1_LINE;
	sCommand.DummyCycles= 0; //QSPI_DUMMY_CYCLES;
	sCommand.NbData= 2;
	if(HAL_QSPI_Command(QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	if(HAL_QSPI_Transmit(QSPIHandle, test_buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		Error_Handler();
		return QSPI_ERROR;
	}
	return QSPI_OK;

}
