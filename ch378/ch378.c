/* ===============================================================
      Project: CH378S USB Read/Write Module testing ground
       Author: Scott C
      Created: 1st May 2015
  Arduino IDE: 1.6.2
      Website: http://arduinobasics.blogspot.com/p/arduino-basics-projects-page.html
  Description: This project will allow you to perform many of the functions available on the CH378S module.
               Checking connection to the module, putting the module into USB mode, resetting the module,
               reading, writing, appending text to files on the USB stick. This is very useful alternative to
               SD card modules, plus it doesn't need any libraries.
================================================================== */

#include "ch378.h"
#include "stm_util.h"
#include <string.h>
#include <str_util.h>


//1. Однокристальный микрокомпьютер генерирует выбор микросхемы SPI для микросхемы CH378, которая активна на низком уровне;
//2. Микроконтроллер отправляет байт данных в соответствии с режимом вывода SPI, а CH378 всегда выбирает первый байт, полученный после того, как выбор микросхемы SPI SCS становится действительным.
//	Байт рассматривается как код команды, а последующие байты рассматриваются как данные;
//3. Микроконтроллер опрашивает вывод BZ и ждет, пока интерфейс SPI CH378 перейдет в режим ожидания, или напрямую задерживает время TSC (около 1,5 мкс);
//4. Если это операция записи, однокристальный микрокомпьютер отправляет байт данных для записи в CH378 и ждет, пока интерфейс SPI станет свободным, а однокристальный микрокомпьютер продолжает
//	 Продолжайте отправлять несколько байтов данных для записи, и CH378 будет получать их последовательно до тех пор, пока выбор микросхемы SPI не будет запрещен микроконтроллером;
//5. Если это операция чтения, микроконтроллер получает один байт данных из CH378, ждет, пока интерфейс SPI освободится, а затем продолжает чтение из
//   CH378, получает несколько байтов данных, пока микроконтроллер не отключит выбор микросхемы SPI;
//6.  Микроконтроллер запрещает выбор микросхемы SPI CH378 для завершения текущей операции SPI.


#define CH378_CHIP_SELECT 			PIN_CLR(UCN_NSS)

#define CH378_CHIP_UNSELECT 		PIN_SET(UCN_NSS)
//#define CH378_GET_BZ 				PIN_GET(UCN_BS)
#define CH378_GET_INT 				PIN_GET(UCN_INT)

#define CH378_SCK(v) 				PIN_WRITE(UCN_SCK, v)
#define CH378_MISO 					PIN_GET(UCN_MISO)
#define CH378_MOSI(v) 				PIN_WRITE(UCN_MOSI, v)

//142 наносекунды
#define CH378_SCK_DELAY 			__NOP()
//#define CH378_SCK_DELAY 			HAL_Delay(1);

//#define CH378_BYTE_DELAY 			delay_us_cycle(1000)

//#define CH378_BYTE_DELAY 			HAL_Delay(5)

//#define CH378_BYTE_DELAY 			{int cnt = 1000; while (CH378_GET_BZ && cnt--) { __NOP(); }}

//#define CH378_BYTE_DELAY 			 while (CH378_GET_BZ)  __NOP()


#define CH378_BYTE_DELAY	delay_us_cycle(250)
//#define CH378_BYTE_DELAY	HAL_Delay(1);

#define CH378_INT_DELAY			delay_us_cycle(3)
//#define CH378_INT_DELAY		HAL_Delay(1)
//#define CH378_INT_DELAY		while (CH378_GET_INT) { __NOP(); }


void ch378_spi_WriteByte(uint8_t value)
{
	for (int i = 7; i >= 0 ; i--) {
	//for (int i = 0; i <=7 ; i++) {
		if (GETBIT(value, i)) CH378_MOSI(1);
		else CH378_MOSI(0);
		CH378_SCK(1);
		CH378_SCK_DELAY;
		CH378_SCK(0);
		CH378_SCK_DELAY;
	}


	CH378_BYTE_DELAY;
}


