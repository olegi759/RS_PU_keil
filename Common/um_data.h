/*
 * um_data.h
 *
 *  Created on: 12 авг. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_UM_DATA_H_
#define COMMON_UM_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


#pragma pack(push, 1)


//Протокол Информационного Взаимодействия (ПИВ) между ПУ и УМ в общем виде
//[uint8_t Адрес кому][uint8_t Адрес от кого][uint16_t длина данных][....данные ....][uint16_t контрольная сумма]
//Данные протокола описаны ниже


//Передача информации о состоянии усилителя:
//[uint8_t UM_CMD_STATE]
#define UM_CMD_STATE        0x10

//Ответ от УМ с информацией о состоянии усилителя:
//[uint8_t UM_CMD_STATE][uint8_t РЕЖИМ ПЕРВОГО КАНАЛА)][uint8_t РЕЖИМ ВТОРОГО КАНАЛА][uint8_t ОШИБКИ УМ]

typedef enum
{
    CH_MODE_STANDBY     = 0x00,
    CH_MODE_TRANSLATION = 0x01
} ChannelMode_e;


#define UM_BIT_PROTECT_CH1		0
#define UM_BIT_PROTECT_CH2		1
#define UM_BIT_BATTERY			2
#define UM_BIT_LINE_OUTAGE		3
#define UM_BIT_CH1_OUTAGE		4
#define UM_BIT_CH2_OUTAGE		5

// СОСТОЯНИЕ НЕИСПРАВНОСТЕЙ УСИЛИТЕЛЯ (флаги неисправностей)
typedef struct
{
    uint8_t dm_protect_ch1 :1; // Защита первого канала
    uint8_t dm_protect_ch2 :1; // Защита второго канала
    uint8_t dm_battery     :1; // Неисправность по модулю питания
    uint8_t dm_line_outage :1; // Неисправность аналоговой линии
    uint8_t dm_ch1_outage  :1; // Неисправность 1й линии вещания
    uint8_t dm_ch2_outage  :1; // Неисправность 2й линии вещания

    uint8_t res :2;
} DeviceMistake_t;


//Выбор режима работы каналов:
//[uint8_t UM_CMD_MODES][uint8_t РЕЖИМ ПЕРВОГО КАНАЛА][uint8_t РЕЖИМ ВТОРОГО КАНАЛА]
#define	UM_CMD_MODES        0x11

//Ответ от УМ на выбор режимов работы каналов:
//[uint8_t UM_CMD_MODES]



//Тест аналоговой линии
//[uint8_t UM_CMD_TEST_LINE]
#define UM_CMD_TEST_LINE   0x12

//Ответ от УМ на тест аналоговой линии
//[uint8_t UM_CMD_TEST_LINE][uint8_t РЕЗУЛЬТАТ ТЕСТА][uint8_t РЕЗУЛЬТАТ ТЕСТА]

typedef enum
{
    TEST_FAILURE = 0x00,
    TEST_SUCCESS = 0x01
}TestResult_e;


//Установка адреса:
//[uint8_t UM_CMD_SET_ADDRESS][uint8_t АДРЕС УМ]
#define	UM_CMD_SET_ADDRESS        0x13

//Ответ от УМ на установка адреса
//[uint8_t UM_CMD_SET_ADDRESS]

#pragma pack(pop)


#ifdef __cplusplus
}
#endif


#endif /* COMMON_UM_DATA_H_ */
