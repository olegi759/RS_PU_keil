#ifndef _WEBPAGE_H_
#define _WEBPAGE_H_


#include "main.h"
#include "mup.h"
#include "setting.h"
#include "fault_data.h"

#define HTML_TITLE				"Тромбон РС-ПУ"
#define CGI_SCRIPT_INDEX		"index.cgi"
#define CGI_SCRIPT_NETWORK		"network.cgi"
#define CGI_SCRIPT_SETTING 		"setting.cgi"
#define CGI_SCRIPT_CONTROL		"control.cgi"
#define CGI_SCRIPT_MODE			"mode.cgi"
#define CGI_SCRIPT_FILEMANAGER	"filemanager.cgi"
#define CGI_SCRIPT_SAFETY		"safety.cgi"
#define CGI_SCRIPT_AUTH			"auth.cgi"
#define CGI_SCRIPT_NETFUNC		"netfunc.cgi"
#define CGI_SCRIPT_ERROR_USER_LEVEL		"errusrlvl.cgi"

#define HTML_AHEADER ""\
"<!DOCTYPE html>"CRLF""\
"<html>"CRLF""\
"<head>"CRLF""\
"	<title>"HTML_TITLE"</title>"CRLF""\
"	<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"CRLF""\
"	<link rel='stylesheet' href='style.css' type='text/css' />"CRLF""\
"	<script type='text/javascript' src='params.js'></script>"CRLF""\
"</head>"CRLF""\
"<body>"CRLF""\
"<ul id='menu'>"CRLF""\
"	<li><a href='index.html'>Информация</a></li>"CRLF""\
"	<li><a href='network.html'>Настройки сети</a></li>"CRLF""\
"	<li><a href='setting.html'>Общие настройки</a></li>"CRLF""\
"	<li><a href='control.html'>Контроль исправности</a></li>"CRLF""\
"	<li><a href='mode.html'>Режим работы</a></li>"CRLF""\
"	<li><a href='netfunc.html'>Сетевые функции</a></li>"CRLF""\
"	<li><a href='filemanager.html'>Файловый менеджер</a></li>"CRLF""\
"	<li><a href='safety.html'>Безопасность</a></li>"CRLF""\
"</ul>"CRLF""\
"<div id='content'>"CRLF""\
"<h1>"HTML_TITLE"</h1>"CRLF



#define HTML_AFOOTER ""\
"</div>"CRLF""\
"</body>"CRLF""\
"</html>"



#define PAGE_STYLE_CSS ""\
"body {text-align: center; margin:0px; padding: 0px;}"CRLF""\
"#menu {display: flex;flex-direction: column; width: 20%; min-height: 100vh; position: fixed; top: 0; left: 0; background-color: #595959; box-shadow: 4px 4px 8px 0 rgba(34, 60, 80, 0.2); margin: 0; padding-top: 20px;}"CRLF""\
"#content {padding: 0px 0px 20px 21%; margin: 0px;}"CRLF""\
"#menu li {position: relative; display: flex; align-items: center; height: 45px;	text-decoration: none; margin-bottom: 10px;}"CRLF""\
"#menu li a {font-family: 'Karla', sans-serif; font-size: 20px; color: #fff; padding: 8px 15px; background-color: #3498db; text-decoration: none; border: none;	border-radius: 4px;	font-weight: bold; margin: 0px;	width: 80%;}"CRLF""\
"#menu li a:hover { background-color: #54B8FB; }"CRLF""\
"table {margin: auto;}"CRLF""\
"table td,th {padding: 5px; border: 1px solid;}"CRLF""\
"h1, h2 {background: #EA700D; padding: 5px; 0px; 5px; 0px;  margin: 0px; width:100%;}"CRLF""\
"h3 {color:#990000;}"CRLF""\
"h4 {width:100%; background: #EEEEEE; padding: 10px 0px 10px 0px;}"CRLF""\
"input[type='text'] {width: 200px; text-align: left;}"CRLF""\
"select {width: 200px; text-align: left;}"CRLF""\
".sqrred {background-color:red; width: 20px; height: 20px;}"CRLF""\
".sqrgray {background-color:gray; width: 20px; height: 20px;}"CRLF""\
".sqryellow {background-color:yellow; width: 20px; height: 20px;}"CRLF""\
".sqrgreen {background-color:green; width: 20px; height: 20px;}"CRLF""\
".sqrblack {background-color:black; width: 20px; height: 20px;}"CRLF""\
".sqrwhite {background-color:white; width: 20px; height: 20px;}"


#define PAGE_PARAMS_JS  	""\
"function GetValById(id)"CRLF""\
"{"CRLF""\
"	var el = document.getElementById(id);"CRLF""\
"	if (el == null) return "";"CRLF""\
"	if (el.className == 'bitfield') {"CRLF""\
"		var res = 0;"CRLF""\
"		let els = document.querySelectorAll('#' + el.id + ' input[type=checkbox]');"CRLF""\
"		for (let i = 0; i < els.length; i++) {"CRLF""\
"			if (els[i].checked) res = res | (1 << i);"CRLF""\
"		}"CRLF""\
"		return 	res;"CRLF""\
"	}"CRLF""\
"    if ((el.type == 'checkbox') || (el.type == 'radio')) return (el.checked ? 'true' : 'false');"CRLF""\
"	else return el.value;"CRLF""\
"}"CRLF""\
CRLF""\
"function SetValById(id, val)"CRLF""\
"{"CRLF""\
"	var el = document.getElementById(id);"CRLF""\
"	if (el == null) return;"CRLF""\
"	if (el.className == 'bitfield') {"CRLF""\
"		let els = document.querySelectorAll('#' + el.id + ' input[type=checkbox]');"CRLF""\
"		for (let i = 0; i < els.length; i++) {"CRLF""\
"			els[i].checked = (val & (1 << i)) ? true : false;"CRLF""\
"		}"CRLF""\
"		return;"CRLF""\
"	}"CRLF""\
"	if (el instanceof HTMLInputElement) {"CRLF""\
"		if ((el.type == 'checkbox') || (el.type == 'radio')) el.checked = (val == 'true' ? true : false);"CRLF""\
"		else el.value = val;"CRLF""\
"	}"CRLF""\
"	else if (el instanceof HTMLSelectElement) {"CRLF""\
"		el.value = val;"CRLF""\
"	}"CRLF""\
"	else { el.innerHTML = val; }"CRLF""\
"}"CRLF""\
CRLF""\
"function GetIDsVals(selector, withval)"CRLF""\
"{"CRLF""\
"	let els = document.querySelectorAll(selector);"CRLF""\
"	var res = '';"CRLF""\
"	els.forEach((el) => {"CRLF""\
"		if (el.id != '') {"CRLF""\
"			if (res != '') res += '&';"CRLF""\
"			res += el.id;"CRLF""\
"			if (withval) {"CRLF""\
"				res += '=';"CRLF""\
"				res += GetValById(el.id);"CRLF""\
"			}"CRLF""\
"		}"CRLF""\
"	});"CRLF""\
"	return res;"CRLF""\
"}"CRLF""\
CRLF""\
"function SendRequest(r_method, r_path, r_args)"CRLF""\
"{"CRLF""\
"    var Request = false;"CRLF""\
"    if (window.XMLHttpRequest){"CRLF""\
"        Request = new XMLHttpRequest();"CRLF""\
"    }"CRLF""\
"    else if (window.ActiveXObject)"CRLF""\
"    {"CRLF""\
"        try {"CRLF""\
"             Request = new ActiveXObject('Microsoft.XMLHTTP');"CRLF""\
"        }"CRLF""\
"        catch (CatchException) {"CRLF""\
"             Request = new ActiveXObject('Msxml2.XMLHTTP');"CRLF""\
"        }"CRLF""\
"    }"CRLF""\
CRLF""\
"	if (!Request) return;"CRLF""\
CRLF""\
"    Request.onreadystatechange = function() {"CRLF""\
"        if (Request.readyState == 4) {"CRLF""\
"			JSON.parse(Request.responseText, (key, value) => {"CRLF""\
"				if (key == 'result') alert(value);"CRLF""\
"				else { SetValById(key, value);  if ((key == '"KEY_LINES_COUNT"') && (typeof OnLinesCount === 'function')) { OnLinesCount(value); }  }"CRLF""\
"			});"CRLF""\
CRLF""\
 "       }"CRLF""\
 "   }"CRLF""\
 CRLF""\
