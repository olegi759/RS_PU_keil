#ifndef __CH376_H__
#define __CH376_H__

#include "main.h"


#ifndef		BOOL
#define		BOOL	unsigned char
#endif


#ifndef		TRUE
#define		TRUE	1
#endif

#ifndef		FALSE
#define		FALSE	0
#endif

#ifndef		NULL
#define		NULL	0
#endif




#define	CMD_GET_IC_VER		 	0x01
#define	CMD_SET_BAUDRATE		0x02
#define	CMD_ENTER_SLEEP   	 	0x03
#define	CMD_RESET_ALL      		0x05
#define	CMD_CHECK_EXIST			0x06
#define	CMD_SET_SD0_INT			0x0B
#define	CMD_GET_FILE_SIZE		0x0C

//378
#define	CMD_SET_FILE_SIZE		0x0D
#define	CMD_SET_USB_MODE		0x15
#define	CMD_GET_STATUS			0x22
//378
#define	CMD_RD_HOST_OFS_DATA	0x26 //378
//378 #define	CMD_RD_USB_DATA0   0x27
#define	CMD_RD_HOST_CUR_DATA	0x27
//378
#define	CMD_RD_HOST_REQ_DATA	0x28
#define	CMD_WR_USB_DATA			0x2C
//378 #define	CMD_WR_REQ_DATA			0x2D
#define	CMD_WR_HOST_OFS_DATA	0x2D
//378 #define	CMD_WR_OFS_DATA			0x2E
#define	CMD_WR_HOST_CUR_DATA	0x2E
#define	CMD_SET_FILE_NAME		0x2F
#define	CMD_DISK_CONNECT		0x30
#define	CMD_DISK_MOUNT			0x31
#define	CMD_FILE_OPEN			0x32
#define	CMD_FILE_ENUM_GO		0x33
#define	CMD_FILE_CREATE			0x34
#define	CMD_FILE_ERASE			0x35
#define	CMD_FILE_CLOSE			0x36
#define	CMD_DIR_INFO_READ		0x37
#define	CMD_DIR_INFO_SAVE		0x38
#define	CMD_BYTE_LOCATE			0x39
#define	CMD_BYTE_READ			0x3A
#define	CMD_BYTE_RD_GO			0x3B
#define	CMD_BYTE_WRITE			0x3C
#define	CMD_BYTE_WR_GO			0x3D
#define	CMD_DISK_CAPACITY		0x3E
#define	CMD_DISK_QUERY			0x3F
#define	CMD_DIR_CREATE			0x40
#define	CMD_SEC_LOCATE			0x4A
#define	CMD_SEC_READ			0x4B
#define	CMD_SEC_WRITE			0x4C
#define	CMD_DISK_BOC_CMD		0x50
#define	CMD_DISK_READ			0x54
//378 #define	CMD_DISK_RD_GO			0x55
#define	CMD_DISK_FILE_QUERY		0x55
#define	CMD_DISK_WRITE			0x56
//378  #define	CMD_DISK_WR_GO			0x57
#define	CMD_FILE_MODIFY			0x57



#define CMD_RET_SUCCESS    0x51
#define CMD_RET_ABORT      0x5F


//ответ на status
#define USB_INT_SUCCESS     0x14  // USB transaction or transfer operation was successful Успешность транзакции или операции передачи данных с SD-картой или USB-накопителем
#define USB_INT_CONNECT     0x15  // USB device connection event detected, may be a new connection or reconnect after disconnecting Обнаружено событие подключения USB-устройства, может быть новое подключение или повторное подключение после отключения
#define USB_INT_DISCONNECT  0x16  // USB device disconnect event detected Обнаружение отсоединения USB-устройства
#define USB_INT_BUF_OVER    0x17  // USB data transmission error or too much data buffer overflow Ошибка передачи данных USB или слишком большое переполнение буфера данных
#define USB_INT_USB_READY   0x18  // USB device has been initialized (USB address assigned) USB-устройство инициализировано (USB-адрес назначен)
#define USB_INT_DISK_READ   0x1D  // USB memory request data read out Считывание данных запроса USB-накопителя
#define USB_INT_DISK_WRITE  0x1E  // USB memory request data write Запись данных запроса на USB-память
#define USB_INT_DISK_ERR    0x1F  // USB memory operation failed Операция с USB-накопителем не удалась Сбой USB-накопителя


