/*
 * opus_config.h
 *
 *  Created on: 10 янв. 2024 г.
 *      Author: Rafael Boltachev
 */

#ifndef OPUS_SRC_OPUS_CONFIG_H_
#define OPUS_SRC_OPUS_CONFIG_H_


#ifndef USE_ALLOCA
#define USE_ALLOCA	1
#endif

//#ifndef VAR_ARRAYS
//#define VAR_ARRAYS	1
//#endif

#ifndef DISABLE_FLOAT_API
#define DISABLE_FLOAT_API	1
#endif

#ifndef FIXED_POINT
#define FIXED_POINT	1
#endif


#ifndef OPUS_BUILD
#define OPUS_BUILD	1
#endif


//#ifndef celt_sig_assert
//#define celt_sig_assert(cond)
//#endif


#endif /* OPUS_SRC_OPUS_CONFIG_H_ */