"    if (r_method.toLowerCase() == 'get' && r_args.length > 0) {"CRLF""\
"		r_path += '?' + r_args;"CRLF""\
"	}"CRLF""\
CRLF""\
"    Request.open(r_method, r_path, true);"CRLF""\
CRLF""\
"    if (r_method.toLowerCase() == 'post') {"CRLF""\
"        Request.setRequestHeader('Content-Type','application/x-www-form-urlencoded; charset=utf-8');"CRLF""\
"        Request.send(r_args);"CRLF""\
"    } else {"CRLF""\
"        Request.send(null);"CRLF""\
"    }"CRLF""\
"}"CRLF""\
"function setColumnVisibility(table, columnIndex, visible)"CRLF""\
"{"CRLF""\
"	const cols = document.querySelectorAll(`#` + table + ` td:nth-child(${columnIndex + 1}), #` + table + ` th:nth-child(${columnIndex + 1})`);"CRLF""\
"	cols.forEach(col => {"CRLF""\
"		col.style.display = (visible ? '' : 'none');"CRLF""\
"	});"CRLF""\
"}"CRLF""\
""CRLF""\
"function setRowVisibility(table, rowIndex, visible)"CRLF""\
"{"CRLF""\
"	const rows = document.querySelectorAll(`#` + table + ` tr:nth-child(${rowIndex + 1})`);"CRLF""\
"	rows.forEach(row => {"CRLF""\
"		row.style.display = (visible ? '' : 'none');"CRLF""\
"	});"CRLF""\
"}"

/*
CRLF""\
CRLF""\
"function setColumnVisibility(table, columnIndex, visible) {"CRLF""\
"	const cols = document.querySelectorAll(`#` + table + ` td:nth-child(${columnIndex + 1}), #` + table + ` th:nth-child(${columnIndex + 1})`);"CRLF""\
"	cols.forEach(col => {  col.style.display = (visible ? '' : 'none'); });"CRLF""\
"}"CRLF""\
CRLF""\
"function setRowVisibility(table, rowIndex, visible) {"CRLF""\
"	const rows = document.querySelectorAll(`#` + table + ` tr:nth-child(${rowIndex + 1})`);"CRLF""\
"	rows.forEach(row => { row.style.display = (visible ? '' : 'none'); });"CRLF""\
"}"
*/

#define HTML_CONTENT_INDEX ""\
"<script type='text/javascript'>"CRLF""\
"	window.onload = function() {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_INDEX"', GetIDsVals('input,#"KEY_INFO_FAULTS"', false));"CRLF""\
"	};"CRLF""\
"</script>"CRLF""\
"<h3>Информация</h3>"CRLF""\
"<h4>Название</h4>"CRLF""\
"<table>"CRLF""\
"	<tr><td>Прибор:</td><td><input id='"KEY_NAME"' disabled></td><tr>"CRLF""\
"</table>"CRLF""\
"<h4>Прошивка</h4>"CRLF""\
"<table>"CRLF""\
"	<tr><td>Версия ядра CM4:</td><td><input id='"KEY_VERSION"' disabled></td><tr>"CRLF""\
"	<tr><td>Версия ядра CM7:</td><td><input id='"KEY_CM7_VERSION"' disabled></td><tr>"CRLF""\
"	<tr><td>Версия модуля МКИ:</td><td><input id='"KEY_MKI_VERSION"' disabled></td><tr>"CRLF""\
"	<tr><td>Версия модуля МКК1:</td><td><input id='"KEY_MKK1_VERSION"' disabled></td><tr>"CRLF""\
"	<tr><td>Версия модуля МКК2:</td><td><input id='"KEY_MKK2_VERSION"' disabled></td><tr>"CRLF""\
"</table>"CRLF""\
"<h4>Сеть</h4>"CRLF""\
"<table>"CRLF""\
"	<tr><th>Параметр</th><th>Основной канал</th><th>Резервный канал</th></tr>"CRLF""\
"	<tr><td>МАК адрес:</td><td><input id='"KEY_NET0_MAC"' disabled></td><td><input id='"KEY_NET1_MAC"' disabled></td></tr>"CRLF""\
"	<tr><td>Адрес IPv4:</td><td><input id='"KEY_NET0_IP"' disabled></td><td><input id='"KEY_NET1_IP"' disabled></td></tr>"CRLF""\
"	<tr><td>Шлюз по умолчанию IP:</td><td><input id='"KEY_NET0_GW"' disabled></td><td><input id='"KEY_NET1_GW"' disabled></td></tr>"CRLF""\
"	<tr><td>Маска подсети IPv4:</td><td><input id='"KEY_NET0_MASK"' disabled></td><td><input id='"KEY_NET1_MASK"' disabled></td></tr>"CRLF""\
"	<tr><td>ДНС сервер IPv4:</td><td><input id='"KEY_NET0_DNS"' disabled></td><td><input id='"KEY_NET1_DNS"' disabled></td></tr>"CRLF""\
"	<tr><td>DHCP:</td><td><input type='checkbox' id='"KEY_NET0_DHCP"' disabled></td><td><input type='checkbox' id='"KEY_NET1_DHCP"' disabled></td></tr>"CRLF""\
"</table>"CRLF""\
"<h4>Текущий режим работы</h4>"CRLF""\
"<input id='"KEY_MODE_NOW"' disabled style='text-align:center;' />"CRLF""\
"<h4>Журнал событий</h4>"CRLF""\
"<a href='pu.log'>Скачать</a>"CRLF""\
"<h4>Неисправностей</h4>"CRLF""\
"<span id='"KEY_INFO_FAULTS"'></span>"CRLF""\
"<h4>Текущий уровеь доступа</h4>"CRLF""\
"<input id='"KEY_LEVEL_NOW"' disabled style='text-align:center;' />"CRLF""\
"<h4>Уникальный идентификатор процессора</h4>"CRLF""\
"<input id='"KEY_STM32_UID"' disabled style='text-align:center;' />"CRLF""\
"<h4>Перезагрузка</h4>"CRLF""\
"<input type='button' value='Перезагрузить ПУ' onclick=\"if (confirm('Перезагрузить ПУ ?')) { SendRequest('GET', '"CGI_SCRIPT_INDEX"', 'reboot'); }\">"CRLF