#define ERR_SUCCESS			0x00
#define ERR_OPEN_DIR 		0x41
#define ERR_MISS_FILE 		0x42
#define ERR_FOUND_NAME 		0x43
#define ERR_DISK_DISCON 	0x82
#define ERR_LARGE_SECTOR	0x84
#define ERR_TYPE_ERROR 		0x92
#define ERR_BPB_ERROR 		0xA1
#define ERR_DISK_FULL 		0xB1
#define ERR_FDT_OVER 		0xB2
#define ERR_FILE_CLOSE  	0xB4

// 3 = /subdir1/subdir2/subdir3
//#define MAXDIRDEPTH 3
// waiting for data from CH
//#define ANSWTIMEOUT 1000

//#define CH376_ERR_TIMEOUT 		0x02
//#define CH376_ERR_LONGFILENAME 	0x04
//#define ANSW_ERR_FOUND_NAME		0x43
//File not found
//#define ANSW_ERR_MISS_FILE		0x42

void ch378_resetAll(void);
//uint8_t ch378_waitForResponse(void);
uint8_t ch378_checkExist(uint8_t in);
uint8_t ch378_setUSBmode(uint8_t code);
uint8_t ch378_getStatus(void);
uint8_t ch378_diskConnectionStatus(void);
uint8_t ch378_mount(void);


uint8_t ch378_openFile(void);
uint8_t ch378_closeFile(uint8_t d);
void ch378_setFileName(const char * path);


uint16_t ch378_byteRead(uint16_t size);
uint8_t ch378_byteReadGo(void);
uint8_t ch378_readData0(uint8_t * data);
uint8_t ch378_diskQuery(void);
uint8_t ch378_fileEnumGo(void);
//uint8_t ch378_listDir(const char* path);
uint8_t ch378_filePosition(uint32_t Position);
//uint8_t ch378_deleteDir(void);
//uint8_t ch378_saveFileAttrb(void);
//uint8_t ch378_deleteFile(void);
//uint8_t ch378_dirCreate(void);
//uint8_t ch378_cd(const char* dirPath, BOOL mkDir);
uint8_t ch378_getVersion(void);

//void ch378_RdUsbData0(uint8_t * rx, uint16_t rx_len);
void ch378_setSd0Int(uint8_t mode);




uint8_t ch378_createFile(void);
uint16_t ch378_byteWrite(uint16_t size);
uint8_t ch378_writeReqData(uint8_t * data, uint8_t cnt);
uint8_t ch378_byteWrGo(void);

uint8_t ch378_deleteFile(void);


uint16_t ch378_rd_host_req_data(uint8_t * data);
void ch378_rd_host_cur_data(uint8_t * data, uint16_t size);
void ch378_wr_host_cur_data(uint8_t * data, uint16_t size);



#pragma pack(push, 1)

typedef struct __ch378_file_info {
	char Filename[8];
	char Extension[3];
	uint8_t File_attrib;

	uint8_t User_attrib;
	uint8_t First_ch_del;
	uint8_t Create_time[2];
	uint8_t Create_date[2];
	uint8_t Owner_ID[2];
	uint8_t Acc_rights[2];

	uint8_t Mod_time[2];
	uint8_t Mod_date[2];
	uint8_t Start_cluster[2];
	uint32_t File_size;
} ch378_file_info_t;

#pragma pack(pop)

//корневой каталог
#define CH378_ENUM_ROOT_PATH	"/*"



#define CH378_MAX_READ_BYTES	20480





#endif
