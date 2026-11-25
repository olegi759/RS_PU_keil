#define JSON_NO_STDIO
#define LIBJSON_IMPLEMENTATION
#include "libjson.h"
#include "ws_rubetec.h"
#include "ws_server.h"
#include <string.h>
#include "w5500.h"

//#include "ulog.h"
#include "stm_util.h"

#define RUBETEC_CH_REQUEST_BUFF_FULL 0XFFFF
#define RUBETEC_MAX_REQUEST_BUFF     16
#define RUBETEC_REQUEST_TIMEOUT_ms   6000
#define RUBETEC_REQUEST_MAX_LEN   (sizeof(((request_node_t *)0)->request_copy_buff) - 1)  /* 511 */
// typedef enum ch_state_e{
//     RUB_FREE = 0,
//     RUB_GET_DATA, 
//     RUB_WAITING_ANSWER,
//     RUB_BUSSY,
//     RUB_READY_ANSWER,
// }ch_state_e;

typedef struct request_node_t{
    uint8_t sock_num;
    uint32_t init_time;
    JsonSlice req_slice;
    uint32_t req_id;
    char request_copy_buff[512];
}request_node_t;

static uint16_t rub_reqs_mask = 0;
static request_node_t rub_reqs[RUBETEC_MAX_REQUEST_BUFF];

static char answer_buf[8192];
//static char request_copy_buff[512];
//static JsonSlice last_response_slice;
//static uint8_t last_sock_num = 0xFF;
//static bool has_parsed = false;          //если еще не ответили на запрос - мы заняты, новые команды не принимаем
//static bool has_answer_sended = false;   //был ли отправлен ответ

typedef bool (*cmd_handler_t)(request_node_t*);

typedef struct rubetec_cmd_entry_t{
    const char* name;
    cmd_handler_t handler;
}rubetec_cmd_entry_t;

static uint8_t push_req_node(uint8_t sock_num, char* req, uint32_t len);
static uint8_t get_req_indx_by_id(uint32_t req_id);

static bool try_parce_json_msg(request_node_t*);

static bool parse_uint8_slice(JsonSlice s, uint8_t *out);
static bool slice_is_literal(JsonSlice v);
static bool is_json_number(JsonSlice s);
static void write_typed_value(JsonWriter *w, const char *key, JsonSlice value);
static void copy_request_fields(JsonSlice request, JsonWriter *w);
static int hex_digit(char c);
static bool parse_hex_uint32_slice(JsonSlice s, uint32_t *out) __attribute__((unused));
static void write_hex_mask_kv(JsonWriter *w, const char *key, uint32_t value) __attribute__((unused));
static bool parse_pu_id_mask(JsonSlice s, uint32_t *out);
static void white_array_by_mask_kv(JsonWriter *w, const char *key, uint32_t value_mask);
static uint32_t convert_id_json(JsonSlice slice);

static void make_json_accept(JsonSlice request);
static void make_json_err(JsonSlice request, const char *error, const char *err_msg);
static void send_answer(uint8_t sock_num);

static bool handle_start_alert(request_node_t*);
static bool handle_reset(request_node_t*);
static bool handle_emergency_start(request_node_t*);
static bool handle_get_status(request_node_t*);
static cmd_handler_t GetHandle(JsonSlice);

static bool slice_eq_str(JsonSlice s, const char *str); 

static const rubetec_cmd_entry_t commands[] = {
    { "start_alert",      handle_start_alert     },
    { "reset",            handle_reset           },
    { "emergency_start",  handle_emergency_start },
    { "get_status",       handle_get_status      },
};

static const char* busy_json = "{\"status\":\"busy\"}";
static const char* toolarge_json = "{\"error\":\"too_large\", \"message\":\"request exceeds 511 bytes\"}";

// static bool sock_has_requests(uint8_t sn)
// {
//     for (uint8_t i = 0; i < RUBETEC_MAX_REQUEST_BUFF; ++i) {
//         if ((rub_reqs_mask & (1u << i)) && rub_reqs[i].sock_num == sn) return true;
//     }
//     return false;
// }

