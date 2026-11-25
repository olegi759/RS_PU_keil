/*
 * external_flash.c
 *
 *  Created on: 14 С„РµРІСЂ. 2025 Рі.
 *      Author: Rafael Boltachev
 */


#include "external_flash.h"
#include "ch378.h"
#include "stm_util.h"
#include "str_util.h"
#include "utf8_util.h"
#include "dynbuf_util.h"
#include "fatfs_util.h"

//СЃС‚Р°СЂС‹Р№ СЃС‚Р°С‚СѓСЃ
static uint8_t ch378_status_old = 0;
//СЃРјРѕРЅС‚РёСЂРѕРІР°РЅР° Р»Рё С„Р»СЌС…Р°
BOOL external_flash_is_mount = FALSE;


void external_flash_init(void)
{
	uint8_t res;
	uint8_t cnt;


	PIN_CLR(VBUS_ON); //РІРєР»СЋС‡РёРј РїРёС‚Р°РЅРёРµ РЅР° РІРЅРµС€РЅСЋСЋ С„Р»СЌС€ (Сѓ EN (VBUS_ON) Р°РєС‚РёРІРЅС‹Р№ СѓСЂРѕРІРµРЅСЊ =0)



	PIN_SET(UCN_RST);
	HAL_Delay(100);
	PIN_CLR(UCN_RST);
	HAL_Delay(100);
	PIN_SET(UCN_RST);
	HAL_Delay(500);



	// 1. CH378 chip reset
	ch378_resetAll();
	ulog_fmt(CONM"ch378: resetAll"CRLF);
	HAL_Delay(100); //Usually, the hardware reset finishes within 35mS.

	//ch378_setSd0Int(0x10);


	// 2. test communication interface
	cnt = 10;
	while (cnt--) {
		res = ch378_checkExist(0xAA);
		ulog_fmt(CONM"ch378: checkExist(0x%02X)=0x%02X"CRLF, 0xAA , res);
		if (res == 0x55) break;
		HAL_Delay(1);
	}

	if (res != 0x55) {
		ulog(CONM"ch378: INIT FAILURE"CRLF);
		//return;
	}



	res = ch378_getVersion();
	ulog_fmt(CONM"ch378: getVersion=0x%02X"CRLF, res);


	res = ch378_setUSBmode(0x07);
	ulog_fmt(CONM"ch378: setUSBmode(0x05)=0x%02X"CRLF, res);

}


uint8_t external_flash_wait_status(uint8_t status_wait, uint16_t count)
{
	uint8_t status_now = 0;

	while (count--) {
		status_now = ch378_getStatus();
		if (status_now == status_wait) break;
		HAL_Delay(1);
	}
	return status_now;
}



void external_flash_set_file_name(const char * path)
{
	char file_path_oem[64];


	memset(file_path_oem, 0, sizeof(file_path_oem));
	if (path[0] == '/') UTF8ToCP866(file_path_oem, sizeof(file_path_oem), path);
	else {
		file_path_oem[0] = '/';
		UTF8ToCP866(&file_path_oem[1], sizeof(file_path_oem) - 1, path);

	}

	ch378_setFileName(file_path_oem); // "/ENGLISH.TXT"
}


BOOL external_flash_open_file(const char * path)
{
	uint8_t res;

	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_open_file not mount"CRLF);
		return FALSE;
	}

	external_flash_set_file_name(path);

	res = ch378_openFile();
    //ulog_fmt("ch378: openFile=0x%02X"CRLF, res);

    //Р•СЃР»Рё СѓСЃРїРµС€РЅРѕ РѕС‚РєСЂС‹С‚ С„Р°Р№Р», РїСЂРµСЂС‹РІР°РЅРёРµ РІРѕР·РІСЂР°С‰Р°РµС‚ ERR_SUCCESS, Рё СЂР°Р·РјРµСЂ С„Р°Р№Р»Р° РґРµР№СЃС‚РІРёС‚РµР»РµРЅ
    res = external_flash_wait_status(ERR_SUCCESS, 1000);
	//ulog_fmt("ch378: external_flash_open_file Status=0x%02X"CRLF, res);

	return (res == ERR_SUCCESS);
}