uint8_t ch378_spi_ReadByte(void)
{
	uint8_t value = 0;


	//for (int i = 0; i  <= 7; i++) {
	for (int i = 7; i >= 0; i--) {
		CH378_SCK(1);
		CH378_SCK_DELAY;
		if (CH378_MISO) SETBIT(value, i);
		else CLRBIT(value, i);
		CH378_SCK(0);
		CH378_SCK_DELAY;
	}

	CH378_BYTE_DELAY;

	return value;
}


void ch378_spi_transmit(uint8_t * tx, uint16_t  tx_len)
{
	if (!tx) return;

	for (uint16_t i = 0; i < tx_len; ++i) {
		ch378_spi_WriteByte(tx[i]);
	}

	//if (!tx_len) return;
	//char str[MAX_STR_SIZE];
	//strcpy_s(str, sizeof(str), "tx: ");
	//mem2hexstr(tx, tx_len, str + strlen(str), sizeof(str) - strlen(str), " ");
	//strcat_s(str, sizeof(str), CRLF);
	//ulog(str);
}


void ch378_spi_receive(uint8_t * rx, uint16_t rx_len)
{
	if (!rx) return;

	for (uint16_t i = 0; i < rx_len; ++i) {
		rx[i] = ch378_spi_ReadByte();
	}

	//if (!rx_len) return;
	//char str[MAX_STR_SIZE];
	//strcpy_s(str, sizeof(str), "rx: ");
	//mem2hexstr(rx, rx_len, str + strlen(str), sizeof(str) - strlen(str), " ");
	//strcat_s(str, sizeof(str), CRLF);
	//ulog(str);
}



void ch378_spi_transmit_and_receive(uint8_t * tx, uint16_t  tx_len, uint8_t * rx, uint16_t  rx_len)
{
	CH378_CHIP_SELECT;

	ch378_spi_transmit(tx, tx_len);
	CH378_INT_DELAY;
	ch378_spi_receive(rx, rx_len);

	CH378_CHIP_UNSELECT;
}


//------------------------------  ДЛЯ ИНИЦИАЛИЗАЦИИ И МОНТИРОВАНИЯ --------------------------------



// сброс чипа [hardware rest of the CH378]
void ch378_resetAll(void)
{
	uint8_t tx[1] = { CMD_RESET_ALL };

	CH378_CHIP_SELECT;
	ch378_spi_transmit(tx, sizeof(tx));
	CH378_CHIP_UNSELECT;
}


uint8_t ch378_getVersion(void)
{
	uint8_t tx[1] = {CMD_GET_IC_VER};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));
	return rx[0];
}


// check the communication between CH378 and MCU (вовращать должен инверсный in)
uint8_t ch378_checkExist(uint8_t in)
{
	uint8_t tx[2] = {CMD_CHECK_EXIST, in};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));
	return rx[0];
}

//если флэха воткнута в гнездо возвращает 0x15
//если флэха вытащена в гнездо возвращает 0x16
uint8_t ch378_getStatus(void)
{
	uint8_t tx[1] = {CMD_GET_STATUS};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));
	return rx[0];
}

// подключение USB Flash — ответ должен быть: 0x14 (USB_INT_SUCCESS) (значит всё хорошо, продолжаем);
uint8_t ch378_diskConnectionStatus(void)
{
	uint8_t tx[1] = {CMD_DISK_CONNECT};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}

// переключение в режим USB [set the USB mode] - ответ должен быть: 51 15 (значит всё хорошо, продолжаем);
//The code of 01H means switch to valid USB-DEVICE, peripheral firmware mode. (not support serial connection)
//The code of 02H means switch to valid USB-DEVICE, inner firmware mode.
//The code of 03H means switch to SD host mode, manage and storage/get file in SD card.
//The code of 04H means switch to invalid USB-HOST mode.
// 0x05: USB Host, non-generate SOF
// 0x06: USB Host, SOF auto
// 0x07: USB Host, reset USB bus
//должно возвращпть 0x51 (CMD_RET_SUCCESS) 0x15
uint8_t ch378_setUSBmode(uint8_t code)
{
	uint8_t tx[2] = {CMD_SET_USB_MODE, code};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}