#define PAGE_INDEX_HTML 	HTML_AHEADER""HTML_CONTENT_INDEX""HTML_AFOOTER


#define HTML_CONTENT_NETWORK	""\
"<script type='text/javascript'>"CRLF""\
"	window.onload = function(){"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_NETWORK"', GetIDsVals('#net input', false));"CRLF""\
"	};"CRLF""\
"</script>"CRLF""\
"<h3>Настроки сети</h3>"CRLF""\
"<table id='net'>"CRLF""\
"	<tr><th>Параметр</th><th>Основной канал</th><th>Резервный канал</th></tr>"CRLF""\
"	<tr><td>МАК адрес:</td><td><input id='"KEY_NET0_MAC"'></td><td><input id='"KEY_NET1_MAC"'></td></tr>"CRLF""\
"	<tr><td>Адрес IPv4:</td><td><input id='"KEY_NET0_IP"'></td><td><input id='"KEY_NET1_IP"'></td></tr>"CRLF""\
"	<tr><td>Шлюз по умолчанию IP:</td><td><input id='"KEY_NET0_GW"'></td><td><input id='"KEY_NET1_GW"'></td></tr>"CRLF""\
"	<tr><td>Маска подсети IPv4:</td><td><input id='"KEY_NET0_MASK"'></td><td><input id='"KEY_NET1_MASK"'></td></tr>"CRLF""\
"	<tr><td>ДНС сервер IPv4:</td><td><input id='"KEY_NET0_DNS"'></td><td><input id='"KEY_NET1_DNS"'></td></tr>"CRLF""\
"	<tr><td>DHCP:</td><td><input type='checkbox' id='"KEY_NET0_DHCP"'></td><td><input type='checkbox' id='"KEY_NET1_DHCP"'></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_NETWORK"', GetIDsVals('#net input', true));\">"CRLF

#define PAGE_NETWORK_HTML 	HTML_AHEADER""HTML_CONTENT_NETWORK""HTML_AFOOTER




#define HTML_CONTENT_SETTING	""\
"<script type='text/javascript'>"CRLF""\
"	window.onload = function(){"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_SETTING"', GetIDsVals('#all input,select,#"KEY_DATE_TIME",#um select,#int_spk,#groups tr[class=bitfield]', false));"CRLF""\
"	};"CRLF""\
"	function OnLinesCount(count) {"CRLF""\
"		for (let i = count; i <= 24; i++) {"CRLF""\
"			setColumnVisibility('groups', i, false);"CRLF""\
"		}"CRLF""\
"		for (let i = count; i <= 12; i++) {"CRLF""\
"			setColumnVisibility('um', i, false);"CRLF""\
"		}"CRLF""\
"		setRowVisibility('um', 2, count > 12);"CRLF""\
"		setRowVisibility('um', 3, count > 12);"CRLF""\
"	}"CRLF""\
"</script>"CRLF""\
"<h3>Общие настройки</h3>"CRLF""\
"<h4>Основные настройки</h4>"CRLF""\
"<table id='all'>"CRLF""\
"	<tr><td>Адрес ПУ:</td><td>"CRLF""\
"		<select id='"KEY_UART_LOCAL_ADDR"'>"CRLF""\
"			<option value='10'>10</option>"CRLF""\
"			<option value='20'>20</option>"CRLF""\
"			<option value='30'>30</option>"CRLF""\
"			<option value='40'>40</option>"CRLF""\
"			<option value='50'>50</option>"CRLF""\
"			<option value='60'>60</option>"CRLF""\
"			<option value='70'>70</option>"CRLF""\
"			<option value='80'>80</option>"CRLF""\
"			<option value='90'>90</option>"CRLF""\
"			<option value='100'>100</option>"CRLF""\
"			<option value='110'>110</option>"CRLF""\
"			<option value='120'>120</option>"CRLF""\
"			<option value='130'>130</option>"CRLF""\
"			<option value='140'>140</option>"CRLF""\
"			<option value='150'>150</option>"CRLF""\
"			<option value='160'>160</option>"CRLF""\
"			<option value='170'>170</option>"CRLF""\
"			<option value='180'>180</option>"CRLF""\
"			<option value='190'>190</option>"CRLF""\
"			<option value='200'>200</option>"CRLF""\
"			<option value='210'>210</option>"CRLF""\
"			<option value='220'>220</option>"CRLF""\
"			<option value='230'>230</option>"CRLF""\
"			<option value='240'>240</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\
"	<tr><td>Кол-во пультов диспетчера (ПД):</td><td>"CRLF""\
"		<select id='"KEY_PD_COUNT"'>"CRLF""\
"			<option value='0'>Нет</option>"CRLF""\
"			<option value='1'>ПД-1</option>"CRLF""\
"			<option value='2'>ПД-2</option>"CRLF""\
"			<option value='3'>ПД-1 и ПД-2</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\
"	<tr><td>Кол-во усилителей мощности (РС-УМ-500):</td><td>"CRLF""\
"		<select id='"KEY_UM_COUNT"'>"CRLF""\
"			<option value='0'>0</option>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"			<option value='5'>5</option>"CRLF""\
"			<option value='6'>6</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\
"	<tr><td>Общая громкость:</td><td><input id='"KEY_SAI_VOLUME"' value=''></td><tr>"CRLF""\
"	<tr><td>Громкость микрофона:</td><td><input id='"KEY_MIC_VOLUME"' value=''></td><tr>"CRLF""\
"	<tr><td>Громкость входа МЧС:</td><td><input id='"KEY_MCHS_VOLUME"' value=''></td><tr>"CRLF""\
"	<tr><td>Максимальный размер лог файла (КБ):</td><td><input id='"KEY_LOGFILE_SIZE"' value=''></td><tr>"CRLF""\
"	<tr><td>Кол-во линий:</td><td>"CRLF""\
"		<select id='"KEY_LINES_COUNT"'>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"			<option value='8'>8</option>"CRLF""\
"			<option value='12'>12</option>"CRLF""\
"			<option value='24'>24</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\
"	<tr><td>Микрофон с фиксацией:</td><td><input type='checkbox' id='"KEY_MIC_FIX"'></td><tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#all input,select', true));\">"CRLF\
"<br>"CRLF""\
"<h4>Дата и время</h4>"CRLF""\
"<table>"CRLF""\
"	<tr><td>Дата и время:</td><td><input type='datetime-local'  id='"KEY_DATE_TIME"' value=''></td><tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#"KEY_DATE_TIME"', true));\">"CRLF""\
CRLF""\
"<h4>Распределение линий вещания между усилителями мощности (УМ)</h4>"CRLF""\
"<table id='um'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Линия вещания</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td>Усилитель мощности</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_1' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_2' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_3' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_4' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_5' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_6' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_7' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_8' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_9' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_10' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_11' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_12' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<th>Линия вещания</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td>Усилитель мощности</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_13' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_14' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_15' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_16' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_17' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_18' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_19' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_20' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_21' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_22' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_23' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"		<td>"CRLF""\
"			<select id='um_24' style='width:auto;'>"CRLF""\
"				<option value='0'>Нет</option>"CRLF""\
"				<option value='1'>УМ-1</option>"CRLF""\
"				<option value='2'>УМ-2</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#um select', true));\">"CRLF""\
CRLF""\
"<h4>Контроль вещания на внутреннем динамике</h4>"CRLF""\
"<table>"CRLF""\
"	<tr><td>Внутренний динамик:</td><td>"CRLF""\
"		<select id='int_spk'>"CRLF""\
"			<option value='0'>Выключить</option>"CRLF""\
"			<option value='1'>Подключить к входу УМ1</option>"CRLF""\
"			<option value='2'>Подключить к входу УМ2</option>"CRLF""\
"			<option value='3'>Подключить к входу УМ1 и УМ2</option>"CRLF""\
"			<option value='4'>Подключить к линейному входу</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#int_spk', true));\">"CRLF""\
"<br>"CRLF""\
"<h4>Настройка групп</h4>"CRLF""\
"<table id='groups'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Группа / Линия</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_1' class='bitfield'>"CRLF""\
"		<td>1</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_2' class='bitfield'>"CRLF""\
"		<td>2</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_3' class='bitfield'>"CRLF""\
"		<td>3</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_4' class='bitfield'>"CRLF""\
"		<td>4</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_5' class='bitfield'>"CRLF""\
"		<td>5</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_6' class='bitfield'>"CRLF""\
"		<td>6</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_7' class='bitfield'>"CRLF""\
"		<td>7</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_8' class='bitfield'>"CRLF""\
"		<td>8</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_9' class='bitfield'>"CRLF""\
"		<td>9</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='group_10' class='bitfield'>"CRLF""\
"		<td>10</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#groups tr[class=bitfield]', true));\">"CRLF""\
"<br>"CRLF""\
"<h4>Настройка адресов УМ</h4>"CRLF""\
"<table id='uma'>"CRLF""\
"	<tr><td>Адрес какому УМ:</td><td><input id='umad' value='255'></td></tr>"CRLF""\
"	<tr><td>Какой адрес установить:</td><td><input id='umas'></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Послать' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SETTING"', GetIDsVals('#uma input', true));\">"CRLF