// "/*"
BOOL external_flash_open_enum(const char * path)
{
	uint8_t res;
	char file_path_oem[64];

	memset(file_path_oem, 0, sizeof(file_path_oem));
	UTF8ToCP866(file_path_oem, sizeof(file_path_oem), path);

	ch378_setFileName(file_path_oem);
	res = ch378_openFile();
    //ulog_fmt("ch378: openFile=0x%02X"CRLF, res);


    res = external_flash_wait_status(USB_INT_DISK_READ, 100);

	//ulog_fmt("ch378: txternal_flash_open_enum Status=0x%02X"CRLF, res);

	return (res == USB_INT_DISK_READ);
}


//The command used to close the file or directory which has been open. Need one input data, to notify
//whether updata file length, 0 means forbid update file length, 1 means automatically update file length.

BOOL external_flash_close_file(uint8_t update)
{
	uint8_t res;


	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_close_file not mount"CRLF);
		return FALSE;
	}

	res = ch378_closeFile(update);
	//ulog_fmt("ch378: closeFile(0)=0x%02X"CRLF, res);


    res = external_flash_wait_status(ERR_SUCCESS, 100);
	//ulog_fmt("ch378: ch378_closeFile Status=0x%02X"CRLF, res);


	return (res == USB_INT_SUCCESS);
}

//Р•СЃР»Рё РёРЅС‚РµСЂРµСЃРЅРѕ, РїРѕРІС‚РѕСЂРёС‚СЊ, С‚Рѕ РґРµР»Р°РµРј С‚Р°Рє:

//1 вЂ” $57$AB$2F/*$00 set_file_name
//2 вЂ” $57$AB$32 open_file

//3 вЂ” $57$AB$33 file_enum_go
//4 вЂ” $57$AB$27 rd_usb_data0
//РµСЃР»Рё РїРѕСЃР»Рµ file_enum_go 1D РїРѕРІС‚РѕСЂСЏРµРј РїРѕСЃР»РµРґРѕРІР°С‚РµР»СЊРЅРѕСЃС‚СЊ 3-4, РїРѕРєР° РЅРµ РїРѕСЏРІРёС‚СЃСЏ 42 РїРѕСЃР»Рµ РІС‹РїРѕР»РЅРµРЅРёСЏ file_enum_go
//РќРµ РѕС‡РµРЅСЊ РїРѕРЅРёРјР°СЋ РїРѕС‡РµРјСѓ РёР· Р±СѓС„РµСЂР° РІРѕР·РІСЂР°С‰Р°РµС‚СЃСЏ 33 Р±Р°Р№С‚Р°, Р° РІСЂРѕРґРµ РґРѕР»Р¶РЅРѕ Р±С‹С‚СЊ 32 ???


void get_file_name_from_ch378_file_info(ch378_file_info_t * ch378_file_info, char * buf)
{
    char Extension[4];
    char file_name_oem[14 + 1 + 4];

    strcpy_s(Extension, sizeof(Extension), strtrim(ch378_file_info->Extension));
    ch378_file_info->Extension[0] = '\0';


    strcpy_s(file_name_oem, sizeof(file_name_oem), strtrim(ch378_file_info->Filename));
    strcat_s(file_name_oem, sizeof(file_name_oem), ".");
    strcat_s(file_name_oem, sizeof(file_name_oem), Extension);

    CP866ToUTF8(buf, file_name_oem);
}


BOOL external_flash_get_file_list_item_func_log(int index, char * file_name, unsigned long file_size, void * param)
{
	ulog_fmt(CONM"ExtFlash: %d) file:%s size:%lu"CRLF, index, file_name, file_size);
	return FALSE;
}


uint16_t external_flash_enum_files(const char * path, on_enum_files_func_t on_enum_files_func, void * param)
{
	uint8_t res;
	int index;
	uint8_t buf[256];
	ch378_file_info_t * file_info;
	char file_name_utf[28 + 1 + 8];
	file_info = (ch378_file_info_t *)buf;


	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_get_file_list not mount"CRLF);
		return 0;
	}

	file_info = (ch378_file_info_t *)buf;
	index = 0;


	if (!external_flash_open_enum(path)) return 0;


    while (TRUE) {
    	res = ch378_fileEnumGo();
    	//ulog_fmt("ch378: fileEnumGo=0x%02X"CRLF, res);

        res = external_flash_wait_status(USB_INT_DISK_READ, 50);
    	//ulog_fmt("ch378: Status=0x%02"CRLF, res);


    	if (res != USB_INT_DISK_READ) break;

    	res = ch378_rd_host_req_data(buf);
    	//ulog_fmt("ch378: ch378_rd_host_req_data=%d"CRLF, res);


    	//ulog_fmt("Ach378: %s.%s"CRLF, file_info->Filename, file_info->Extension);

    	//char str[200];
        //mem2hexstr(rfile_info, res, str, sizeof(str), " ");
    	//ulog_fmt(CONM" %s"CRLF,  str);



    	if (res < sizeof(ch378_file_info_t)) continue;

    	get_file_name_from_ch378_file_info(file_info, file_name_utf);
    	//ulog_fmt("ch378: file_name_utf=%s"CRLF, file_name_utf);

    	if (on_enum_files_func(index, file_name_utf, file_info->File_size, param)) break;
    	index++;
    }

    //external_flash_close_file();

    return index;
}


