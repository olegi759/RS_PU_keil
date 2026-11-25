/*
 * safety.c
 *
 *  Created on: 20 авг. 2025 г.
 *      Author: Rafael Boltachev
 */

#include "safety.h"
#include "str_util.h"
#include "setting.h"
#include "common.h"


char safety_web_login[20] = {'\0'};
char safety_web_password[20] = {'\0'};



#define FACTORY_LOGIN 		"comonyx"
#define FACTORY_PASSWORD	"QsXc1357"

//корректен ли введенный логин и пароль
//BOOL safety_is_login_and_password_good(void)
//{
//	if ((strlen(setting.web_login) == 0) || (strlen(setting.web_password) == 0)) return TRUE;
//	if  ( strequals(safety_web_login, FACTORY_LOGIN) && strequals(safety_web_password, FACTORY_PASSWORD) ) return TRUE;
//	return strequals(safety_web_login, setting.web_login) && strequals(safety_web_password, setting.web_password);
//}