#define PAGE_SETTING_HTML 	HTML_AHEADER""HTML_CONTENT_SETTING""HTML_AFOOTER


#define HTML_SQR_RED 	"<div class='sqrred'/>"
#define HTML_SQR_GRAY 	"<div class='sqrgray'/>"
#define HTML_SQR_GREEN 	"<div class='sqrgreen'/>"
#define HTML_SQR_BLACK 	"<div class='sqrblack'/>"
#define HTML_SQR_YELLOW "<div class='sqryellow'/>"
#define HTML_SQR_WHITE 	"<div class='sqrwhite'/>"


#define HTML_CONTENT_CONTROL	""\
"<script type='text/javascript'>"CRLF""\
"	function update() {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#msls span,#miss span,#mois span,#mup span,#pdcons span,#ums span', false));"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#fsound,#finsig,#fstart,#fums,#fall', false));"CRLF""\
"	}"CRLF""\
"	window.onload = function() {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#"KEY_LINES_COUNT",#"KEY_CONTROL_TIME",#lsl,#csls,#ciss,#cois input', false));"CRLF""\
"		update();"CRLF""\
"	};"CRLF""\
"	setInterval(update, 10000);"CRLF""\
"	function OnLinesCount(count) {"CRLF""\
"		for (let i = count; i <= 24; i++) {"CRLF""\
"			setColumnVisibility('cslmsl', i, false);"CRLF""\
"			setColumnVisibility('cismis', i, false);"CRLF""\
"		}"CRLF""\
"		setColumnVisibility('coimoi', 2, count > 12);"CRLF""\
"	}"CRLF""\
"</script>"CRLF""\
"<input id='"KEY_LINES_COUNT"' type='hidden'>"CRLF""\
"<h3>Контроль исправности</h3>"CRLF""\
"<h4>Период контроля исправностей линий звукового оповещения, линий входов сигнализации, линии входа \"Неисправность\" в секундах</h4>"CRLF""\
"<input id='"KEY_CONTROL_TIME"' style='text-align:center'>"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#"KEY_CONTROL_TIME"', true));\">"CRLF""\
"<h4>Контроль исправности линий звукового оповещения</h4>"CRLF""\
"<table id='cslmsl'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Линия оповещения</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<!-- csl - check sound line -->"CRLF""\
"	<tr id='csls' class='bitfield'>"CRLF""\
"		<td>Включить</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<!-- msl - measure sound line -->"CRLF""\
"	<tr id='msls'>"CRLF""\
"		<td>Исправность</td>"CRLF""\
"		<td><span id='msl_1'></span></td>"CRLF""\
"		<td><span id='msl_2'></span></td>"CRLF""\
"		<td><span id='msl_3'></span></td>"CRLF""\
"		<td><span id='msl_4'></span></td>"CRLF""\
"		<td><span id='msl_5'></span></td>"CRLF""\
"		<td><span id='msl_6'></span></td>"CRLF""\
"		<td><span id='msl_7'></span></td>"CRLF""\
"		<td><span id='msl_8'></span></td>"CRLF""\
"		<td><span id='msl_9'></span></td>"CRLF""\
"		<td><span id='msl_10'></span></td>"CRLF""\
"		<td><span id='msl_11'></span></td>"CRLF""\
"		<td><span id='msl_12'></span></td>"CRLF""\
"		<td><span id='msl_13'></span></td>"CRLF""\
"		<td><span id='msl_14'></span></td>"CRLF""\
"		<td><span id='msl_15'></span></td>"CRLF""\
"		<td><span id='msl_16'></span></td>"CRLF""\
"		<td><span id='msl_17'></span></td>"CRLF""\
"		<td><span id='msl_18'></span></td>"CRLF""\
"		<td><span id='msl_19'></span></td>"CRLF""\
"		<td><span id='msl_20'></span></td>"CRLF""\
"		<td><span id='msl_21'></span></td>"CRLF""\
"		<td><span id='msl_22'></span></td>"CRLF""\
"		<td><span id='msl_23'></span></td>"CRLF""\
"		<td><span id='msl_24'></span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#csls', true));\">"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"<table>"CRLF""\
"	<tr><td>Допуск отклонений %:</td><td><input id='lsl' value=''></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#lsl', true));\">"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"Расшифровка неисправности<br>"CRLF""\
"<span id='fsound'></span>"CRLF""\
CRLF""\
"<h4>Контроль исправности линий входов сигнализации</h4>"CRLF""\
"<table id='cismis'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Входы сигнализации</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<!-- cis - check input signalization -->"CRLF""\
"	<tr id='ciss' class='bitfield'>"CRLF""\
"		<td>Включить</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<!-- mis - measure input signalization -->"CRLF""\
"	<tr id='miss'>"CRLF""\
"		<td>Исправность</td>"CRLF""\
"		<td><span id='mis_1'></span></td>"CRLF""\
"		<td><span id='mis_2'></span></td>"CRLF""\
"		<td><span id='mis_3'></span></td>"CRLF""\
"		<td><span id='mis_4'></span></td>"CRLF""\
"		<td><span id='mis_5'></span></td>"CRLF""\
"		<td><span id='mis_6'></span></td>"CRLF""\
"		<td><span id='mis_7'></span></td>"CRLF""\
"		<td><span id='mis_8'></span></td>"CRLF""\
"		<td><span id='mis_9'></span></td>"CRLF""\
"		<td><span id='mis_10'></span></td>"CRLF""\
"		<td><span id='mis_11'></span></td>"CRLF""\
"		<td><span id='mis_12'></span></td>"CRLF""\
"		<td><span id='mis_13'></span></td>"CRLF""\
"		<td><span id='mis_14'></span></td>"CRLF""\
"		<td><span id='mis_15'></span></td>"CRLF""\
"		<td><span id='mis_16'></span></td>"CRLF""\
"		<td><span id='mis_17'></span></td>"CRLF""\
"		<td><span id='mis_18'></span></td>"CRLF""\
"		<td><span id='mis_19'></span></td>"CRLF""\
"		<td><span id='mis_20'></span></td>"CRLF""\
"		<td><span id='mis_21'></span></td>"CRLF""\
"		<td><span id='mis_22'></span></td>"CRLF""\
"		<td><span id='mis_23'></span></td>"CRLF""\
"		<td><span id='mis_24'></span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#ciss', true));\">"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"Расшифровка неисправности<br>"CRLF""\
"<span id='finsig'></span>"CRLF""\
CRLF""\
"<h4>Контроль исправности линии входа \"Неисправность\"</h4>"CRLF""\
"<table id='coimoi'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Вход</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"	</tr>"CRLF""\
"	<!-- coi - check other input -->"CRLF""\
"	<tr id='cois'>"CRLF""\
"		<td>Включить</td>"CRLF""\
"		<td><input type='checkbox' id='coi_1'></td>"CRLF""\
"		<td><input type='checkbox' id='coi_2'></td>"CRLF""\
"	</tr>"CRLF""\
"	<!-- moi - measure other input -->"CRLF""\
"	<tr id='mois'>"CRLF""\
"		<td>Исправность</td>"CRLF""\
"		<td><span id='moi_1'></span></td>"CRLF""\
"		<td><span id='moi_2'></span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_CONTROL"', GetIDsVals('#cois input', true));\">"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"Расшифровка неисправности<br>"CRLF""\
"<span id='fstart'></span>"CRLF""\
"<h4>Контроль электропитания</h4>"CRLF""\
"<table id='mup'>"CRLF""\
"	<tr>"CRLF""\
"		<td>"MUP_BIT_ERROR_220V_STR"</td>"CRLF""\
"		<td><span id='pw_220'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td>АКБ подключена</td>"CRLF""\
"		<td><span id='pw_akb''>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td>"MUP_BIT_DISCHARGE_ACC_STR"</td>"CRLF""\
"		<td><span id='pw_charge'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"	<!-- <tr>"CRLF""\
"		<td>"MUP_BIT_CAPACITY_ACC_STR"</td>"CRLF""\
"		<td><span id='pw_cap'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr> -->"CRLF""\
"	<tr>"CRLF""\
"		<td>"MUP_BIT_TD_BP_STR"</td>"CRLF""\
"		<td><span id='pw_temp'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<h4>Контроль связи с ПД</h4>"CRLF""\
"<table>"CRLF""\
"	<tr>"CRLF""\
"		<th>ПД</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>Кольцо</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='pdcons'>"CRLF""\
"		<td>Исправность</td>"CRLF""\
"		<td><span id='pdcon1'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='pdcon2'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='pdring'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<h4>Контроль УМ</h4>"CRLF""\
"<table id='ums'>"CRLF""\
"	<tr>"CRLF""\
"		<th>УМ</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td>Исправность</td>"CRLF""\
"		<td><span id='umf_1'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='umf_2'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='umf_3'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='umf_4'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='umf_5'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"		<td><span id='umf_6'>"HTML_SQR_GRAY"</span></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"Расшифровка неисправности<br>"CRLF""\
"<span id='fums'></span>"CRLF""\
"<h4>Другие неисправности</h4>"CRLF""\
"<span id='fall'></span>"