void rubetec_task(void)
{
    for (uint8_t j = 0; j < RUBETEC_MAX_REQUEST_BUFF; ++j) {
        if (!(rub_reqs_mask & (1u << j))) continue;
        if (HAL_GetTick() - rub_reqs[j].init_time <= RUBETEC_REQUEST_TIMEOUT_ms) continue;

        //uint8_t sn = rub_reqs[j].sock_num;
        rub_reqs_mask &= ~(1u << j);            /* запрос протух — слот освобождаем */
#ifdef RUBETEC_LOG
        ulog("ERROR: rubetec answer Timeout\r\n");
#endif

        make_json_err(rub_reqs->req_slice, "timeout", "answer data process timeout");
        send_answer(rub_reqs->sock_num);

        // if (sock_has_requests(sn)) continue;    /* на этом сокете ещё ждут другие */

        // if (getSn_SR(sn) != SOCK_ESTABLISHED) {
        //     ws_server_close(sn);                /* канал уже не тот — переоткрыть */
        // } else if (!ws_try_ping_pong(sn, rubetec_msg_handler, 100)) {
        //     ws_server_close(sn);                /* клиент не отвечает — рвём и переоткрываем */
        // }
    }
}

void rubetec_msg_handler(uint8_t sock_num, uint8_t* request_data, uint32_t data_len){
    if(rub_reqs_mask == RUBETEC_CH_REQUEST_BUFF_FULL){
        ws_send_data(sock_num, (uint8_t*)busy_json, strlen(busy_json), 0x1); 
        return;
    }
    if(data_len > RUBETEC_REQUEST_MAX_LEN){
        ws_send_data(sock_num, (uint8_t*)toolarge_json, strlen(toolarge_json), 0x1); 
        return;
    }
    uint8_t free_ind = RUBETEC_MAX_REQUEST_BUFF;
    //сохраняем текст сообщения в свободный слот 
    free_ind = push_req_node(sock_num, (char*)request_data, data_len);
    if(free_ind >= RUBETEC_MAX_REQUEST_BUFF) return;

    bool has_parsed = try_parce_json_msg(&rub_reqs[free_ind]);
    if(!has_parsed){
        //если данные не распарсили как json или была ошибка в json параметрах
        //мы либо ответили ошибкой, либо просто пропустили сообщение
        rub_reqs_mask &= ~(0x01 << free_ind); //просто сбросим бит в маске  
        return;
    } 
}

static uint8_t push_req_node(uint8_t sock_num, char* req, uint32_t len){
    if(rub_reqs_mask == RUBETEC_CH_REQUEST_BUFF_FULL) return 0xFF;
    
    JsonSlice slice = json_from_parts(req, len);
    JsonSlice req_id_slice;
    if (!json_get(slice, "request_id", &req_id_slice)) return 0xFF;

    //теперь ищем свободный слот
    for(uint8_t i = 0; i < RUBETEC_MAX_REQUEST_BUFF; ++i){
        if(rub_reqs_mask & (0x01 << i)) continue;
        
        rub_reqs[i].init_time = HAL_GetTick();
        rub_reqs[i].sock_num = sock_num;
        rub_reqs[i].req_id = convert_id_json(req_id_slice);
        memcpy(rub_reqs[i].request_copy_buff, req, len);
        rub_reqs[i].request_copy_buff[len] = '\0';
        rub_reqs[i].req_slice = json_from_parts(rub_reqs[i].request_copy_buff, len);
        rub_reqs_mask |= (0x01 << i);
        return i;
    }
    return 0xFF;
}

static uint8_t get_req_indx_by_id(uint32_t req_id){
    for(uint8_t i = 0; i < RUBETEC_MAX_REQUEST_BUFF; ++i){
        if(!(rub_reqs_mask & (0x01 << i))) continue;
        if(rub_reqs[i].req_id == req_id){
            rub_reqs_mask &= ~(0x01 << i);
            return i;
        }
    }
    return 0xFF;
}

