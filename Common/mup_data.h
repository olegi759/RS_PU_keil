/*
 * mup.h
 *
 *  Created on: 10 дек. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef COMMON_MUP_DATA_H_
#define COMMON_MUP_DATA_H_

//Протокол Информационного Взаимодействия (ПИВ) между ПУ и МУП

//Скорость 115200.
//UART 9 bit. девятый бит устанавливается только в первом байте.
//запрос: 0xEE/0xAA/0/80/0/0/0/0/0/0/ два байта суммы всех байт
//ответ: 0xAA/0xEE/0/80/аварии/вых реле/0/0/0/0/ два байта суммы всех байт

//ФЛАГИ АВАРИЙ
//авария 220
#define MUP_BIT_ERROR_220V		0
//авария источника
#define MUP_BIT_ERROR_PS		1
//авария акк
#define MUP_BIT_ERROR_ACC		2
//акк разряжен
#define MUP_BIT_DISCHARGE_ACC	3
//потеря емкости
#define MUP_BIT_CAPACITY_ACC	4
//авария цепей измерений
#define MUP_BIT_METERAGE		5
//авария тепература БП
#define MUP_BIT_TD_BP			6
//авария тепература акк
#define MUP_BIT_TD_ACC			7


//СЛСТОЯНИЕ РЕЛЕ (загрузка tpic)
//свет 220в
#define MUP_BIT_LED_220V				0
//свет акк
#define MUP_BIT_LED_ACC					1
//реле акк
#define MUP_BIT_RELAY_ACC				2
//вых 1
#define MUP_BIT_OUT1					3
//вых 2
#define MUP_BIT_OUT2					4
//шунтирующий транзистор
#define MUP_BIT_TRANSISTOR_FOR_DIODE	5

#endif /* COMMON_MUP_DATA_H_ */