//монтируем USB Flash (DISK_MOUNT) — ответ должен быть: 14 (значит всё хорошо, продолжаем);
uint8_t ch378_mount(void) // return ANSWSUCCESS or ANSW DISK DISCON USB_INT_SUCCESS
{
	uint8_t tx[1] = {CMD_DISK_MOUNT};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}

//------------------------------  ДЛЯ ЧТЕНИЯ --------------------------------

//возвращает USB_INT_SUCCESS
uint8_t ch378_openFile(void)
{
	uint8_t tx[1] = {CMD_FILE_OPEN};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}



uint8_t ch378_createFile(void)
{
	uint8_t tx[1] = {CMD_FILE_CREATE};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}


uint8_t ch378_deleteFile(void)
{
	uint8_t tx[1] = {CMD_FILE_ERASE};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}



//return USB_INT_SUCCESS
//update -  0x00 - without filesize update, 0x01 with filesize update
//Команда, используемая для закрытия файла или каталога, который был открыт. Нужны одни входные данные, чтобы уведомить
//обновлять ли длину файла, 0 означает запретить обновление длины файла, 1 означает автоматическое обновление длины файла

//После чтения/записи файла закройте файл. Для корневого каталога необходимо закрыть файл.
//Для обычной операции чтения файла закрытие файла необязательно. Для обычной операции записи файла закройте файл и выберите автоматическое обновление длины файла через CH378
uint8_t ch378_closeFile(uint8_t update)
{
	uint8_t tx[2] = {CMD_FILE_CLOSE, update};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}


//Команда, используемая для установки имени файла или каталога.
//Входные данные с 0 в конце, и длина менее 128 символов.
//Для файла с множеством подкаталогов разложите столько же подкаталогов и одно имя файла, задайте имя много раз и откройте из корневого каталога.
//Если при открытии файла возникла ошибка, вернитесь в корневой каталог, чтобы снова открыть
//Способ имени файла такой же, как и короткое имя файла в системе DOS, не нужен символ диска и
//двоеточие. «/» равнозначно «\», рекомендуется «/». Все символы являются заглавными буквами, цифрами или китайскими,
//и некоторыми специальными символами. Длина менее 11 символов, основное имя файла не более 8 символов, а имя файла расширения не более 3 символов. Если есть имя файла расширения, используйте десятичную
//двузначную дробь, чтобы отличить. Проконсультируйтесь с EXAM11
//Ни одного символа в строке (конечный символ — 0, следующий — то же самое), инициализировать файловую систему, не открывать файл;
//Только один «/» или «\», открыть корневой каталог;
//Первый символ — «/» или «\», а следующий — имя файла, файл в корневом каталоге;
//Имя файла как символ, означает файл в текущем каталоге.
//Например, для FILENAME.EXT в корневом каталоге используйте «/FILENAME.EXT\0», чтобы задать, 14 символов в
//всего, «\0» означает 0 на языке C, как конечный символ. «/» означает в корневом каталоге. Используйте «\\» как корневой каталог в языке C.
//Другой, файл имеет три подкаталога \YEAR2004\MONTH05.NEW\DATE18\ADC.TXT, открываются следующим образом:
//① используйте «/YEAR2004\0» для установки имени файла (имя каталога), используйте CMD_FILE_OPEN для открытия первого каталога;
//② используйте «MONTH05.NEW\0» для установки имени файла (имя каталога), используйте CMD_FILE_OPEN для открытия второго каталога;
//③ используйте «DATE18\0» для установки имени файла (имя каталога), используйте CMD_FILE_OPEN для открытия третьего каталога;
//④ используйте «ADC.TXT\0» для установки имени файла (имя каталога), используйте CMD_FILE_OPEN для открытия конечного файла;



