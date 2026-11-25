#include "wizchip_conf.h"
#include "context_w5500.h"

typedef void (*csSelect)(void);

static void W5500_SelectMainChip(void);
static void W5500_SelectReserveChip(void);
static void W5500_UnselectMainChip(void);
static void W5500_UnselectReserveChip(void);

static void SwitchContextToMainChip();
static void SwitchContextToReserveChip();

typedef struct NetChipContext_t{
    W550_Chip_e id;
    csSelect select;
    csSelect unSelect;
}NetChipContext_t;

typedef struct NetChipHandle_t{
    NetChipContext_t* onWebChip;
    NetChipContext_t* onRubetecChip;
    NetChipContext_t* chips[2];
}NetChipHandle_t;

NetChipContext_t mainChip = {
    .id = W5500_MAIN_CHIP,
    .select = W5500_SelectMainChip,
    .unSelect = W5500_UnselectMainChip
};

NetChipContext_t reserveChip = {
    .id = W5500_MAIN_CHIP,
    .select = W5500_SelectReserveChip,
    .unSelect = W5500_UnselectReserveChip
};

NetChipHandle_t hChipW5500 = {
    .onWebChip = &mainChip,
    .onRubetecChip = &mainChip,
    .chips = { &mainChip, &reserveChip }
};



static void SwitchContextToMainChip(){
    W5500_UnselectReserveChip();
    reg_wizchip_cs_cbfunc(W5500_SelectMainChip, W5500_UnselectMainChip);
}
static void SwitchContextToReserveChip(){
    W5500_SelectReserveChip();
    reg_wizchip_cs_cbfunc(W5500_SelectReserveChip, W5500_UnselectReserveChip);
}

static void W5500_SelectMainChip(void)
{
	//ulog("CS0 main"CRLF);
    HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_RESET);
}

static void W5500_SelectReserveChip(void)
{
	//ulog("CS0 reserve"CRLF);
    HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_RESET);
} 

static void W5500_UnselectMainChip(void)
{
	//ulog("CS1"CRLF);
    HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_SET);
}

static void W5500_UnselectReserveChip(void)
{
	//ulog("CS1"CRLF);
    HAL_GPIO_WritePin(E_CS_SPI_GPIO_Port, E_CS_SPI_Pin, GPIO_PIN_SET);
}