BOOL external_flash_file_exist(const char * path)
{
	BOOL res;


	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_file_exist not mount"CRLF);
		return FALSE;
	}


	res = external_flash_open_file(path);
	external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);


    return res;
}

uint16_t external_flash_read_file(uint8_t * buf, uint16_t cnt)
{
	//uint8_t len;
	//uint8_t res;
	uint16_t l;

	//l =
	ch378_byteRead(cnt);
	//ulog_fmt("ch378: byteRead(%d)=%d"CRLF, cnt, l);
	//if (!l) return 0;

    //res =
    external_flash_wait_status(ERR_SUCCESS, 100);
	//ulog_fmt("ch378: byteRead Status=0x%02"CRLF, res);

	//ch378_rd_host_cur_data(buf, l);
	//ulog_fmt("ch378: ch378_rd_host_cur_data()=%d"CRLF, l);

	l = ch378_rd_host_req_data(buf);
	//ulog_fmt("ch378: ch378_rd_host_req_data()=%d"CRLF, l);

	return l;
}


//.4.5. Create file and write data
//1) CMD_SET_FILE_NAME command + CMD_FILE_CREATE command, create file
//2) many times CMD_BYTE_WRITE command + CMD_WR_REQ_DATA command + CMD_BYTE_WR_GO command, write data
//3) CMD_FILE_CLOSE command, parameter is 1, close file and allow automatic update file size


uint8_t external_flash_write_file(uint8_t * buf, uint8_t cnt)
{
	//uint16_t l;
	//uint8_t res;

	ch378_wr_host_cur_data(buf, cnt);

	//l =
			ch378_byteWrite(cnt); //CMD_BYTE_WRITE
	//ulog_fmt("ch378: byteWrite(%d)=%d"CRLF, l);

	//res =
			external_flash_wait_status(ERR_SUCCESS, 1000);
	//ulog_fmt("ch378: byteWrite external_flash_wait_status=0x%02"CRLF, res);

	return cnt;


/*
	uint8_t res;
	uint8_t len;


	res = ch378_byteWrite(cnt); //CMD_BYTE_WRITE
	//ulog_fmt("ch378: byteWrite(%d)=%d"CRLF, cnt, res);

	//res = external_flash_wait_status(USB_INT_DISK_WRITE, 1000); //РІРѕР·РІСЂ USB_INT_DISK_WRITE
	//ulog_fmt("ch378: byteWrite wait=0x%02"CRLF, res);

	len = 0;
    while (len < cnt) {
    	res = ch378_writeReqData(&buf[len], cnt - len); //CMD_WR_REQ_DATA
    	//ulog_fmt("ch378: writeReqData(%d)=%d"CRLF, cnt - len, res);

    	if (!res) {

    		res = ch378_byteWrGo(); //CMD_BYTE_WR_GO

        	res = ch378_byteWrite(cnt - len);
        	//ulog_fmt("---ch378: byteWrite(%d)=%d"CRLF, cnt - len, res);

        	res = ch378_writeReqData(&buf[len], cnt - len); //CMD_WR_REQ_DATA
        	//ulog_fmt("---ch378: writeReqData(%d)=%d"CRLF, cnt - len, res);

        	if (!res) break;

    	}

    	len += res;



    	res = ch378_byteWrGo(); //CMD_BYTE_WR_GO
    	//ulog_fmt("ch378: byteWrGo()=%d"CRLF, res);

    	//res = external_flash_wait_status(USB_INT_SUCCESS, 1000); //РІРѕР·РІСЂ USB_INT_SUCCESS
    	//ulog_fmt("ch378: byteWrGo wait=0x%02"CRLF, res);
    }

	return len;
*/
}