//path с 0 в конце и длиной менее 14 символов.
void ch378_setFileName(const char * path)
{
	uint8_t tx[1 + 128 + 1] = {0};
	uint16_t i;
	uint16_t len;

	len = strlen(path);
	if (len > 128) len = 128;

	tx[0] = CMD_SET_FILE_NAME;
	for (i = 0; i < len; ++i) {
		tx[i + 1] = *(uint8_t *)&path[i];
	}
	tx[len + 1] = 0;


	CH378_CHIP_SELECT;
	ch378_spi_transmit(tx, len + 2);
	CH378_CHIP_UNSELECT;
}

uint32_t ch378_getFileSize(void)
{
	uint8_t tx[2] = {CMD_GET_FILE_SIZE, 0x68};
	uint8_t rx[4] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));


	return (rx[0] | (rx[1] << 8) | (rx[2] << 16) | (rx[3] << 24));
}

//Команда CMD_BYTE_READ используется для считывания блока данных по байтам с текущей позиции
//Микроконтроллер посылает команду CMD_BYTE_READ и вводит запрашиваемое количество байт, начинается чтение;
//CH378 рассчитывает оставшуюся длину файла от текущего указателя до конца файла, и на основе запрошенного
//количества байт, оставшейся длины и состояния внутреннего буфера вычисляет разрешенное количество байт для
//чтения в этот раз, читает данные файла во внутренний буфер и перемещает указатель файла, затем прерывает
//внешнюю микроконтроллерную систему; статус прерывания — ERR_SUCCESS.
//Микроконтроллер с помощью отправки команды CMD_GET_REAL_LEN быстро получает фактическую длину, разрешенную для чтения в этот раз;
//Микроконтроллер может считывать фактические данные из внутреннего буфера CH378 командой
//CMD_RD_HOST_OFS_DATA или CMD_RD_HOST_CUR_DATA. При достаточной памяти RAM можно считать все данные
//целиком за один раз. При меньшем объеме RAM можно считывать данные частями, обрабатывая по мере чтения.

uint16_t ch378_byteRead(uint16_t size)
{
	uint8_t tx[3] = {CMD_BYTE_READ, (size & 0xFF), (size >> 8)};
	uint8_t rx[2] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return (rx[1] << 8) | rx[0];

}


uint16_t ch378_byteWrite(uint16_t size)
{
	uint8_t tx[3] = {CMD_BYTE_WRITE, (size & 0xFF), (size >> 8)};
	uint8_t rx[2] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return (rx[1] << 8) | rx[0];
}


//Команда используется для чтения блока данных из текущего местоположения в байтах, CMD_BYTE_RD_GO используется для
//DataSheet CH378 (первый) 15
//продолжения операции чтения байтов. Чтение успешно, CH378 автоматически синхронизирует указатель перемещения файла, поэтому
//следующая операция чтения/записи может следовать за этим местоположением. Для этой команды необходимо ввести номер байта запроса на чтение,
//запрос основан на младшем байте впереди (Little-Endian) двух байтов 16-битных данных
//Полное чтение байта обычно начинается с команды CMD_BYTE_READ, состоящей из множества уведомлений о прерывании, множества прочтений блоков данных и нескольких CMD_BYTE_RD_GO. Полные шаги чтения байта следующие:
//1 открыть файл и подтвердить в соответствующем месте (указатель файла)
//2 MCU передает CMD_BYTE_READ и вводит номер байта, который является запросом, начинает операцию чтения;
//3 CH378 вычисляет свободную длину от текущего указателя файла до конца местоположения. Если текущий
//указатель файла находится в конце файла или свободная длина равна 0, то завершает операцию чтения и прерывает
//уведомление MCU, статус прерывания USB_INT_SUCCESS. В противном случае вычисляет количество байтов, которые
//разрешено читать через номер байта запроса, оставшуюся длину файла, внутренний буфер и вычитает число
//из номера байта запроса, чтобы получить оставшийся номер байта запроса, одновременно перемещает указатель файла, затем уведомляет MCU, статус прерывания USB_INT_DISK_READ;
//4 Анализ состояния прерывания MCU, если это USB_INT_DISK_READ, читает блок данных через
//CMD_RD_USB_DATA0 и продолжает; если это USB_INT_SUCCESS, переходите к шагу 6;
//5 MCU передает CMD_BYTE_RD_GO для приказа CH378 продолжить чтение, CH378 автоматически переходит к шагу 3;
//6 файл заканчивается или запрошенные байты чтения были прочитаны, вся операция чтения завершена.
//MCU будет прервана несколько раз, чтобы получить уведомление о длине блока данных и подсчитать длину нескольких блоков данных, сравнить сумму с запрошенным номером байта,
//если сумма меньше запрошенного номера байта, это означает, что указатель достиг конца.

