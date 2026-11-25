/*
 * fault_data.h
 *
 *  Created on: 7 мая 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef FAULT_DATA_H_
#define FAULT_DATA_H_

#include "main.h"


//авария 220
#define FAULT_MUP_220V				0
//авария источника
#define FAULT_MUP_ERROR_PS			1
//авария акк
#define FAULT_MUP_ERROR_ACC			2
//акк разряжен
#define FAULT_MUP_DISCHARGE_ACC		3
//потеря емкости
#define FAULT_MUP_CAPACITY_ACC		4
//авария цепей измерений
#define FAULT_MUP_METERAGE			5
//авария тепература БП
#define FAULT_MUP_TD_BP				6
//авария тепература акк
#define FAULT_MUP_TD_ACC			7


#define FAULT_MKK_INSIG_1			8
#define FAULT_MKK_INSIG_2			9
#define FAULT_MKK_INSIG_3			10
#define FAULT_MKK_INSIG_4			11
#define FAULT_MKK_INSIG_5			12
#define FAULT_MKK_INSIG_6			13
#define FAULT_MKK_INSIG_7			14
#define FAULT_MKK_INSIG_8			15
#define FAULT_MKK_INSIG_9			16
#define FAULT_MKK_INSIG_10			17
#define FAULT_MKK_INSIG_11			18
#define FAULT_MKK_INSIG_12			19

#define FAULT_MKK_INSIG_13			20
#define FAULT_MKK_INSIG_14			21
#define FAULT_MKK_INSIG_15			22
#define FAULT_MKK_INSIG_16			23
#define FAULT_MKK_INSIG_17			24
#define FAULT_MKK_INSIG_18			25
#define FAULT_MKK_INSIG_19			26
#define FAULT_MKK_INSIG_20			27
#define FAULT_MKK_INSIG_21			28
#define FAULT_MKK_INSIG_22			29
#define FAULT_MKK_INSIG_23			30
#define FAULT_MKK_INSIG_24			31


#define FAULT_MKK_SOUND_1			32
#define FAULT_MKK_SOUND_2			33
#define FAULT_MKK_SOUND_3			34
#define FAULT_MKK_SOUND_4			35
#define FAULT_MKK_SOUND_5			36
#define FAULT_MKK_SOUND_6			37
#define FAULT_MKK_SOUND_7			38
#define FAULT_MKK_SOUND_8			39
#define FAULT_MKK_SOUND_9			40
#define FAULT_MKK_SOUND_10			41
#define FAULT_MKK_SOUND_11			42
#define FAULT_MKK_SOUND_12			43

#define FAULT_MKK_SOUND_13			44
#define FAULT_MKK_SOUND_14			45
#define FAULT_MKK_SOUND_15			46
#define FAULT_MKK_SOUND_16			47
#define FAULT_MKK_SOUND_17			48
#define FAULT_MKK_SOUND_18			49
#define FAULT_MKK_SOUND_19			50
#define FAULT_MKK_SOUND_20			51
#define FAULT_MKK_SOUND_21			52
#define FAULT_MKK_SOUND_22			53
#define FAULT_MKK_SOUND_23			54
#define FAULT_MKK_SOUND_24			55


#define FAULT_MKK_START_1			56
#define FAULT_MKK_START_2			57

#define FAULT_MKK_DEVICE_1			58
#define FAULT_MKK_DEVICE_2			59

#define FAULT_PD_CON_1				60
#define FAULT_PD_CON_2				61

#define FAULT_UM_CON_1				62
#define FAULT_UM_CON_2				63
#define FAULT_UM_CON_3				64
#define FAULT_UM_CON_4				65
#define FAULT_UM_CON_5				66
#define FAULT_UM_CON_6				67


#define FAULT_UM_PROTECT_CH1_1		68
#define FAULT_UM_PROTECT_CH1_2		69
#define FAULT_UM_PROTECT_CH1_3		70
#define FAULT_UM_PROTECT_CH1_4		71
#define FAULT_UM_PROTECT_CH1_5		72
#define FAULT_UM_PROTECT_CH1_6		73

#define FAULT_UM_PROTECT_CH2_1		74
#define FAULT_UM_PROTECT_CH2_2		75
#define FAULT_UM_PROTECT_CH2_3		76
#define FAULT_UM_PROTECT_CH2_4		77
#define FAULT_UM_PROTECT_CH2_5		78
#define FAULT_UM_PROTECT_CH2_6		79

#define FAULT_UM_BATTERY_1			80
#define FAULT_UM_BATTERY_2			81
#define FAULT_UM_BATTERY_3			82
#define FAULT_UM_BATTERY_4			83
#define FAULT_UM_BATTERY_5			84
#define FAULT_UM_BATTERY_6			85

#define FAULT_UM_LINE_OUTAGE_1		86
#define FAULT_UM_LINE_OUTAGE_2		87
#define FAULT_UM_LINE_OUTAGE_3		88
#define FAULT_UM_LINE_OUTAGE_4		89
#define FAULT_UM_LINE_OUTAGE_5		90
#define FAULT_UM_LINE_OUTAGE_6		91

#define FAULT_UM_CH1_OUTAGE_1		92
#define FAULT_UM_CH1_OUTAGE_2		93
#define FAULT_UM_CH1_OUTAGE_3		94
#define FAULT_UM_CH1_OUTAGE_4		95
#define FAULT_UM_CH1_OUTAGE_5		96
#define FAULT_UM_CH1_OUTAGE_6		97

#define FAULT_UM_CH2_OUTAGE_1		98
#define FAULT_UM_CH2_OUTAGE_2		99
#define FAULT_UM_CH2_OUTAGE_3		100
#define FAULT_UM_CH2_OUTAGE_4		101
#define FAULT_UM_CH2_OUTAGE_5		102
#define FAULT_UM_CH2_OUTAGE_6		103

#define FAULT_UM_TEST_LINE1_1		104
#define FAULT_UM_TEST_LINE1_2		105
#define FAULT_UM_TEST_LINE1_3		106
#define FAULT_UM_TEST_LINE1_4		107
#define FAULT_UM_TEST_LINE1_5		108
#define FAULT_UM_TEST_LINE1_6		109

#define FAULT_UM_TEST_LINE2_1		110
#define FAULT_UM_TEST_LINE3_2		111
#define FAULT_UM_TEST_LINE2_3		112
#define FAULT_UM_TEST_LINE2_4		113
#define FAULT_UM_TEST_LINE2_5		114
#define FAULT_UM_TEST_LINE2_6		115

#define FAULT_PD_RING				116
#define FAULT_PU_RING_CMD			117
#define FAULT_PU_RING_AUDIO			118

#define FAULT_ZASH_UM1				119
#define FAULT_PER_UM1				120

#define FAULT_ZASH_UM2				121
#define FAULT_PER_UM2				122

#define FAULT_FATFS_NOT_MOUNT		123
#define FAULT_MKI_NOT_RECV			124

#define FAULT_MKK1_NOT_RECV			125
#define FAULT_MKK2_NOT_RECV			126

#define MKK_INSIG_FAULT		"НЕИСП. ВХ. СИГН. "
#define MKK_SOUND_FAULT 	"НЕИСП. ЛИН. ОПОВ. "
#define MKK_START_FAULT		"НЕИСП. ЛИН. ВНЕШ. "
#define MKK_DEVICE_FAULT 	"НЕИСП. ПРИБ. ВНЕШ. "
#define PD_CON_FAULT		"СОЕД. С ПД-"
#define UN_CON_FAULT		"СОЕД. С УМ-"
#define UM_PROTECT_CH1		"ЗАЩИТА 1 КАНАЛА"
#define UM_PROTECT_CH2		"ЗАЩИТА 2 КАНАЛА"
#define UM_BATTERY			"МОД. ПИТАНИЯ"
#define UM_LINE_OUTAGE		"АНАЛОГ. ЛИНИЯ"
#define UM_CH1_OUTAGE		"1 ЛИНИИ ВЕЩ."
#define UM_CH2_OUTAGE		"2 ЛИНИИ ВЕЩ."
#define UM_TEST_LINE1		"ТЕСТ ЛИНИИ 1"
#define UM_TEST_LINE2		"ТЕСТ ЛИНИИ 2"


#define MUP_BIT_ERROR_220V_STR		"Наличие сети 230В"
#define MUP_BIT_ERROR_PS_STR		"Авария источника"
#define MUP_BIT_ERROR_ACC_STR		"АКБ не подключена"
#define MUP_BIT_DISCHARGE_ACC_STR	"АКБ разряжен"
#define MUP_BIT_CAPACITY_ACC_STR	"Потеря емкости АКБ"
#define MUP_BIT_METERAGE_STR		"Авария цепей измерений"
#define MUP_BIT_TD_BP_STR			"Перегрев БП"
#define MUP_BIT_TD_ACC_STR			"Авария тепература акк"



//void get_fault_list(char * buf, uint16_t buf_size);

//extern uint8_t faults_small;


#endif /* COMMON_FAULT_DATA_H_ */