static bool try_parce_json_msg(request_node_t* req_node){

    JsonSlice cmd_slice;
    if (json_get(req_node->req_slice, "cmd", &cmd_slice)){
        cmd_handler_t handler = GetHandle(cmd_slice);
        if(handler == 0) return false;
        return handler(req_node);
    }

    return false;
}

void rubetec_send_acept(uint32_t req_id){
    uint8_t req_indx = get_req_indx_by_id(req_id);
    if (req_indx >= RUBETEC_MAX_REQUEST_BUFF) return;
    make_json_accept(rub_reqs[req_indx].req_slice);
    send_answer(rub_reqs[req_indx].sock_num);
}

void rubetec_send_error(uint32_t req_id, const char* error, const char* err_msg){
    uint8_t req_indx = get_req_indx_by_id(req_id);
    if (req_indx >= RUBETEC_MAX_REQUEST_BUFF) return;
    make_json_err(rub_reqs[req_indx].req_slice, error, err_msg);
    send_answer(rub_reqs[req_indx].sock_num);
}

void rubetec_send_pu_states(uint32_t req_id, 
                            const char *timestamp,
                            rubetec_pu_state_t *pu_states,
                            uint8_t cnt)
{
    uint8_t req_indx = get_req_indx_by_id(req_id);
    if (req_indx >= RUBETEC_MAX_REQUEST_BUFF) return;

    JsonWriter w;
    json_writer_init(&w, answer_buf, sizeof(answer_buf));

    json_write_object_begin(&w);

    // response_to / request_id / pu_id_mask из исходного запроса
    copy_request_fields(rub_reqs[req_indx].req_slice, &w);

    // "devices": [ {...}, ... ]
    json_write_array_begin_k(&w, "devices");
    for (uint8_t i = 0; i < cnt; ++i) {
        rubetec_pu_state_t *pu = &pu_states[i];

        json_write_object_begin(&w);

        json_write_int_kv(&w, "pu_id", pu->pu_id);
        json_write_str_kv(&w, "mode",  pu->mode);

        white_array_by_mask_kv(&w, "active_inputs", pu->active_inputs_mask);
        white_array_by_mask_kv(&w, "active_zones",  pu->active_zones_mask);

        // "faults": [ {"type":..., "message":...}, ... ]
        // Одна структура rubetec_pu_fault_t = одна запись в массиве.
        json_write_array_begin_k(&w, "faults");
        for (uint8_t f = 0; f < RUBETEC_MAX_FAULTS; ++f) {
            rubetec_pu_fault_t *fault = &pu->faults[f];

            // Пустой слот: type == NULL или пустая строка
            if (fault->type == NULL || fault->type[0] == '\0') continue;

            json_write_object_begin(&w);
            json_write_str_kv(&w, "type",    fault->type);
            json_write_int_kv(&w, "param",    fault->param);
            json_write_str_kv(&w, "message", fault->message);
            json_write_object_end(&w);
        }
        json_write_array_end(&w);

        white_array_by_mask_kv(&w, "automation_disabled_mask", pu->automation_disabled_mask);
        white_array_by_mask_kv(&w, "outputs_disabled_mask",    pu->outputs_disabled_mask);

        json_write_object_end(&w);
    }
    json_write_array_end(&w);

    json_write_str_kv(&w, "timestamp", timestamp);

    json_write_object_end(&w);

    if (!json_writer_ok(&w)) {
#ifdef RUBETEC_LOG
        ulog("ERROR: answer_buf overflow\r\n");
#endif
        make_json_err(rub_reqs[req_indx].req_slice, "internal error", "answer_buf overflow");
    } 
    send_answer(rub_reqs[req_indx].sock_num);
}