//возвращает USB_INT_SUCCESS или USB_INT_DISK_READ
uint8_t ch378_byteReadGo(void)
{
	uint8_t tx[1] = {CMD_BYTE_RD_GO};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}



uint8_t ch378_byteWrGo(void)
{
	uint8_t tx[1] = {CMD_BYTE_WR_GO};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}


uint8_t ch378_readData0(uint8_t * data)
{
	uint8_t tx[1] = {CMD_RD_HOST_REQ_DATA};
	uint8_t rx[1] = {0};

/*
	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	if (rx[0]) {
		CH378_CHIP_SELECT;
		ch378_spi_receive(data, rx[0]);
		CH378_CHIP_UNSELECT;
	}
*/


	CH378_CHIP_SELECT;

	ch378_spi_transmit(tx, sizeof(tx));
	CH378_INT_DELAY;
	ch378_spi_receive(rx, sizeof(rx));
	if (rx[0]) ch378_spi_receive(data, rx[0]);

	CH378_CHIP_UNSELECT;



	return rx[0];
}

//Эта команда используется для чтения запрошенного блока данных из внутреннего буфера начиная с текущего смещения. Сначала считывается длина
//блока данных в 2 байтах, то есть количество байтов последующего потока данных, затем чтение последующих данных блоками по мере указанной длины.
uint16_t ch378_rd_host_req_data(uint8_t * data)
{
	uint8_t tx[1] = {CMD_RD_HOST_REQ_DATA};
	uint8_t rx[2] = {0};
	uint16_t len = 0;


	CH378_CHIP_SELECT;

	ch378_spi_transmit(tx, sizeof(tx));
	CH378_INT_DELAY;
	ch378_spi_receive(rx, sizeof(rx));
	len = (rx[1] << 8) | rx[0];
	if (len) ch378_spi_receive(data, len);

	CH378_CHIP_UNSELECT;

	return len;
}

//Эта команда предназначена для чтения блока данных текущего смещения внутри заданного буфера. Сначала записываются 2 байта длины блока данных
//для чтения, то есть количество байт последующего потока, затем считываются данные последовательно в соответствии с длиной.
void ch378_rd_host_cur_data(uint8_t * data, uint16_t size)
{
	uint8_t tx[3] = {CMD_RD_HOST_CUR_DATA, (size & 0xFF), (size >> 8)};


	ch378_spi_transmit_and_receive(tx, sizeof(tx), data, size);
}

//Эта команда записывает блок данных в текущее смещение внутреннего буфера. Сначала записывается 2 байта длины блока
//данных для ввода, то есть количество байтов последующего потока, затем по длине вводятся последующие данные.
void ch378_wr_host_cur_data(uint8_t * data, uint16_t size)
{
	uint8_t tx[3] = {CMD_WR_HOST_CUR_DATA, (size & 0xFF), (size >> 8)};


	CH378_CHIP_SELECT;

	ch378_spi_transmit(tx, sizeof(tx));
	CH378_INT_DELAY;
	ch378_spi_transmit(data, size);

	CH378_CHIP_UNSELECT;
}



