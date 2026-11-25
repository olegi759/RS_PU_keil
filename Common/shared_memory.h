/*
 * shared_memory.h
 *
 *  Created on: 22 апр. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_SHARED_MEMORY_H_
#define COMMON_SHARED_MEMORY_H_

#include "def_util.h"
#include "ringbuff_util.h"
#include "setting_data.h"

//всего разделяемой памяти 64 кб
#define SM_BASE_ADDRESS				 0x38000000

//размер данных для буффера обмена
#define SM_RINGBUFF_SIZE		(30 * 1024)
//размер максимального пакета (должен быть больше sizeof(setting))
#define SM_MAX_PACKET_SIZE		(sizeof(setting_t) + 100)
//#define SM_MAX_LOCK_PTR		10

typedef struct _shared_memory {

	//комманды от CM4 к CM7
	ringbuff_t rb_cm4_to_cm7;
	uint8_t rb_cm4_to_cm7_data[SM_RINGBUFF_SIZE];

	//комманды от CM7 к CM4
	ringbuff_t rb_cm7_to_cm4;
	uint8_t rb_cm7_to_cm4_data[SM_RINGBUFF_SIZE];

} shared_memory_t;

//запрос версии -> CM7
#define CORE_CMD_GET_VERSION					1
//добавить в лог -> в CM4
#define CORE_CMD_ADD_LOG						2
//включить линии -> в СМ4
#define CORE_CMD_SET_ENABLE_LINES				3
//обновление RTC -> в СМ7
#define CORE_CMD_RTC_UPDATE						4


//приняли данные из консоли -> в CM7
#define CORE_CMD_CONSOLE_RCV					11
//передача данных в консоль -> в СМ4
#define CORE_CMD_CONSOLE_SND					12

//У МКИ нажалась кнопка  -> в СМ7
#define CORE_CMD_MKI_BUTTON_PRESS				21
//У МКИ отжалась кнопка  -> в СМ7
#define CORE_CMD_MKI_BUTTON_UNPRESS				22
//У МКИ установить дисплей -> в СМ4
#define CORE_CMD_MKI_SET_DISPLAY				23
//включить ТЕСТ на МКИ -> в СМ4
#define CORE_CMD_MKI_TEST						24
//установить 1 LED -> в СМ4
#define CORE_CMD_MKI_SET_LED					25
//передать измененные LED-ы на МКИ -> в СМ4
#define CORE_CMD_MKI_SEND_NEED_LEDS				26
//включить светодиоды из линий -> в СМ4
#define CORE_CMD_MKI_PAINT_LED_FROM_LINES		27


//установить ошибку -> в СМ4
#define CORE_CMD_FAULT_SET						31
//снять ошибку -> в СМ4
#define CORE_CMD_FAULT_UNSET					32

//открыть файл (параметр имя файла)  -> в СМ4
#define CORE_CMD_OPEN_FILE						41
//прочитать днные (параметр размер данных)  -> в СМ4
#define CORE_CMD_READ_FILE						42
//закрвть файл -> в СМ4
#define CORE_CMD_CLOSE_FILE						43
//есть ли файл -> в СМ4
#define CORE_CMD_FILE_EXIST						44
//список опус файлов  -> в СМ4
#define CORE_CMD_GET_OPUS_LIST					45
//список опус и мп3 файлов  -> в СМ4
#define CORE_CMD_GET_MP3_OPUS_LIST				46

//оправить на пд -> в СМ4
#define CORE_CMD_PD_SEND						51
//пришли данные с пд -> в СМ7
#define CORE_CMD_PD_RECV						52
#define CORE_CMD_PD_I2C_RECV					53	/* ответ ПД по линии I2C2 (CM4 -> CM7) */
#define CORE_CMD_PD_I2C_SEND					54	/* пакет ПД на линию I2C2 (CM7 -> CM4) */





//обновить setting -> в СМ7
#define CORE_CMD_CHANGE_SETTING					61
//сменилась опция
#define CORE_CMD_CHANGE_OPTION					62
//сменилось pu_mode -> в СМ4
#define CORE_CMD_CHANGE_MODE					63
//сменились faults -> СМ7
#define CORE_CMD_CHANGE_FAULTS					64
//сменились мкк данные -> CM7
#define CORE_CMD_CHANGE_MKK_INSIG_LIN			65
//сменился пин код -> в СМ4
#define CORE_CMD_CHANGE_SAFETY_PINCODE_CM4		66
//передача UID от CM7 -> в CM4
#define CORE_CMD_UID							67
//ping от CM7 -> в CM4
#define CORE_CMD_PING							68
//TEST от CM7 -> в CM4
#define CORE_CMD_UM_TEST						69
//сменился пин код -> в СМ7
#define CORE_CMD_CHANGE_SAFETY_PINCODE_CM7		70

#define CORE_CMD_RESULT							0x80


#define CORE_CMD_GET_VERSION_RESULT				(CORE_CMD_RESULT | CORE_CMD_GET_VERSION)
#define CORE_CMD_OPEN_FILE_RESULT				(CORE_CMD_RESULT | CORE_CMD_OPEN_FILE)
#define CORE_CMD_READ_FILE_RESULT				(CORE_CMD_RESULT | CORE_CMD_READ_FILE)
#define CORE_CMD_GET_OPUS_LIST_RESULT			(CORE_CMD_RESULT | CORE_CMD_GET_OPUS_LIST)
#define CORE_CMD_FILE_EXIST_RESULT				(CORE_CMD_RESULT | CORE_CMD_FILE_EXIST)
#define	CORE_CMD_SET_ENABLE_LINES_RESULT		(CORE_CMD_RESULT | CORE_CMD_SET_ENABLE_LINES)
#define CORE_CMD_GET_MP3_OPUS_LIST_RESULT		(CORE_CMD_RESULT | CORE_CMD_GET_MP3_OPUS_LIST)

/* Define semaphores */
#define HSEM_TAKE_RELEASE(_id_)             do { HAL_HSEM_FastTake((_id_)); HAL_HSEM_Release((_id_), 0); } while (0)
#define HSEM_WAKEUP_CPU2                    0
#define HSEM_WAKEUP_CPU2_MASK               __HAL_HSEM_SEMID_TO_MASK(HSEM_WAKEUP_CPU2)
#define HSEM_CM4_TO_CM7                     1
#define HSEM_CM4_TO_CM7_MASK                __HAL_HSEM_SEMID_TO_MASK(HSEM_CM4_TO_CM7)
#define HSEM_CM7_TO_CM4                     2
#define HSEM_CM7_TO_CM4_MASK                __HAL_HSEM_SEMID_TO_MASK(HSEM_CM7_TO_CM4)

/* Flags management */
#define WAIT_COND_WITH_TIMEOUT(c, t)        do {        \
    int32_t timeout = (int32_t)(t);                     \
    while ((c) && timeout-- > 0) {}                     \
    if (timeout < 0) {                                  \
        Error_Handler();                                \
    }                                                   \
} while (0)




#endif /* COMMON_SHARED_MEMORY_H_ */
