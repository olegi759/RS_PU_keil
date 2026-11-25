/*
 * adc_util.h
 *
 *  Created on: 2 сент. 2026 г.
 *      Author: Rafael Boltachev
 */

#ifndef UTIL_ADC_UTIL_H_
#define UTIL_ADC_UTIL_H_

#include "stm_util.h"


void adc12_get_min_max_value(uint16_t * adc_values, uint16_t adc_values_cnt, uint16_t * adc_min_value, uint16_t * adc_max_value);

#endif /* UTIL_ADC_UTIL_H_ */
