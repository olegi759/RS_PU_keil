/*
 * logfile_cfg.h
 *
 *  Created on: 18 июн. 2025 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_LOGFILE_CFG_H_
#define CORE_INC_LOGFILE_CFG_H_

#include "setting.h"
#include "common.h"

#define LOG_FILE_NAME		"pu.log"
#define LOG_FILE_OLD_NAME	"pu.old.log"
#define LOG_FILE_MAX_SIZE	(setting.log_file_size * 1024)

#endif /* CORE_INC_LOGFILE_CFG_H_ */