uint8_t ch378_writeReqData(uint8_t * data, uint8_t cnt)
{
	uint8_t tx[1] = {CMD_WR_HOST_OFS_DATA};
	uint8_t rx[1] = {0};



	CH378_CHIP_SELECT;

	ch378_spi_transmit(tx, sizeof(tx));
	CH378_INT_DELAY;
	ch378_spi_receive(rx, sizeof(rx));
	if (rx[0]) {
		ch378_spi_transmit(data, rx[0]);
		CH378_INT_DELAY;
	}


	CH378_CHIP_UNSELECT;



	return rx[0];
}




//возвращает USB_INT_SUCCESS
uint8_t ch378_diskQuery(void)
{
	uint8_t tx[1] = {CMD_DISK_QUERY};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}



//возвращает USB_INT_DISK_READ и ERR_MISS_FILE
uint8_t ch378_fileEnumGo(void)
{

	uint8_t tx[1] = {CMD_FILE_ENUM_GO};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}

//возвращает USB_INT_SUCCESS
uint8_t ch378_filePosition(uint32_t Position)
{

	uint8_t tx[5] = {CMD_BYTE_LOCATE, Position & 0xFF, (Position >> 8) & 0xFF, (Position >> 16) & 0xFF, (Position >> 24) & 0xFF};
	uint8_t rx[1] = {0};

	ch378_spi_transmit_and_receive(tx, sizeof(tx), rx, sizeof(rx));

	return rx[0];
}



//Команда используется для установки режима прерывания вывода SD0 интерфейса SPI. Сначала введите 16H, затем введите
//новый режим прерывания. Существует два режима прерывания: 10H означает запрет использования SD0 в качестве выхода прерывания, когда
//выбор микросхемы SCS неактивен, это запрет выхода с тремя состояниями, поэтому легко использовать шину SPI SPI с другим устройством;
//90H устанавливает SD0 в качестве состояния выхода, когда выбор микросхемы SCS неактивен, его можно использовать в качестве выхода прерывания, равнозначно
//выводу INT#, MCU может запрашивать состояние запроса прерывания.

void ch378_setSd0Int(uint8_t mode)
{
	uint8_t tx[3] = {CMD_SET_SD0_INT, 0x16, mode};

	CH378_CHIP_SELECT;
	ch378_spi_transmit(tx, sizeof(tx));
	CH378_CHIP_UNSELECT;
}