#define PAGE_CONTROL_HTML 	HTML_AHEADER""HTML_CONTENT_CONTROL""HTML_AFOOTER




#define TILIBOM_STR		"звук уведомления"

#define HTML_CONTENT_MODE	""\
"<h3>Режим работы</h3>"CRLF""\
"<script type='text/javascript'>"CRLF""\
"	function fire_update()  {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_MODE"', GetIDsVals('#fire_uo', true) + '&' + GetIDsVals('#mode_fire_other input,#mode_fire_lines tr[class=bitfield]', false));"CRLF""\
"	}"CRLF""\
"	window.onload = function(){"CRLF""\
"		fire_update();"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_MODE"', '"KEY_LINES_COUNT"&mchs_lines&' + GetIDsVals('#sound_lines,#mode_sound_src select,#mode_sound_src input,#manual_cycle,#fire_cycle,#mode_manual input', false));"CRLF""\
"  };"CRLF""\
"	function OnLinesCount(count) {"CRLF""\
"		for (let i = count; i <= 24; i++) {"CRLF""\
"			setColumnVisibility('mode_fire_lines', i, false);"CRLF""\
"			setRowVisibility('mode_fire_lines', i, false);"CRLF""\
"			setColumnVisibility('mode_mchs', i, false);"CRLF""\
"			setColumnVisibility('mode_sound_ls', i, false);"CRLF""\
"		}"CRLF""\
"	}"CRLF""\
"</script>"CRLF""\
"<input id='"KEY_LINES_COUNT"' type='hidden'>"CRLF""\
"<h4>Режим Ручной ПУСК</h4>"CRLF""\
"<table id='mode_manual'>"CRLF""\
"	<tr><td>Проигрывать "TILIBOM_STR" перед трансляцией микрофона:</td><td><input type='checkbox' id='manual_tb_mic'></td><tr>"CRLF""\
"	<tr><td>Проигрывать "TILIBOM_STR" перед воспроизведением файла:</td><td><input type='checkbox' id='manual_tb_file'></td><tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_MODE"', GetIDsVals('#mode_manual input', true));\">"CRLF""\
"<br>"CRLF""\
"<h4>Режим ПОЖАР</h4>"CRLF""\
"<table><tr><td>Зацикливать последний активный УО:</td><td><input type='checkbox' id='fire_cycle'></td><tr></table>"CRLF""\
"<br>"CRLF""\
"Уровень оповещения:"CRLF""\
"<select id='fire_uo' onchange='fire_update();'>"CRLF""\
"	<option value='1'>УО-1</option>"CRLF""\
"	<option value='2'>УО-2</option>"CRLF""\
"	<option value='3'>УО-3</option>"CRLF""\
"	<option value='4'>УО-4</option>"CRLF""\
"	<option value='5'>УО-5</option>"CRLF""\
"	<option value='6'>УО-6</option>"CRLF""\
"	<option value='7'>УО-7</option>"CRLF""\
"	<option value='8'>УО-8</option>"CRLF""\
"	<option value='9'>УО-9</option>"CRLF""\
"	<option value='10'>УО-10</option>"CRLF""\
"</select>"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"<table id='mode_fire_other'>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Включено</td><td><input type='checkbox' id='fire_enable'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Проигрывать "TILIBOM_STR" перед воспроизведением файла</td><td><input type='checkbox' id='fire_tb'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Задержка секунд</td><td><input id='fire_delay'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Кол-во повторов</td><td><input id='fire_repeat'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Файл</td><td><input id='fire_file'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<table id='mode_fire_lines'>"CRLF""\
"	<tr>"CRLF""\
"		<th><div><div style='display: inline-block;'>Пожарный<br>вход</div><div style='display: inline-block;'>/</div><div style='display: inline-block;'>Линия<br>оповещения</div></div></th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_1' class='bitfield'>"CRLF""\
"		<td>1</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox''></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_2' class='bitfield'>"CRLF""\
"		<td>2</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_3' class='bitfield'>"CRLF""\
"		<td>3</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_4' class='bitfield'>"CRLF""\
"		<td>4</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_5' class='bitfield'>"CRLF""\
"		<td>5</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_6' class='bitfield'>"CRLF""\
"		<td>6</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_7' class='bitfield'>"CRLF""\
"		<td>7</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_8' class='bitfield'>"CRLF""\
"		<td>8</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_9' class='bitfield'>"CRLF""\
"		<td>9</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_10' class='bitfield'>"CRLF""\
"		<td>10</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_11' class='bitfield'>"CRLF""\
"		<td>11</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_12' class='bitfield'>"CRLF""\
"		<td>12</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_13' class='bitfield'>"CRLF""\
"		<td>13</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_14' class='bitfield'>"CRLF""\
"		<td>14</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_15' class='bitfield'>"CRLF""\
"		<td>15</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_16' class='bitfield'>"CRLF""\
"		<td>16</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_17' class='bitfield'>"CRLF""\
"		<td>17</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_18' class='bitfield'>"CRLF""\
"		<td>18</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_19' class='bitfield'>"CRLF""\
"		<td>19</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_20' class='bitfield'>"CRLF""\
"		<td>20</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_21' class='bitfield'>"CRLF""\
"		<td>21</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_22' class='bitfield'>"CRLF""\
"		<td>22</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_23' class='bitfield'>"CRLF""\
"		<td>23</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='fls_24' class='bitfield'>"CRLF""\
"		<td>24</td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_MODE"', GetIDsVals('#fire_uo', true) + '&' + GetIDsVals('#mode_fire_other input, #mode_fire_lines tr[class=bitfield], #fire_cycle', true));\">"CRLF""\
"<br>"CRLF""\
CRLF""\
"<h4>Режим МЧС</h4>"CRLF""\
"<table id='mode_mchs'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Линия оповещения</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='mchs_lines' class='bitfield'>"CRLF""\
"		<td></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_MODE"', GetIDsVals('#mchs_lines', true));\">"CRLF""\
"<br>"CRLF""\
CRLF""\
"<h4>Режим Звуковое вещание</h4>"CRLF""\
"<table id='mode_sound_src'>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Источник звукового вещания</td><td>"CRLF""\
"			<select id='sound_src'>"CRLF""\
"				<option value='0'>Микрофон</option>"CRLF""\
"				<option value='1'>Плеер ПУ</option>"CRLF""\
"				<option value='2'>Линейный вход</option>"CRLF""\
"			</select>"CRLF""\
"		</td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Файл</td><td><input id='sound_file'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Проигрывать "TILIBOM_STR" перед воспроизведением:</td><td><input id='sound_tb' type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<table id='mode_sound_ls'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Линия оповещения</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='sound_lines' class='bitfield'>"CRLF""\
"		<td></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_MODE"', GetIDsVals('#sound_lines,#mode_sound_src select,#mode_sound_src input', true));\">"CRLF""\
"<br>"