void external_flash_status_usb_int_connect(void)
{
	uint8_t res;
	//uint16_t cnt;

	ulog(CONM"ch378: USB device connection event detected"CRLF);
	//ch378_setUSBmode(0x05);
	//HAL_Delay(10);
	res = ch378_setUSBmode(0x06);
	ulog_fmt(CONM"ch378: setUSBmode(6)=0x%02X"CRLF, res);

	HAL_Delay(10);


	int i = 10;

	while (i--) {

		res = ch378_diskConnectionStatus();
		//ulog_fmt(CONM"ch378: diskConnectionStatus=0x%02X"CRLF, res);
		if (res == ERR_SUCCESS) break;
		HAL_Delay(10);
	}

	ulog_fmt(CONM"ch378: ch378_diskConnectionStatus=0x%02X"CRLF, res);

    //res = external_flash_wait_status(USB_INT_SUCCESS, 50);
	//ulog_fmt(CONM"ch378: Status=0x%02"CRLF, res);

	if (res != ERR_SUCCESS) {
		ulog(CONM"ch378: diskConnectionStatus failure"CRLF);
		ch378_setUSBmode(0x07);
		return;
	}


	i = 10;
	while (i--) {

		res = ch378_mount();
		//ulog_fmt(CONM"ch378: mount=0x%02X"CRLF, res);
		if (res == CMD_DISK_MOUNT) break;
		HAL_Delay(10);
	}
	ulog_fmt(CONM"ch378: mount=0x%02X"CRLF, res);

	//res = external_flash_wait_status(USB_INT_SUCCESS, 500);
	//ulog_fmt("ch378: Status=0x%02"CRLF, res);

	if (res != CMD_DISK_MOUNT) {
		ulog(CONM"ch378: error mount"CRLF);
		ch378_setUSBmode(0x07);
		return;
	}

	ulog(CONM"ch378: IS MOUNT NOW"CRLF);

	external_flash_is_mount = TRUE;

	//cnt = external_flash_get_file_list(CH376_ENUM_ROOT_PATH, external_flash_get_file_list_item_func_log);
	//ulog_fmt("ch378: external_flash_get_file_list()=%d"CRLF, cnt);
}

void external_flash_status_usb_int_disconnect(void)
{
	ulog(CONM"ch378: USB device disconnect event detected"CRLF);
	external_flash_is_mount = FALSE;
	ch378_setUSBmode(0x06);
}


//РЅРµС‡Р°СЃС‚Рѕ
void external_flash_task(void)
{
	uint8_t ch378_status_now;

	ch378_status_now = ch378_getStatus();
	//ulog_fmt(CONM"ch378: ch378_getStatus()=0x%02X"CRLF, ch378_status_now);


	if ((ch378_status_old != USB_INT_CONNECT) && (ch378_status_now == USB_INT_CONNECT)) {
		external_flash_status_usb_int_connect();
	}
	else if ((ch378_status_old != USB_INT_DISCONNECT) && (ch378_status_now == USB_INT_DISCONNECT)) {
		external_flash_status_usb_int_disconnect();
	}

	ch378_status_old = ch378_status_now;

}



unsigned int external_flash_stream_read(void * afile, void * data, unsigned int data_len)
{
	//return external_flash_read_file(data, ( data_len > CH378_MAX_READ_BYTES ? CH378_MAX_READ_BYTES : data_len ) );

	unsigned int br, len;
	unsigned char * buf;
	uint8_t res;




	//ulog_fmt("external_flash_stream_read(%lu)"CRLF, data_len);

	br = 0;
	buf = data;
	while (br < data_len) {
		len = data_len - br;
		res = external_flash_read_file(&buf[br], ( len > 255 ? 255 : len ) );
		if (!res) break;
		br += res;
	}

	return br;

}