static bool parse_uint8_slice(JsonSlice s, uint8_t *out)
{
    if (s.len == 0 || s.data == NULL) return false;

    uint32_t acc = 0;
    for (size_t i = 0; i < s.len; ++i) {
        char c = s.data[i];
        if (c < '0' || c > '9') return false;  // не цифра
        acc = acc * 10u + (uint32_t)(c - '0');
        if (acc > UINT8_MAX) return false;     // переполнение uint8_t
    }
    *out = (uint8_t)acc;
    return true;
}
// true/false/null — единственные литералы, начинающиеся с t/f/n.
// Проверяем точное совпадение, чтобы не спутать со строками вида "test".
static bool slice_is_literal(JsonSlice v) {
    return slice_eq_str(v, "true")  ||
           slice_eq_str(v, "false") ||
           slice_eq_str(v, "null");
}

static bool is_json_number(JsonSlice s)
{
    if (s.len == 0 || s.data == NULL) return false;
    size_t i = 0;

    if (s.data[i] == '-') i++;

    // int part
    if (i >= s.len) return false;
    if (s.data[i] == '0') {
        i++;
        // ведущий ноль не может продолжаться цифрой
        if (i < s.len && s.data[i] >= '0' && s.data[i] <= '9') return false;
    } else if (s.data[i] >= '1' && s.data[i] <= '9') {
        while (i < s.len && s.data[i] >= '0' && s.data[i] <= '9') i++;
    } else {
        return false;
    }

    // frac part
    if (i < s.len && s.data[i] == '.') {
        i++;
        if (i >= s.len || s.data[i] < '0' || s.data[i] > '9') return false;
        while (i < s.len && s.data[i] >= '0' && s.data[i] <= '9') i++;
    }

    // exp part
    if (i < s.len && (s.data[i] == 'e' || s.data[i] == 'E')) {
        i++;
        if (i < s.len && (s.data[i] == '+' || s.data[i] == '-')) i++;
        if (i >= s.len || s.data[i] < '0' || s.data[i] > '9') return false;
        while (i < s.len && s.data[i] >= '0' && s.data[i] <= '9') i++;
    }

    return i == s.len;   // токен израсходован полностью
}

// Пишет пару "ключ": значение, сохраняя тип значения.
// key — уже готовая C-строка (выходной ключ).
static void write_typed_value(JsonWriter *w, const char *key, JsonSlice value)
{
    json_write_key(w, key);

    if (value.len == 0 || value.data == NULL) {
        json_write_null(w);
        return;
    }

    unsigned char first = (unsigned char)value.data[0];
    bool is_container = (first == '{') || (first == '[');
    bool is_quoted    = (first == '"');

    if (is_json_number(value) || is_container || is_quoted || slice_is_literal(value)) {
        json_write_raw(w, value.data, value.len);   // уже корректный JSON-токен
    } else {
        json_write_strn(w, value.data, value.len);  // всё остальное — строкой
    }
}

// Копирует все поля запроса, переименовывая "cmd" -> "response_to".
// Без проверок: считается, что ключ всегда помещается в key_buf.
static void copy_request_fields(JsonSlice request, JsonWriter *w)
{
    JsonObjectIter iter;
    json_object_iter_init(request, &iter);

    JsonSlice key, value;
    char key_buf[64]; // с запасом под любой ключ из спецификации

    while (json_object_iter_next(&iter, &key, &value)) {
        json_slice_copy(key, key_buf, sizeof(key_buf)); // результат игнорируем
        const char *out_key = slice_eq_str(key, "cmd") ? "response_to" : key_buf;
        write_typed_value(w, out_key, value);
    }
}