#define PAGE_MODE_HTML 	HTML_AHEADER""HTML_CONTENT_MODE""HTML_AFOOTER

/* "<form method='POST' action='' enctype='multipart/form-data'>"CRLF""\ */
/* "	<input type='file' name='uploadfile' required><br><br>"CRLF""\ */
/* "	<input type='submit' value='Загрузить файл' />"CRLF""\ */
/* "</form>"CRLF""\ */


#define HTML_CONTENT_FILEMANAGER	""\
"<h3>Файловый менеджер</h3>"CRLF""\
"<script type='text/javascript'>"CRLF""\
"	function update()  {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', GetIDsVals('#local_files,#flash_files', false));"CRLF""\
"	}"CRLF""\
"	window.onload = function(){"CRLF""\
"		update();"CRLF""\
"  };"CRLF""\
"</script>"CRLF""\
"<h4>Файлы на локальном диске ПУ</h4>"CRLF""\
"<span id='local_files'></span>"CRLF""\
"<br><br>"CRLF""\
"<table>"CRLF""\
"<tr><td style='padding: 15px;'>Удалить файл: <input id='local_delete'><input type='button' value='Удалить' onclick=\"SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', GetIDsVals('#local_delete', true)); update();\"></td></tr>"CRLF""\
"<tr><td style='padding: 15px;'>"CRLF""\
"<form method='POST' action='' enctype='multipart/form-data'>"CRLF""\
"	<input type='file' name='uploadfile' required>"CRLF""\
"	<input type='submit' value='Загрузить файл' />"CRLF""\
"</form>"CRLF""\
"</td></tr>"CRLF""\
"<tr><td style='padding: 15px;'>Переименовать файл: Из:<input id='local_rename_old'> В:<input id='local_rename_new'> <input type='button' value='Переименовать' onclick=\"SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', GetIDsVals('#local_rename_old,#local_rename_new', true)); update();\"></td></tr>"CRLF""\
"<tr><td style='padding: 15px;'>Копировать файл на внешний Flash Disk: <input id='local_copy'><input type='button' value='Скопировать' onclick=\"SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', GetIDsVals('#local_copy', true)); update();\"></td></tr>"CRLF""\
"<tr><td style='padding: 15px;'>Форматировать локальный диск: <input type='button' value='Форматировать' onclick=\"if (confirm('Форматировать ?')) { SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', 'local_format'); update();}\"></td></tr>"CRLF""\
"</table>"CRLF""\
"<h4>Файлы на Flash Disk</h4>"CRLF""\
"<span id='flash_files'></span>"CRLF""\
"<br><br>"CRLF""\
"Копировать файл на локальный диск: <input id='flash_copy'><input type='button' value='Скопировать' onclick=\"SendRequest('GET', '"CGI_SCRIPT_FILEMANAGER"', GetIDsVals('#flash_copy', true)); update();\">"CRLF




#define PAGE_FILEMANAGER_HTML 	HTML_AHEADER""HTML_CONTENT_FILEMANAGER""HTML_AFOOTER