//СЃРєРѕРїРёСЂРѕРІР°С‚СЊ С„Р°Р№Р» РёР· FlashDisk РІ Р»РѕРєР°Р»СЊРЅСѓСЋ РїР°РјСЏС‚СЊ
BOOL external_flash_copy_to_local(const char * file_name)
{

	FIL file;
	uint8_t buf[250], rd;
	UINT bw;


	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_copy_to_local not mount"CRLF);
		return FALSE;
	}

	ulog_fmt(CONM"external_flash_copy_to_local(%s) begin"CRLF, file_name);

	if (!external_flash_open_file(file_name)) {
		ulog_fmt(CONM"external_flash_copy_to_local error open 1 (%s)"CRLF, file_name);
		return FALSE;
	}


	if ( f_open(&file, file_name, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
		ulog_fmt(CONM"external_flash_copy_to_local error open 2 (%s)"CRLF, file_name);
		return FALSE;
	}

	while ((rd = external_flash_read_file(buf, sizeof(buf))) > 0) {
		f_write(&file, buf, rd, &bw);
		HAL_IWDG_Refresh(&hiwdg2);
	}


	f_close(&file);
	external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);

	ulog_fmt(CONM"external_flash_copy_to_local (%s) end"CRLF, file_name);

	return TRUE;
}



uint8_t external_flash_file_delete(const char * file_name)
{
	uint8_t res;

	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_file_delete not mount"CRLF);
		return FALSE;
	}

	external_flash_set_file_name(file_name);
	//external_flash_open_file(file_name);

	res = ch378_deleteFile();
	//ulog_fmt("external_flash_file_delete(%s)=%d"CRLF, file_name, res);

	res = external_flash_wait_status(USB_INT_SUCCESS, 1000); //РІРѕР·РІСЂ USB_INT_SUCCESS
	ulog_fmt("external_flash_file_delete  deleteFile wait=%d"CRLF, res);

	return res;

}

uint8_t external_flash_file_create(const char * file_name)
{
	uint8_t res;

	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_file_create not mount"CRLF);
		return FALSE;
	}

	external_flash_set_file_name(file_name);

	res = ch378_createFile();
	//ulog_fmt("external_flash_file_create  createFile=%d"CRLF, res);

	res = external_flash_wait_status(USB_INT_SUCCESS, 1000); //РІРѕР·РІСЂ USB_INT_SUCCESS
	ulog_fmt("external_flash_file_create  createFile wait=%d"CRLF, res);

	return res;

}


//СЃРєРѕРїРёСЂРѕРІР°С‚СЊ С„Р°Р№Р» РёР· Р»РѕРєР°Р»СЊРЅРѕР№ РїР°РјСЏС‚Рё РІ FlashDisk
BOOL external_flash_copy_from_local(const char * file_name)
{

	FIL file;
	uint8_t buf[255], res;
	UINT br;
	uint64_t sz;

	if (!external_flash_is_mount) {
		ulog(CONM"external_flash_copy_from_local not mount"CRLF);
		return FALSE;
	}

	ulog_fmt(CONM"external_flash_copy_from_local(%s) begin(t:%lu)"CRLF, file_name, (unsigned long)HAL_GetTick());


	if ( f_open(&file, file_name, FA_READ) != FR_OK) {
		ulog_fmt(CONM"external_flash_copy_from_local error open 1 (%s)"CRLF, file_name);
		return FALSE;
	}

	res = external_flash_file_delete(file_name);
	//ulog_fmt("external_flash_copy_to  external_flash_file_delete=%d"CRLF, res);


	res = external_flash_file_create(file_name);
	//ulog_fmt("external_flash_copy_to  external_flash_file_create=%d"CRLF, res);


	sz = 0;
	while (f_read(&file, buf, sizeof(buf), &br) == FR_OK) {
		if (br == 0) break;
		res = external_flash_write_file(buf, br);
		HAL_IWDG_Refresh(&hiwdg2);
		if (!res) break;
		sz += br;
	}




	f_close(&file);
	external_flash_close_file(EXTERNAL_FLASH_CLOSE_AUTO_UPDATE);

	ulog_fmt(CONM"external_flash_copy_from_local(%s) end(t:%lu) sz:%s"CRLF, file_name, (unsigned long)HAL_GetTick(), ull2str(sz));

	return TRUE;
}

uint32_t  external_flash_test(const char * path)
{
	uint8_t buf[256], rd;
	uint32_t tick;

	tick = HAL_GetTick();
	external_flash_open_file(path);
	while ((rd = external_flash_read_file(buf, 255)) > 0) {
		HAL_IWDG_Refresh(&hiwdg2);
	}
	external_flash_close_file(EXTERNAL_FLASH_CLOSE_FORBID_UPDATE);
	return HAL_GetTick() - tick;
}
