#ifndef WS_RUBETEC_H
#define WS_RUBETEC_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//************описание библиотеки web socket с Рубетек
//
// 1. проинициализировать web socket: ws_server_init(uint8_t ws_sock), где ws_sock - номер сокета
//    я выделял 2Кб на rx и tx буфферы у W5500
// 2. в цикле вызвать ws_server_poll(void) - ОПРОС ВЕБ СОКЕТА
// 3. переопределить _weak функции rubetec_on_start_alert_rec, rubetec_on_reset_rec, rubetec_on_emergency_start, rubetec_on_get_status
//    это колбеки, вызываются после парсинга команды от рубетек
// 4. где-то в коде или в _weak функциях выше, согласно протоколу вызывать rubetec_send_acept или rubetec_send_error или rubetec_send_pu_states
//    только на успешный ответ команды 4.1.	Запрос состояния  нужно использовать rubetec_send_pu_states
//    в остальных случаях это либо rubetec_send_acept либо rubetec_send_error
// 5. отключи логи, закоменти #define RUBETEC_LOG
//************//

#define RUBETEC_LOG

#define MAX_PU_NUMBER              32
#define RUBETEC_MAX_FAULTS         5

void rubetec_msg_handler(uint8_t sock_num, uint8_t* request_data, uint32_t data_len);

void rubetec_task();

typedef struct rubetec_pu_fault_t{
    char* type;                            // тип ошибки
    uint8_t param;                         // дополнительный параметр (0 - нет параметра)
    char* message;                         // доп описание, например: Неисправность линии речевого оповещения (зона) №8
}rubetec_pu_fault_t;

typedef struct rubetec_pu_state_t {
    uint8_t pu_id;
    char* mode;                             // Режим – например Тревога пожар 
    uint32_t active_inputs_mask;            // активный тревожный вход (номер бита в маске отвечает за вход)
    uint32_t active_zones_mask;             // активные линии речевого оповещения (номер бита в маске отвечает за линию)
    rubetec_pu_fault_t faults[RUBETEC_MAX_FAULTS];  // ошибки ПУ (кол-во выбирается #define MAX_FAULTS    5)
    uint32_t automation_disabled_mask;      // отключена автоматика по входам сигнализации (номер бита в маске отвечает за линию)
    uint32_t outputs_disabled_mask;         // отключенные линии/зоны оповещения (номер бита в маске отвечает за зону)
}rubetec_pu_state_t;


//ответы на команду 
////успешный ответ на последний запрос
void rubetec_send_acept(uint32_t msg_id);
////не успешный на ответ на последний запрос 
//error - тип ошибки. err_msg - доп описание
void rubetec_send_error(uint32_t msg_id, const char* error, const char* err_msg);
////ответ на запрос состояний ПУ
//timestamp - время. pu_states - массив ПУ запрошенных состояний. cnt - размер этого массива
void rubetec_send_pu_states(uint32_t msg_id, const char* timestamp, rubetec_pu_state_t* pu_states, uint8_t cnt);


//// weak функции которые нужно переопределить !!!!

////Команда запуска оповещения
//pu_id - Номер ПУ в сети (1..32); input_line - Номер тревожного входа этого ПУ (1..24)
void rubetec_on_start_alert_rec(uint32_t msg_id, uint8_t pu_id, uint8_t input_line);
////Команда Сброс оповещения (переход в дежурный режим)
// pu_id_mask - каждый бит отвечает за одно пу, допустим 0x03 – выбран первый и второй ПУ, 0xFFFFFFFF – выбраны все ПУ
void rubetec_on_reset_rec(uint32_t msg_id, uint32_t pu_id_mask);
////Команда Экстренный запуск оповещение в случае террористической опасности
// pu_id_mask - каждый бит отвечает за одно пу в которых нужно запустить оповещение 
void rubetec_on_emergency_start(uint32_t msg_id, uint32_t pu_id_mask);
////Команда Запрос текущего состояния
// pu_id_mask - каждый бит отвечает за одно пу, допустим 0x03 – выбран первый и второй ПУ, 0xFFFFFFFF – выбраны все ПУ
void rubetec_on_get_status(uint32_t msg_id, uint32_t pu_id_mask);
#endif