#define HTML_CONTENT_SAFETY	""\
"<h3>Безопасность</h3>"CRLF""\
"<script type='text/javascript'>"CRLF""\
"	window.onload = function(){"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_SAFETY"', GetIDsVals('#pincodes input,#pincodes select,#"KEY_WEB_LOGIN",#"KEY_WEB_PASSWORD"', false));"CRLF""\
"  };"CRLF""\
"</script>"CRLF""\
"<h4>Пользвователи ПУ</h4>"CRLF""\
"<table id='pincodes'>"CRLF""\
"<tr><th>Номер</th><th>Имя</th><th>Пин-код</th><th>Уровень доступа</th></tr>"CRLF""\
"<tr>"CRLF""\
"	<td>1</td>"CRLF""\
"	<td><input id='uname_1'></td>"CRLF""\
"	<td><input id='upin_1' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_1'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>2</td>"CRLF""\
"	<td><input id='uname_2'></td>"CRLF""\
"	<td><input id='upin_2' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_2'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>3</td>"CRLF""\
"	<td><input id='uname_3'></td>"CRLF""\
"	<td><input id='upin_3' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_3'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>4</td>"CRLF""\
"	<td><input id='uname_4'></td>"CRLF""\
"	<td><input id='upin_4' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_4'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>5</td>"CRLF""\
"	<td><input id='uname_5'></td>"CRLF""\
"	<td><input id='upin_5' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_5'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>6</td>"CRLF""\
"	<td><input id='uname_6'></td>"CRLF""\
"	<td><input id='upin_6' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_6'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>7</td>"CRLF""\
"	<td><input id='uname_7'></td>"CRLF""\
"	<td><input id='upin_7' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_7'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>8</td>"CRLF""\
"	<td><input id='uname_8'></td>"CRLF""\
"	<td><input id='upin_8' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_8'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>9</td>"CRLF""\
"	<td><input id='uname_9'></td>"CRLF""\
"	<td><input id='upin_9' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_9'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"	<td>10</td>"CRLF""\
"	<td><input id='uname_10'></td>"CRLF""\
"	<td><input id='upin_10' value='' pattern='[1-9]{0,8}' title='цифры от 1 до 9 максимум 8 символов' style='text-align:center;'></td>"CRLF""\
"	<td>"CRLF""\
"		<select id='ulevel_10'>"CRLF""\
"			<option value='1'>1</option>"CRLF""\
"			<option value='2'>2</option>"CRLF""\
"			<option value='3'>3</option>"CRLF""\
"			<option value='4'>4</option>"CRLF""\
"		</select>"CRLF""\
"	</td>"CRLF""\
"</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SAFETY"', GetIDsVals('#pincodes input,#pincodes select', true));\">"CRLF



#define PAGE_SAFETY_HTML 	HTML_AHEADER""HTML_CONTENT_SAFETY""HTML_AFOOTER




#define HTML_CONTENT_AUTH	""\
"<h3>Введите логин и пароль</h3>"CRLF""\
"<table>"CRLF""\
"<tr><td>Логин:</td><td><input id='"KEY_INPUT_WEB_LOGIN"' value=''></td></tr>"CRLF""\
"<tr><td>Пароль:</td><td><input id='"KEY_INPUT_WEB_PASSWORD"' value='' type='password'></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Ввойти' onclick=\"SendRequest('POST', '"CGI_SCRIPT_AUTH"', GetIDsVals('#"KEY_INPUT_WEB_LOGIN",#"KEY_INPUT_WEB_PASSWORD"', true)); setTimeout(function(){location.href='index.html'} , 1000);\">"CRLF


#define PAGE_AUTH_HTML 	HTML_AHEADER""HTML_CONTENT_AUTH""HTML_AFOOTER





#define HTML_CONTENT_NETFUNC	""\
"<h3>Сетевые функции</h3>"CRLF""\
"<script type='text/javascript'>"CRLF""\
"	window.onload = function(){"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_NETFUNC"', GetIDsVals('#"KEY_LINES_COUNT",#nfs input,#nfs tr[class=bitfield],#nmchsa,#nmchsl', false));"CRLF""\
"  };"CRLF""\
"	function OnLinesCount(count) {"CRLF""\
"		for (let i = count; i <= 24; i++) {"CRLF""\
"			setColumnVisibility('nmchs', i, false);"CRLF""\
"			setColumnVisibility('nfs', i + 2, false);"CRLF""\
"		}"CRLF""\
"	}"CRLF""\
"</script>"CRLF""\
"<input id='"KEY_LINES_COUNT"' type='hidden'>"CRLF""\
"<h4>Реакция на пожарные входы</h4>"CRLF""\
"<table id='nfs'>"CRLF""\
"	<tr>"CRLF""\
"		<th>Адрес<br>ПУ</th>"CRLF""\
"		<th>Пожарный<br>вход</th>"CRLF""\
"		<th>Файл / Линия оповещения</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_1' class='bitfield'>"CRLF""\
"		<td><input id='nfa_1' size=3></td>"CRLF""\
"		<td><input id='nfl_1' size=3></td>"CRLF""\
"		<td><input id='nff_1'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_2' class='bitfield'>"CRLF""\
"		<td><input id='nfa_2' size=3></td>"CRLF""\
"		<td><input id='nfl_2' size=3></td>"CRLF""\
"		<td><input id='nff_2'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_3' class='bitfield'>"CRLF""\
"		<td><input id='nfa_3' size=3></td>"CRLF""\
"		<td><input id='nfl_3' size=3></td>"CRLF""\
"		<td><input id='nff_3'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_4' class='bitfield'>"CRLF""\
"		<td><input id='nfa_4' size=3></td>"CRLF""\
"		<td><input id='nfl_4' size=3></td>"CRLF""\
"		<td><input id='nff_4'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_5' class='bitfield'>"CRLF""\
"		<td><input id='nfa_5' size=3></td>"CRLF""\
"		<td><input id='nfl_5' size=3></td>"CRLF""\
"		<td><input id='nff_5'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_6' class='bitfield'>"CRLF""\
"		<td><input id='nfa_6' size=3></td>"CRLF""\
"		<td><input id='nfl_6' size=3></td>"CRLF""\
"		<td><input id='nff_6'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_7' class='bitfield'>"CRLF""\
"		<td><input id='nfa_7' size=3></td>"CRLF""\
"		<td><input id='nfl_7' size=3></td>"CRLF""\
"		<td><input id='nff_7'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_8' class='bitfield'>"CRLF""\
"		<td><input id='nfa_8' size=3></td>"CRLF""\
"		<td><input id='nfl_8' size=3></td>"CRLF""\
"		<td><input id='nff_8'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_9' class='bitfield'>"CRLF""\
"		<td><input id='nfa_9' size=3></td>"CRLF""\
"		<td><input id='nfl_9' size=3></td>"CRLF""\
"		<td><input id='nff_9'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nfs_10' class='bitfield'>"CRLF""\
"		<td><input id='nfa_10' size=3></td>"CRLF""\
"		<td><input id='nfl_10' size=3></td>"CRLF""\
"		<td><input id='nff_10'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_NETFUNC"', GetIDsVals('#nfs input,#nfs tr[class=bitfield]', true));\">"CRLF""\
"<br>"CRLF""\
"<h4>Реакция на МЧС</h4>"CRLF""\
"<table id='nmchs'>"CRLF""\
"	<tr>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<th>Адрес ПУ / Линия оповещения</th>"CRLF""\
"		<th>1</th>"CRLF""\
"		<th>2</th>"CRLF""\
"		<th>3</th>"CRLF""\
"		<th>4</th>"CRLF""\
"		<th>5</th>"CRLF""\
"		<th>6</th>"CRLF""\
"		<th>7</th>"CRLF""\
"		<th>8</th>"CRLF""\
"		<th>9</th>"CRLF""\
"		<th>10</th>"CRLF""\
"		<th>11</th>"CRLF""\
"		<th>12</th>"CRLF""\
"		<th>13</th>"CRLF""\
"		<th>14</th>"CRLF""\
"		<th>15</th>"CRLF""\
"		<th>16</th>"CRLF""\
"		<th>17</th>"CRLF""\
"		<th>18</th>"CRLF""\
"		<th>19</th>"CRLF""\
"		<th>20</th>"CRLF""\
"		<th>21</th>"CRLF""\
"		<th>22</th>"CRLF""\
"		<th>23</th>"CRLF""\
"		<th>24</th>"CRLF""\
"	</tr>"CRLF""\
"	<tr id='nmchsl' class='bitfield'>"CRLF""\
"		<td><input id='nmchsa' size=3></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"		<td><input type='checkbox'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_NETFUNC"', GetIDsVals('#nmchsa,#nmchsl', true));\">"CRLF