// Преобразует один hex-символ в значение 0..15. Возвращает -1 при ошибке. 
static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Разбирает hex-строку из JsonSlice в uint32_t.
// Поддерживает необязательный префикс "0x" / "0X" и любое число ведущих нулей:
//   "0x00000003", "0x03", "0x3", "3", "0XFFFFFFFF" — всё корректно.
// Возвращает true при успехе; *out не меняется при ошибке.
static bool parse_hex_uint32_slice(JsonSlice s, uint32_t *out)
{
    if (s.len == 0 || s.data == NULL) return false;

    size_t i = 0;
    // Пропускаем необязательный префикс 0x / 0X
    if (s.len >= 2 && s.data[0] == '0' && (s.data[1] == 'x' || s.data[1] == 'X')) {
        i = 2;
    }
    if (i >= s.len) return false; // после префикса ничего нет

    uint32_t acc = 0;
    for (; i < s.len; ++i) {
        int d = hex_digit(s.data[i]);
        if (d < 0) return false;

        // Проверка переполнения ДО сдвига:
        // если acc > 0x0FFFFFFF, то acc << 4 уже не поместится в uint32_t.
        if (acc > (UINT32_MAX >> 4)) return false;
        acc = (acc << 4) | (uint32_t)d;
    }
    *out = acc;
    return true;
}

static void write_hex_mask_kv(JsonWriter *w, const char *key, uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[12];                          // "0x" + до 8 цифр + '\0'
    char *p = buf + sizeof(buf) - 1;
    *p = '\0';

    int digits = 0;
    do {
        *--p = hex[value & 0xF];
        value >>= 4;
        digits++;
    } while (value > 0 || digits < 2);

    *--p = 'x';
    *--p = '0';

    json_write_key(w, key);
    json_write_str(w, p);                  // пишет с кавычками
}

// Разбирает "pu_id_mask" в битовую маску uint32_t.
// Поддерживает:
//   - массив:  [1, 2, 3]  -> 0x00000007 (ПУ N -> бит N-1)
//   - число 0:            -> 0xFFFFFFFF (все ПУ выбраны)
//   - число N (1..32):    -> 0x1 << (N-1) (одно ПУ)
// Ведущие пробелы/табы/CR/LF игнорируются.
static bool parse_pu_id_mask(JsonSlice s, uint32_t *out)
{
    if (s.data == NULL || s.len == 0) return false;

    // 1. Пропуск ведущих whitespace (корректное условие — &&)
    size_t i = 0;
    while (i < s.len &&
           (s.data[i] == ' '  || s.data[i] == '\t' ||
            s.data[i] == '\r' || s.data[i] == '\n')) {
        i++;
    }
    if (i >= s.len) return false;            // только пробелы — не значение

    JsonSlice trimmed = { s.data + i, s.len - i };

    // 2. Массив [1, 2, 3]
    if (trimmed.data[0] == '[') {
        JsonArrayIter iter;
        json_array_iter_init(trimmed, &iter);

        JsonSlice item;
        uint32_t mask  = 0;
        size_t   count = 0;

        while (json_array_iter_next(&iter, &item)) {
            uint8_t id;
            if (!parse_uint8_slice(item, &id)) return false;
            if (id < 1 || id > 32)            return false;

            mask |= (uint32_t)1u << (id - 1);
            count++;
        }
        if (count == 0) return false;        // пустой массив []
        *out = mask;
        return true;
    }

    // 3. Одиночное число: 0 или 1..32
    uint8_t val;
    if (!parse_uint8_slice(trimmed, &val)) return false;

    if (val == 0) {
        // 0 = выбрать все ПУ (1..32)
        *out = 0;
        for (uint8_t b = 0; b < MAX_PU_NUMBER; ++b) {
            *out |= (uint32_t)1u << b;
        }
        // Эквивалент: *out = 0xFFFFFFFFu;
        return true;
    }
    if (val > 32) return false;

    *out = (uint32_t)1u << (val - 1);        // одно ПУ -> один бит
    return true;
}

static void white_array_by_mask_kv(JsonWriter *w, const char *key, uint32_t value_mask){
    json_write_array_begin_k(w, key);
    for(uint8_t i = 0; i < 32; ++i){
        if( (value_mask & ((uint32_t)1 << i)) ){
             json_write_int(w, i + 1);
        }
    }
    json_write_array_end(w);
}