/*
enum fileProcessENUM { // for file read/write state machine
	REQUEST,
	NEXT,
	READWRITE,
	DONE
};



uint8_t ch378_listDir(const char* path)
{
// __________________________________________________________________________________________________________
// | 00 - 07 | 08 - 0A |  	0B     |     0C    |     0D     | 0E  -  0F | 10  -  11 | 12 - 13|  14 - 15 |
// |Filename |Extension|File attrib|User attrib|First ch del|Create time|Create date|Owner ID|Acc rights|
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// | 16 - 17 | 18 - 19 |   1A - 1B   |  1C  -  1F |
// |Mod. time|Mod. date|Start cluster|  File size |
//
// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
// http://www.tavi.co.uk/phobos/fat.html
//

	BOOL moreFiles = TRUE;  // more files waiting for read out
	BOOL doneFiles = FALSE; // done with reading a file
	uint32_t tmOutCnt = HAL_GetTick();
	enum fileProcessENUM fileProcesSTM = REQUEST;
	uint8_t _answer = 0;
	uint8_t buf[256] = {0};

	while (!doneFiles) {
		if (HAL_GetTick() - tmOutCnt >= ANSWTIMEOUT) return CH378_ERR_TIMEOUT;
		switch (fileProcesSTM) {
			case REQUEST:
				ch378_setFileName(path);
				_answer = ch378_openFile();
				//_fileWrite = 2; // if in subdir
				fileProcesSTM = READWRITE;
				break;
			case READWRITE:
				if(_answer == ANSW_ERR_MISS_FILE){
					fileProcesSTM = DONE;
					moreFiles = FALSE;// no more files in the directory
				}//end if
				if(_answer == USB_INT_DISK_READ){
					ch378_readData0(buf); // read data to fatInfo buffer
					buf[8] = 0;
					ulog((char *)buf);
					ulog(CRLF);
					fileProcesSTM = NEXT;
				}
				break;
			case NEXT:
				_answer = ch378_fileEnumGo(); // go for the next filename
				fileProcesSTM = DONE;
				break;
			case DONE:
				if(!moreFiles){
					//closeFile(); // if no more files in the directory, close the file
					//closing file is not required after print dir (return value was always 0xB4 File is closed)
					fileProcesSTM = REQUEST;
				} else {
					fileProcesSTM = READWRITE;
				}
				doneFiles = TRUE;
				break;
		}// end switch
	}//end while
	return moreFiles;
}


//возвращает кол-во байт
uint8_t ch378_diskInfo(uint8_t * data)
{
	if (ch378_diskQuery() != USB_INT_SUCCESS) return 0;
	return ch378_readData0(data);
}

//------------------------------  ДЛЯ ЗАПИСИ --------------------------------


uint8_t ch378_deleteDir(void)
{
	uint8_t res;


	CH378_CHIP_SELECT;
	ch378_spi_transmit_receive(CMD_FILE_ERASE);
	res = ch378_waitForResponse();
	CH378_CHIP_UNSELECT;

	//ch378_cd("/",0);
	return res;
}
uint8_t ch378_saveFileAttrb(void)
{
	uint8_t res;

	CH378_CHIP_SELECT;
	ch378_spi_transmit_receive(CMD_DIR_INFO_READ);
	ch378_spi_transmit_receive(0xFF);
	res = ch378_waitForResponse();
	CH378_CHIP_UNSELECT;


	//ch378_writeFatData();//send fat data

	CH378_CHIP_SELECT;
	ch378_spi_transmit_receive(CMD_DIR_INFO_SAVE);
	res = ch378_waitForResponse();
	CH378_CHIP_UNSELECT;

	return res;
}

uint8_t ch378_deleteFile(void)
{
	uint8_t res;

	ch378_openFile();

	CH378_CHIP_SELECT;
	ch378_spi_transmit_receive(CMD_FILE_ERASE);
	res = ch378_waitForResponse();
	CH378_CHIP_UNSELECT;

	//ch378_cd("/",0);
	return res;
}

uint8_t ch378_dirCreate(void)
{
	uint8_t res;

	CH378_CHIP_SELECT;
	ch378_spi_transmit_receive(CMD_DIR_CREATE);
	res = ch378_waitForResponse();
	CH378_CHIP_UNSELECT;

	return res;
}


uint8_t ch378_cd(const char* dirPath, BOOL mkDir)
{
	uint8_t res = 0;
	uint8_t pathLen = strlen(dirPath);
	char input[pathLen + 1];
	char* command;
	uint8_t _dirDepth;


	if (pathLen >= ((MAXDIRDEPTH*8)+(MAXDIRDEPTH+1)) ) return CH378_ERR_LONGFILENAME;

	_dirDepth = 0;
	strcpy(input, dirPath);
	ch378_setFileName("/");
	res = ch378_openFile();
	command = strtok(input, "/");//split path into tokens
	while (command != NULL) {
		 if (strlen(command) > 8) { //if a dir name is longer than 8 char
			 res = CH378_ERR_LONGFILENAME;
			 break;
		 }
		 ch378_setFileName(command);
		 res = ch378_openFile();
		 if (res == USB_INT_SUCCESS) {//if file already exist with this name
			 res = ANSW_ERR_FOUND_NAME;
			 ch378_closeFile(0x01);
			 break;
		 } else if(mkDir && (res == ANSW_ERR_MISS_FILE)){
			 res = ch378_dirCreate();
			 if(res != USB_INT_SUCCESS) break;
		 }//end if file exist

		 _dirDepth++;

		 command = strtok (NULL, "/");
	 }
	return res;
}
*/