#define PAGE_NETFUNC_HTML 	HTML_AHEADER""HTML_CONTENT_NETFUNC""HTML_AFOOTER


#define HTML_CONTENT_ERROR_USER_LEVEL(x)	""\
"<h3>Не верный уровень доступа-"x"</h3>"CRLF""\
"Введите пин-код: <input id='pincode'>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_ERROR_USER_LEVEL"', GetIDsVals('#pincode', true));\">"CRLF

//#define PAGE_ERROR_USER_LEVEL_0 	HTML_AHEADER""HTML_CONTENT_ERROR_USER_LEVEL("0")""HTML_AFOOTER
#define PAGE_ERROR_USER_LEVEL_1 	HTML_AHEADER""HTML_CONTENT_ERROR_USER_LEVEL("1")""HTML_AFOOTER
#define PAGE_ERROR_USER_LEVEL_2 	HTML_AHEADER""HTML_CONTENT_ERROR_USER_LEVEL("2")""HTML_AFOOTER
//#define PAGE_ERROR_USER_LEVEL_3 	HTML_AHEADER""HTML_CONTENT_ERROR_USER_LEVEL("3")""HTML_AFOOTER
//#define PAGE_ERROR_USER_LEVEL_4 	HTML_AHEADER""HTML_CONTENT_ERROR_USER_LEVEL("4")""HTML_AFOOTER



/*
"<h4>Настройка логических номеров зон оповещения</h4>"CRLF""\
"<!-- line zone number --- line zone description -->"CRLF""\
"<table id='lz'>"CRLF""\
"	<tr><th>Линия вещания</th><th>Зона</th><th>Описание</th></tr>"CRLF""\
"	<tr><td>1</td><td><input id='lzn_1'></td><td><input id='lzd_1' size='50'></td></tr>"CRLF""\
"	<tr><td>2</td><td><input id='lzn_2'></td><td><input id='lzd_2' size='50'></td></tr>"CRLF""\
"	<tr><td>3</td><td><input id='lzn_3'></td><td><input id='lzd_3' size='50'></td></tr>"CRLF""\
"	<tr><td>4</td><td><input id='lzn_4'></td><td><input id='lzd_4' size='50'></td></tr>"CRLF""\
"	<tr><td>5</td><td><input id='lzn_5'></td><td><input id='lzd_5' size='50'></td></tr>"CRLF""\
"	<tr><td>6</td><td><input id='lzn_6'></td><td><input id='lzd_6' size='50'></td></tr>"CRLF""\
"	<tr><td>7</td><td><input id='lzn_7'></td><td><input id='lzd_7' size='50'></td></tr>"CRLF""\
"	<tr><td>8</td><td><input id='lzn_8'></td><td><input id='lzd_8' size='50'></td></tr>"CRLF""\
"	<tr><td>9</td><td><input id='lzn_9'></td><td><input id='lzd_9' size='50'></td></tr>"CRLF""\
"	<tr><td>10</td><td><input id='lzn_10'></td><td><input id='lzd_10' size='50'></td></tr>"CRLF""\
"	<tr><td>11</td><td><input id='lzn_11'></td><td><input id='lzd_11' size='50'></td></tr>"CRLF""\
"	<tr><td>12</td><td><input id='lzn_12'></td><td><input id='lzd_12' size='50'></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT"', GetIDsVals('#lz input', true));\">"CRLF""\

"<form method='POST' action='' enctype='multipart/form-data'>"CRLF""\
"	<input type='file' name='uploadfile' required><br><br>"CRLF""\
"	<input type='submit' value='Загрузить файл' />"CRLF""\
"</form>"CRLF""\
"<br><br>"CRLF""\

"	<tr><td>Кол-во линий:</td><td>"CRLF""\
"		<select id='"KEY_LINES_COUNT"'>"CRLF""\
"			<option value='12'>12</option>"CRLF""\
"			<option value='24'>24</option>"CRLF""\
"		</select>"CRLF""\
"	</td><tr>"CRLF""\



"<h4>Доступ к WEB интерфейсу</h4>"CRLF""\
"<table>"CRLF""\
"<tr><td>Логин:</td><td><input id='"KEY_WEB_LOGIN"' value=''></td></tr>"CRLF""\
"<tr><td>Пароль:</td><td><input id='"KEY_WEB_PASSWORD"' value='' type='password'></td></tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_SAFETY"', GetIDsVals('#"KEY_WEB_LOGIN",#"KEY_WEB_PASSWORD"', true));\">"CRLF""\

"	<tr><td>Название прибора:</td><td><input id='"KEY_NAME"' value=''></td><tr>"CRLF""\

"	function manual_update()  {"CRLF""\
"		SendRequest('GET', '"CGI_SCRIPT_MODE"', GetIDsVals('#manual_uo', true) + '&' + GetIDsVals('#mode_manual input', false));"CRLF""\
"	}"CRLF""\

"<h4>Режим Ручной ПУСК</h4>"CRLF""\
"<table><tr><td>Зацикливать последний активный УО:</td><td><input type='checkbox' id='manual_cycle'></td><tr></table>"CRLF""\
"<br>"CRLF""\
"Уровень оповещения:"CRLF""\
"<select id='manual_uo' onchange='manual_update();'>"CRLF""\
"	<option value='1'>УО-1</option>"CRLF""\
"	<option value='2'>УО-2</option>"CRLF""\
"	<option value='3'>УО-3</option>"CRLF""\
"	<option value='4'>УО-4</option>"CRLF""\
"	<option value='5'>УО-5</option>"CRLF""\
"	<option value='6'>УО-6</option>"CRLF""\
"	<option value='7'>УО-7</option>"CRLF""\
"	<option value='8'>УО-8</option>"CRLF""\
"	<option value='9'>УО-9</option>"CRLF""\
"	<option value='10'>УО-10</option>"CRLF""\
"</select>"CRLF""\
"<br>"CRLF""\
"<br>"CRLF""\
"<table id='mode_manual'>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Включено</td><td><input type='checkbox' id='manual_enable'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Задержка секунд</td><td><input id='manual_delay'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Кол-во повторов</td><td><input id='manual_repeat'></td>"CRLF""\
"	</tr>"CRLF""\
"	<tr>"CRLF""\
"		<td style='text-align:left;'>Файл</td><td><input id='manual_file'></td>"CRLF""\
"	</tr>"CRLF""\
"</table>"CRLF""\
"<br>"CRLF""\
"<input type='button' value='Сохранить' onclick=\"SendRequest('POST', '"CGI_SCRIPT_MODE"', GetIDsVals('#manual_uo', true) + '&' + GetIDsVals('#mode_manual input, #manual_cycle', true));\">"CRLF""\
"<br>"CRLF""\
CRLF""\

"			<option value='250'>250</option>"CRLF""\
"	<tr><td>Микрофон с фиксацией:</td><td><input type='checkbox' id='"KEY_MIC_FIX"'></td><tr>"CRLF""\

*/

#endif