uint32_t convert_id_json(JsonSlice slice)
{
    if (slice.data == NULL || slice.len == 0) {
        return 0;
    }

    const char *end = slice.data + slice.len;
    const char *start = NULL;

    /* Ищем последнее '_' */
    for (size_t i = slice.len; i > 0; --i) {
        if (slice.data[i - 1] == '_') {
            start = slice.data + i;
            break;
        }
    }

    /* Если '_' нет — ищем первую цифру */
    if (start == NULL) {
        start = slice.data;
        while (start < end && (*start < '0' || *start > '9')) {
            ++start;
        }
    }

    if (start >= end) {
        return 0; /* цифр не найдено */
    }

    uint32_t value = 0;
    size_t digits = 0;

    for (const char *p = start; p < end; ++p) {
        if (*p < '0' || *p > '9') {
            return 0; /* нецифровой символ */
        }

        if (++digits > 6) {
            return 0; /* больше 6 цифр */
        }

        value = value * 10u + (uint32_t)(*p - '0');
    }

    return value;
}

// Успешный ответ: поля запроса + "status":"accepted"
static void make_json_accept(JsonSlice request)
{
#ifdef RUBETEC_LOG
    ulog_fmt("ACCEPT: req.len=%u req.first=0x%02X req.data=%p answer_buf=%p\r\n",
           (unsigned)request.len,
           request.len ? (unsigned char)request.data[0] : 0,
           (void*)request.data,
           (void*)answer_buf);
#endif
    JsonWriter w;
    json_writer_init(&w, answer_buf, sizeof(answer_buf));

    json_write_object_begin(&w);

    copy_request_fields(request, &w);
    json_write_str_kv(&w, "status", "accepted");

    json_write_object_end(&w);
}
// Ответ с ошибкой: поля запроса + status/error/message
static void make_json_err(JsonSlice request, const char *error, const char *err_msg)
{
    JsonWriter w;
    json_writer_init(&w, answer_buf, sizeof(answer_buf));

    json_write_object_begin(&w);

    copy_request_fields(request, &w);
    json_write_str_kv(&w, "status",  "rejected");
    json_write_str_kv(&w, "error",   error);
    json_write_str_kv(&w, "message", err_msg);

    json_write_object_end(&w);
}

static void send_answer(uint8_t sock){
#ifdef RUBETEC_LOG
    ulog(answer_buf);
#endif
    ws_send_data(sock, (uint8_t*)answer_buf, strlen(answer_buf), 0x01);
}

static bool handle_start_alert(request_node_t* req_node){
    
    //собираем поля сообщения
    // JsonSlice cmd_slice;
    // if (!json_get(req_node->req_slice, "cmd", &cmd_slice)) return false;
    
    // JsonSlice id_slice;
    // if (!json_get(req_node->req_slice, "request_id", &id_slice)) return false;
    
    JsonSlice pu_id_slice; // Номер ПУ в сети (1..32)
    if (!json_get(req_node->req_slice, "pu_id", &pu_id_slice)) return false;
    
    JsonSlice input_slice; // Номер тревожного входа этого ПУ (1..24)
    if (!json_get(req_node->req_slice, "input", &input_slice)) return false;

    //забираем значения полей структур и проверяем их
    uint8_t pu_id = 0;
    if (!parse_uint8_slice(pu_id_slice, &pu_id)) {
        rubetec_send_error(req_node->req_id, "param_err", "internal pu_id");
        return false;
    }
    if(pu_id == 0 || pu_id > 24) {
        rubetec_send_error(req_node->req_id, "param_err", "pu_id должен быть от (1..24)");
        return false;
    }  
    uint8_t input_line_id = 0;
    if (!parse_uint8_slice(input_slice, &input_line_id)) {
        rubetec_send_error(req_node->req_id, "param_err", "internal input");
        return false;
    }
    if(input_line_id == 0 || input_line_id > 24) {
        rubetec_send_error(req_node->req_id, "param_err", "input должен быть от (1..24)");
        return false;
    }

    rubetec_on_start_alert_rec(req_node->req_id, pu_id, input_line_id);
    return true;
}

__attribute__((weak)) void rubetec_on_start_alert_rec(uint32_t msg_id, uint8_t pu_id, uint8_t input_line){

}

static bool handle_reset(request_node_t* req_node)
{
    //собираем поля сообщения
    // JsonSlice cmd_slice;
    // if (!json_get(json_slice, "cmd", &cmd_slice)) return false;

    // JsonSlice id_slice;
    // if (!json_get(json_slice, "request_id", &id_slice)) return false;

    JsonSlice pu_id_slice;
    if (!json_get(req_node->req_slice, "pu_id", &pu_id_slice)) return false;

    //забираем значения полей структур и проверяем их
    uint32_t pu_id = 0;
    if (!parse_pu_id_mask(pu_id_slice, &pu_id)) {
        rubetec_send_error(req_node->req_id, "param_err",
                      "pu_id должен быть массивом номеров ПУ (1..24)");
        return false;
    }
//    if (pu_id == 0) {
//        make_json_err(json_slice, "param_err", "ни один ПУ не выбран");
//        send_answer();
//        return true;
//    }

    rubetec_on_reset_rec(req_node->req_id, pu_id);
    return true;
}

__attribute__((weak)) void rubetec_on_reset_rec(uint32_t msg_id, uint32_t pu_id_mask){

}

static bool handle_emergency_start(request_node_t* req_node){
    //собираем поля сообщения
    // JsonSlice cmd_slice;
    // if (!json_get(json_slice, "cmd", &cmd_slice)) return false;
    
    // JsonSlice id_slice;
    // if (!json_get(json_slice, "request_id", &id_slice)) return false;
    
    JsonSlice pu_id_slice;
    if (!json_get(req_node->req_slice, "pu_id", &pu_id_slice)) return false;

    //забираем значения полей структур и проверяем их
    uint32_t pu_id = 0;
    if (!parse_pu_id_mask(pu_id_slice, &pu_id)) {
        rubetec_send_error(req_node->req_id, "param_err",
                      "pu_id должен быть массивом номеров ПУ (1..24)");
        return false;
    }
//    if (pu_id == 0) {
//        make_json_err(json_slice, "param_err", "ни один ПУ не выбран");
//        send_answer();
//        return true;
//    }

    rubetec_on_emergency_start(req_node->req_id, pu_id);
    return true;
}

__attribute__((weak)) void rubetec_on_emergency_start(uint32_t msg_id, uint32_t pu_id_mask){

}

static bool handle_get_status(request_node_t* req_node){
     //собираем поля сообщения
    // JsonSlice cmd_slice;
    // if (!json_get(json_slice, "cmd", &cmd_slice)) return false;
    
    // JsonSlice id_slice;
    // if (!json_get(json_slice, "request_id", &id_slice)) return false;
    
    JsonSlice pu_id_slice;
    if (!json_get(req_node->req_slice, "pu_id", &pu_id_slice)) return false;

    //забираем значения полей структур и проверяем их
    uint32_t pu_id = 0;
    if (!parse_pu_id_mask(pu_id_slice, &pu_id)) {
        rubetec_send_error(req_node->req_id, "param_err",
                      "pu_id должен быть массивом номеров ПУ (1..24)");
        return false;
    }
//    if (pu_id == 0) {
//        make_json_err(json_slice, "param_err", "ни один ПУ не выбран");
//        send_answer();
//        return true;
//    }

    rubetec_on_get_status(req_node->req_id, pu_id);
    return true;
}

__attribute__((weak)) void rubetec_on_get_status(uint32_t msg_id, uint32_t pu_id_mask){

}

static cmd_handler_t GetHandle(JsonSlice cmd_slice){
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (slice_eq_str(cmd_slice, commands[i].name)) {
            return commands[i].handler;
        }
    }
    return 0;
}

static bool slice_eq_str(JsonSlice s, const char *str) {
   size_t len = strlen(str);
   return (s.len == len) && (s.data != NULL) && (memcmp(s.data, str, len) == 0);
}