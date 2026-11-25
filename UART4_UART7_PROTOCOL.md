# UART4 и UART7 в pu_new8: структура обмена

Область: CM7 (STM32H747) проекта `pu_new8`.
Основные файлы: `CM7/Core/Src/uarts.c`, `CM7/Core/Src/pd.c`, `Common/common.c`,
`Common/pd_data.h`, `CM7/Core/Src/cm7core.c`.
Состояние кода — рабочая папка на 23.11.2026 (включая третью линию I2C2).

---

## 0. Коротко

* **UART4** — линия 1 командного кольца панелей («влево»), **UART7** — линия 2 («вправо»).
  В тех же проводах идут звуковые линии UART5/UART8 (левый/правый звук), но это
  отдельный обмен на 230400.
* Кадр — **ПИВ**: `[daddr][saddr][ID 2 байта][cmd][data…][CRC16 2 байта]`,
  115200 8N1, CRC-16/CCITT-FALSE. Минимум 7 байт.
* Каждая панель — **шлюз кольца**: чужой кадр передаётся в остальные линии,
  свой (saddr = локальный адрес) снимается и обновляет `rcv_ring_tick`,
  кадр «моим ПД» (daddr = local+1 / local+2) уходит в CM4,
  кадр «мне» (daddr = local) или широковещательный (0xFF) обрабатывается на месте.
* Третья линия кольца — **I2C2** (`UART_LINE_I2C2 = 9`) через CM4 (добавлена 22.11.2026).
* Периодика: PING по одной линии в секунду (каждая линия — раз в 3 с),
  контроль кольца — раз в 59 с.
* Важно: в кольце байты 2–3 — **счётчик пакета**, а на границе с ПД они
  заменяются на **длину данных** (диалект ПИВ со стороны ПД).

---

## 1. Роли линий и топология

```
                    кольцо панелей (ПУ)

   UART4 (линия 1) ──►  панель N  ──► UART4 ──► панель N+1 ...
          ▲                  │
          │            uart4_uart7_rcv()
          │            (шлюз: чужие кадры — в другие линии)
          │                  │
   UART7 (линия 2) ◄──  панель N  ◄── UART7 ◄── ...
```

* UART4 и UART7 равноправны: `uart4_uart7_snd()` шлёт один и тот же кадр
  во все линии кольца (UART4, UART7, I2C2), а `uart4_uart7_snd_others()`
  передаёт кадр во все линии, **кроме** той, откуда он пришёл.
* Кадр, отправленный панелью, возвращается к ней по кольцу; признак возврата —
  `packet[1] == setting.uart_local_addr` (saddr = свой адрес). Это же событие
  обновляет `rcv_ring_tick[]` — основу контроля кольца.
* Локальные ПД панели (адреса `local+1`, `local+2`) обслуживаются через CM4:
  `CORE_CMD_PD_SEND` → CM4 → UART6 через мультиплексор (P1C/P2C/EXT_AMP/TMP),
  альтернативно — отдельным каналом I2C2. Поэтому кадр «моим ПД» не идёт в
  UART4/7, а отдаётся в CM4.
* Локальный звук от ПД1/ПД2 приходит по UART1/UART3 — к командному кольцу
  UART4/7 отношения не имеет.

> Нумерация линий в этом документе — по UART панели (линия 1 = UART4,
> линия 2 = UART7). В документации ModulInterface «линия 1» — это его UART5,
> «линия 2» — I2C1; это другая нумерация.

Логика одной панели как шлюза:

```
UART4 ──►┌─────────────────────┐──► UART7
         │ uart4_uart7_rcv()   │──► I2C2   (через CM4)
UART7 ──►│                     │──► CM4    (если daddr = local+1/+2)
         │                     │──► pd_rcv (если daddr = local или 0xFF)
         └─────────────────────┘
```

---

## 2. Настройка UART

| Параметр | UART4 (линия 1) | UART7 (линия 2) |
|---|---|---|
| Экземпляр | `huart4` | `huart7` |
| TX / RX | PA0 (`EXT1_CONT_TX`) / PC11 (`EXT1_CONT_RX`), AF8 | PE8 (`EXT2_CONT_TX`) / PE7 (`EXT2_CONT_RX`), AF7 |
| Формат | 115200, 8 бит, 1 стоп, без чётности, без flow control, oversampling 16 | то же |
| Тактирование | D2PCLK1 (`RCC_PERIPHCLK_UART4`) | D2PCLK1 (`RCC_PERIPHCLK_UART7`) |
| FIFO | выключено, пороги TX/RX 1/8 | то же |
| Передача | `HAL_UART_Transmit_DMA`, DMA1 Stream3 (`DMA_REQUEST_UART4_TX`), NORMAL, байт/байт | DMA1 Stream7 (`DMA_REQUEST_UART7_TX`) |
| Приём | `HAL_UARTEx_ReceiveToIdle_IT` (IRQ), DMA1 Stream2 инициализирован, но **не используется** | то же, DMA1 Stream6 |
| NVIC | `UART4_IRQn`, приоритет 0 | `UART7_IRQn`, приоритет 0 |
| RX-буфер | `ext1_count_rx_buf[128]` | `ext2_count_rx_buf[128]` |
| TX-буфер | `ext1_count_tx_buf[128]` | `ext2_count_tx_buf[128]` |
| Номер линии | 4 | 7 |

Инициализация: `MX_UART4_Init()` / `MX_UART7_Init()` в `main.c`, MSP — в
`stm32h7xx_hal_msp.c` (UART4 — строка ~251, UART7 — ~415).
Старт приёма — `uarts_init()` (`uarts.c:1035`), она же сбрасывает линии и
выключает выходы звука (`UART_P1_SOUND_EN`, `UART_P2_SOUND_EN`).

Максимальный кадр ограничен буфером 128 байт (`#define MAX_UART_PACKET_SIZE 124`
в `uarts.h` не используется). При передаче кадр обрезается до `tx_buf_size`
(`uarts_transmit_it_by_param`). При приёме номер линии дописывается в
`rx_buf[Size]`, поэтому безопасная длина кадра — не более 127 байт.

---

## 3. Формат кадра (ПИВ)

### 3.1 Байты

| Смещение | Поле | Размер | Примечание |
|---|---|---|---|
| 0 | `daddr` | 1 | адрес получателя |
| 1 | `saddr` | 1 | адрес отправителя |
| 2–3 | ID / длина | 2 | big-endian. В кольце — ID пакета (`count_tx_id`), в диалекте ПД — длина данных (`1 + cmd_data_len`) |
| 4 | `cmd` | 1 | код команды |
| 5…N−3 | `data` | 0…121 | данные команды |
| N−2, N−1 | CRC16 | 2 | big-endian, CRC-16/CCITT-FALSE по байтам 0…N−3 |

`N = 7 + cmd_data_len`; поле «длина» в диалекте ПД равно `1 + cmd_data_len`,
то есть `N = 6 + length`.

Структура-описание — `pu_packet_cmd_t` (`Common/common.h:126`, размер 5 байт),
но в коде кадр разбирается по индексам, структура при разборе не используется.

### 3.2 CRC

* Полином 0x1021, начальное значение 0xFFFF, без отражения, без финального XOR
  (CRC-16/CCITT-FALSE). Табличная реализация — `Crc16()` (`util/math_util.c:58`).
* `fill_crc16(packet, len)` пишет CRC в последние 2 байта (`common.c:470`).
* `is_correct_crc16(packet, len)` читает последние 2 байта как big-endian и
  сравнивает (`common.c:555`).
* `is_correct_datalen()` (`common.c:564`) проверяет `6 + length == packet_len` —
  применимо только к диалекту ПД, для кольца не используется (там байты 2–3 — ID).

### 3.3 Два диалекта одного формата

Геометрия кадра у кольца и у ПД одинаковая, но байты 2–3 означают разное:

| | Кольцо (UART4/UART7, I2C2) | ПД (CM4 ↔ ПД, I2C2-слейв) |
|---|---|---|
| Байты 2–3 | ID пакета (`count_tx_id`), big-endian | длина данных (`1 + cmd_data_len`), big-endian |
| Формируется на ПУ | `create_piv_packet_cmd2()` / `uart4_uart7_snd_piv()` | `create_piv_packet_cmd()` (кладёт `ID = 1 + cmd_data_len`) |
| PING | 0x3F | 0x55 (сторона ПД) |
| Адрес ПУ | 10, 20, … | 0x0A |
| Адрес ПД | `local+1`, `local+2` | 0x01 |

Преобразование делает панель на границе:

* **кольцо → ПД** (кадр к моим ПД, `uarts.c:570-587`):
  `daddr -= local`; `packet[2..3] = packet_len - 6` (= длина); пересчёт CRC;
  `cm7core_pd_send()` → `CORE_CMD_PD_SEND` (51) → CM4 → ПД;
* **ПД → кольцо** (`cm7core.c:294-310`, `CORE_CMD_PD_RECV` = 52):
  кадр лежит в `packet[1..]`; `packet[2] += local` (saddr), `packet[3..4] = ++count_tx_id`
  (это байты 2–3 ПИВ), CRC по `&packet[1]`, затем `uart4_uart7_snd(&packet[1], …)` —
  кадр уходит во все линии кольца.

Именно поэтому `create_piv_packet_cmd()` намеренно кладёт в ID значение
`1 + cmd_data_len`: одноразовые кадры (PING, кадры в ПД) оказываются
совместимы с ПИВ-диалектом ПД без пересчёта.

### 3.4 Примеры (CRC рассчитаны)

| Что | Байты |
|---|---|
| PING от ПУ 10 (ID = 1) | `0A 0A 00 01 3F CB 84` |
| DEJURNIY к ПУ 20, ID = 5 | `14 0A 00 05 42 63 88` |
| CHANGE_MODE (0xFF), ID = 0x13: адрес 10, режим 1, 12 линий, линии 0 | `FF 0A 00 13 39 0A 01 0C 00 00 00 1D 24` |
| VESHANIE_PD к ПУ 20, ID = 7: 16000 Гц, моно, линия 1 | `14 0A 00 07 41 00 00 3E 80 00 01 00 00 00 00 00 01 27 A1` |

В последнем кадре поле `id` (4 байта в конце данных) в прошивке не заполняется —
см. п. 10.

---

## 4. Адресация

| Адрес | Кто это |
|---|---|
| `setting.uart_local_addr` | локальный адрес панели, по умолчанию **10** (`CM4/Core/Src/setting.c:290`); задаётся в настройках, кратен 10 |
| 10, 20, 30, … 250 | панели ПУ (шаг 10) |
| `local+1`, `local+2` | ПД1 и ПД2 этой панели (на кольце) |
| `0xFF` | широковещательный кадр |
| 0x01 | адрес ПД в диалекте ПД (сторона ModulInterface) |
| 0x0A | адрес ПУ в диалекте ПД |

Таблица соседей: `pu_small_infos[26]`, `pu_small_infos_tick[26]`, индекс —
`addr / 10`. Запись живёт 90 с (`pu_get_data_from_small_infos`, `pu.c:122`);
обновляется широковещательными `PU_CMD_CHANGE_MODE`. Обработчик `PD_CMD_SPISOK_PU`
перед ответом вносит в таблицу только самого себя (`pu_fill_local_small_infos`),
поэтому ответ содержит «свои» записи плюс всё, что успело прийти по 0x39.

`pu_netinfo[3]` — 3 байта «сетевого вещания», приходят широковещательной
командой `PU_CMD_NET_INFO_UPDATE` (0x40) и отдаются в INFO_RES.

---

## 5. Обработка принятого кадра

### 5.1 Таблица решений `uart4_uart7_rcv(line, packet, len)`

| Условие | Действие |
|---|---|
| CRC неверна | кадр отбрасывается |
| `packet[1] == local` (saddr — свой) | свой кадр вернулся по кольцу: `rcv_ring_tick[line] = HAL_GetTick()`, выход |
| `packet[0] == local+1` или `local+2` | кадр моим ПД: фильтр дублей → `daddr -= local` → `packet[2..3] = len-6` → CRC → `cm7core_pd_send()` (CM4), выход |
| `packet[0] != local` и `!= 0xFF` | кадр не мне: `uart4_uart7_snd_others(line, …)` — в две другие линии, выход |
| `packet[0] == 0xFF` | фильтр дублей; `cmd`: 0x39 → `uart4_uart7_recv_change_mode()`; 0x40 → `pu_netinfo[0..2]`; затем `snd_others()` |
| `packet[0] == local` | фильтр дублей; `pd_rcv(packet, len, line)` — обработка команды и ответ |
| иначе (`0xFF`, не обработан) | `snd_others()` |

Псевдокод (`uarts.c:541`):

```c
if (!is_correct_crc16(packet, packet_len)) return;

if (packet[1] == setting.uart_local_addr) {       // свой кадр по кольцу
    rcv_ring_tick[huart_number] = HAL_GetTick();
    return;
}

if (packet[0] == local + 1 || packet[0] == local + 2) {  // моим ПД
    ... dedup ...
    packet[0] -= local;
    packet[2] = (packet_len - 6) >> 8;
    packet[3] = (packet_len - 6) & 0xFF;
    fill_crc16(packet, packet_len);
    cm7core_pd_send(packet, packet_len);
    return;
}

if (packet[0] != local && packet[0] != 0xFF) {    // транзит
    uart4_uart7_snd_others(huart_number, packet, packet_len);
    return;
}

... dedup ...
if (packet[0] == 0xFF) { ... 0x39 / 0x40 ... }
if (packet[0] == local) pd_rcv(packet, packet_len, huart_number);
else uart4_uart7_snd_others(huart_number, packet, packet_len);
```

### 5.2 Фильтр дублей

Один общий счётчик `count_rx_id` на все линии и всех отправителей:

```c
rx_id = (packet[2] << 8) | packet[3];
if (rx_id) {
    if (rx_id <= count_rx_id && (count_rx_id - rx_id) < 100) return; // дубль
} else {
    if (!count_rx_id) return;
}
count_rx_id = rx_id;
```

То есть кадр, ID которого не больше последнего принятого и отстаёт от него
меньше чем на 100, считается дублем. Транзитные кадры фильтр не проходят — они
всегда передаются дальше; фильтр стоит только на ветках «моим ПД», «0xFF» и «мне».

### 5.3 Ответ

`pd_rcv(packet, len, huart_number)` для команд отвечает через
`pd_transmit_piv(daddr = packet[1], saddr = packet[0], <ответ>, …, huart_number)`:

* `huart_number != 0` → `uarts_transmit_piv(line, …)` — ответ уходит **только в ту
  линию, откуда пришёл запрос** (обратный путь по кольцу), с ID `++count_tx_id`;
* `huart_number == 0` (кадр пришёл от ПД через CM4) → `cm7core_pd_send()` —
  ответ уходит в ПД через CM4, без кольца.

---

## 6. Приём и передача: конвейер и функции

### 6.1 Приём

```
UART4/UART7 IRQ + IDLE
  → HAL_UARTEx_RxEventCallback()            main.c:156
  → HAL_UARTS_RxEventCallback()             uarts.c:296   (Size < 7 — отбросить;
                                                            rx_buf[Size] = номер линии;
                                                            push в uarts_rx_buf, 60 КБ)
  → uarts_task()                            uarts.c:1002  (главный цикл, main.c:623)
  → uarts_task_data_rx()                    uarts.c:989   (pop, номер линии — последний байт)
  → uarts_rcv_data()                        uarts.c:973   (switch по номеру линии)
  → uart4_uart7_rcv()                       uarts.c:541   (для 4, 7; для 9 — из CM4)
```

Приём перезапускается сразу в колбэке (`uarts_ReceiveToIdle_IT_by_param`,
`uarts.c:147`); при ошибке `HAL_UART_ErrorCallback` (main.c:151) вызывает
`uarts_init_huart()` — abort + flush + повторный старт приёма.

Данные из прерывания не обрабатываются: они кладутся в динамический буфер
`uarts_rx_buf` (`dynamic_buf_t`, 60 КБ), разбор — в главном цикле. Это
разгружает прерывание, но добавляет задержку до одного оборота цикла.

### 6.2 Передача

```
uart4_uart7_snd(packet, len)          uarts.c:352   → UART4 + UART7 + I2C2
uart4_uart7_snd_others(src, …)        uarts.c:363   → все линии, кроме src
uart4_uart7_snd_piv(daddr, …)         uarts.c:370   → ++count_tx_id + create_piv_packet_cmd2 + snd()
uarts_transmit_piv(line, …)           uarts.c:282   → ++count_tx_id (для 4/7), только одна линия
uarts_transmit_piv2(…)                uarts.c:273   → create_piv_packet_cmd2
uarts_transmit_it_by_num(line, …)     uarts.c:225   → поиск huart по номеру; 9 → cm7core_i2c_send()
uarts_transmit_it_by_param(…)         uarts.c:171   → abort TX, memcpy в tx_buf, HAL_UART_Transmit_DMA
```

Особенности:

* перед каждой отправкой вызывается `HAL_UART_AbortTransmit()` — очередь на
  линию не строится, новый кадр прерывает недоотправленный;
* `count_tx_id` — общий на UART4 и UART7, `sound_tx_id` — на UART5/UART8;
* `uarts_transmit_piv()` обрабатывает только линии 4, 5, 7, 8; для линии 9
  (I2C2) ветки нет — см. п. 10.

### 6.3 Сервис

| Функция | Файл:строка | Назначение |
|---|---|---|
| `uarts_init()` | uarts.c:1035 | abort/flush всех UART, старт `ReceiveToIdle`, `uarts_ring_init()` |
| `uarts_init_huart()` | uarts.c:1023 | перезапуск приёма одной линии (после ошибки) |
| `uarts_abort_huart()` | uarts.c:118 | abort RX IT/RX/всё |
| `UART_flush()` | uarts.c:125 | сброс RX/TX FIFO и флагов ошибок |
| `uarts_ReceiveToIdle_IT_by_param()` | uarts.c:147 | `HAL_UARTEx_ReceiveToIdle_IT` с обработкой ошибки |
| `uarts_ring_init()` | uarts.c:1013 | инициализация `rcv_ring_tick[0..9]` текущим тиком |
| `uarts_send_ring()` | uarts.c:1077 | round-robin PING: 4 → 7 → 9, раз в секунду |
| `uart4_uart7_ring_check()` | uarts.c:695 | контроль кольца, `FAULT_PU_RING_CMD` |

Сопутствующие функции уровня приложения:

| Функция | Файл:строка | Назначение |
|---|---|---|
| `pd_rcv()` | pd.c:59 | диспетчер команд ПД, формирование ответов |
| `pd_transmit_piv()` | pd.c:43 | ответ в линию или в CM4 (`huart_number == 0`) |
| `uart4_uart7_recv_change_mode()` | uarts.c:383 | обработка `PU_CMD_CHANGE_MODE` |
| `uarts_transmit_change_mode()` | uarts.c:677 | широковещательная рассылка своего режима |
| `uarts_transmit_dejurniy()` | uarts.c:659 | `PD_CMD_DEJURNIY` конкретной панели |
| `uarts_transmit_veshanie_pd()` | uarts.c:664 | `PD_CMD_VESHANIE_PD` конкретной панели |
| `cm7core_pd_send()` | cm7core.c:549 | кадр в CM4 (`CORE_CMD_PD_SEND` = 51) |
| `cm7core_i2c_send()` | cm7core.c:556 | кадр в CM4 для I2C2 (`CORE_CMD_PD_I2C_SEND` = 54) |

Публичный API — `CM7/Core/Inc/uarts.h`. Функция `uart4_uart7_rcv()` объявлена
там же и вызывается в том числе из `cm7core.c` для линии I2C2.

---

## 7. Структуры данных

Размеры проверены компилятором ARMCC 5.06 (как в проекте).

| Структура | Размер | Где | Назначение |
|---|---|---|---|
| `huart_param_t` | — | uarts.c:79 | привязка `huart` + номер линии + RX/TX буферы |
| `huart_params[6]` | — | uarts.c:88 | таблица линий 1, 3, 4, 5, 7, 8 (UART4 — `ext1_count_*`, UART7 — `ext2_count_*`) |
| `pu_packet_cmd_t` | 5 (+данные) | common.h:126 | заголовок ПИВ: `daddr`, `saddr`, `data_len`, `cmd`, `cmd_data[]` |
| `pd_proto_header_t` | 4 | pd_data.h:235 | `daddr`, `saddr`, `data_len` |
| `pd_onlycommand_packet_t` | 7 | pd_data.h:250 | команда без данных + CRC (PING, DEJURNIY, INFO-запрос) |
| `pd_audio_param_cmd_data_t` | 12 | pd_data.h:286 | `uint32 frequency` (BE), `uint8 stereo`, `uint24 sound_lines`, `uint32 id` |
| `pd_sound_lines_cmd_data_t` | 7 | pd_data.h:309 | `uint24 sound_lines`, `uint32 id` |
| `pd_info_cmd_data_t` | 23 | pd_data.h:256 | описание INFO_RES (устарело, реально 27 байт) |
| `pu_small_info_t` | 6 | common.h:45 | `addr`, `mode`, `lines_count`, `lines:24` |
| `pd_spiskok_pu_item_t` | 3 | pd_data.h:334 | устарело; реальный элемент списка — 6 байт (`pu_small_info_t`) |
| `dynamic_buf_t uarts_rx_buf` | 60 КБ | uarts.c:39 | очередь принятых кадров (кадр + номер линии) |

`pu_small_info_t` — единственная «компактная» структура состояния панели:
адрес, режим (`pu_mode_t`), количество линий и 24-битное поле сработавших
пожарных линий. Она же — элемент ответа `PD_CMD_SPISOK_PU_RES`.

Фактический ответ `PD_CMD_INFO_RES` — **27 байт** (`pu_get_info()`, `pu.c:334`):

```
[0] режим ПУ            [1] адрес ПД, включившего режим   [2] ошибки (faults_small)
[3..5]  пожарные линии  [6..8] линии вещания              [9..11]  «отключение»
[12..14] «автоматика отключена»                            [15..19] год, месяц, день, час, мин
[20] текущее УО         [21] секунд до старта УО           [22] количество линий
[23..25] NETINFO        [26] исполнение
```

---

## 8. Команды

### 8.1 Команды кольца (широковещательные, 0xFF)

| Код | Имя | Данные | Обработчик |
|---|---|---|---|
| 0x39 | `PU_CMD_CHANGE_MODE` | `pu_small_info_t` (6 байт) | `uart4_uart7_recv_change_mode()` → смена режима/линий, файл пожарного оповещения |
| 0x40 | `PU_CMD_NET_INFO_UPDATE` | 3 байта `pu_netinfo` | `uart4_uart7_rcv()` |
| 0x37 | `PU_CMD_RING_CMD_CHECK` | — | определено, не используется |
| 0x38 | `PU_CMD_RING_AUDIO_CHECK` | — | определено, используется только на звуковых линиях (код закомментирован) |

`PU_CMD_CHANGE_MODE` шлётся при каждой смене режима (`pu_set_mode()` →
`uarts_transmit_change_mode()`), а также после смены настроек. Именно так соседи
узнают режим и количество линий друг друга.

### 8.2 Команды ПД (запросы/ответы), проходящие через кольцо

| Запрос | Имя | Данные запроса | Ответ | Обработчик |
|---|---|---|---|---|
| 0x3F | `PD_CMD_PING` | — | 0x6F `PD_CMD_PONG` | pd.c:76 |
| 0x6F | `PD_CMD_PONG` | — | — | не обрабатывается |
| 0x40 | `PD_CMD_INFO` | — (7 байт) | 0x70 + 27 байт | pd.c:79 |
| 0x41 | `PD_CMD_VESHANIE_PD` | 12 байт | 0x71, ошибка — 0xC1 + 1 байт | pd.c:88 |
| 0x42 | `PD_CMD_DEJURNIY` | — (7 байт) | 0x72 | pd.c:148 |
| 0x43 | `PD_CMD_ZVUKOVOE_VESHANIE` | 7 байт | 0x73, ошибка — 0xC3 | pd.c:159 |
| 0x44 | `PD_CMD_VESHANIE_POJAR` | 7 байт | 0x74, ошибка — 0xC4 | pd.c:186 |
| 0x45 | `PD_CMD_VESHANIE_MCHS` | 7 байт | 0x75, ошибка — 0xC5 | pd.c:209 |
| 0x46 | `PD_CMD_OTKLUCHENIE` | 7 байт | 0x76 | pd.c:236 |
| 0x47 | `PD_CMD_AVTOMATIKA_OTKLUCHENA` | 7 байт | 0x77 | pd.c:253 |
| 0x48 | `PD_CMD_RUCHNOI_PUSK` | 12 байт | 0x78, ошибка — 0xC8 | pd.c:271 |
| 0x49 | `PD_CMD_SPISOK_PU` | — | 0x79 + N × 6 байт | pd.c:322 |
| 0x75 | `PD_CMD_NET_INFO` | 3 байта | 0x76; если запрос пришёл от ПД (`huart_number == 0`), дополнительно широковещательный 0x40 | pd.c:328 |

Ошибка: код `0x80 | cmd`, данные — 1 байт `MKK_ERROR_PARAMS` (0xF1).

Раскладка данных:

* `0x41` / `0x48` (12 байт): `uint32 frequency` big-endian, `uint8 stereo`,
  `uint24 sound_lines` little-endian, `uint32 id` big-endian;
* `0x43`/`0x44`/`0x45`/`0x46`/`0x47` (7 байт): `uint24 sound_lines` little-endian,
  `uint32 id` big-endian;
* `0x49` ответ: последовательность `pu_small_info_t` по 6 байт
  (`addr`, `mode`, `lines_count`, `lines[3]`), только записи не старше 90 с;
* `0x70` ответ (на 0x40): 27 байт по раскладке из п. 7.

`0x55 PD_CMD_QUERY` — команда диалекта ПД (опрос дежурного), на кольце UART4/7
не обрабатывается: в `pd_rcv()` нет ветки 0x55. На линии I2C2 её шлёт CM4
(`PD_I2C_Poll()`), и ModulInterface отвечает 0x40.

`0x71…0x74` — коды звуковых пакетов Opus (`PD_CMD_OPUS48mono` и т. д.) на
линиях UART5/UART8; на UART4/7 они не ходят, хотя коды пересекаются с
ответами `*_RES` (различаются линией).

### 8.3 Что и когда отправляет сама панель по UART4/7

| Событие | Функция | Кадр |
|---|---|---|
| раз в секунду, по одной линии (4 → 7 → 9) | `uarts_send_ring()` | PING 0x3F на свой адрес, ID = 1, 7 байт |
| смена режима, смена настроек | `uarts_transmit_change_mode()` | 0xFF / 0x39 + `pu_small_info_t` |
| получен `PD_CMD_NET_INFO` от ПД | `pd.c:337` | 0xFF / 0x40 + 3 байта NETINFO |
| выход из режима «управление другими ПУ» | `uarts_transmit_dejurniy(addr)` | `addr` / 0x42 |
| экраны 5, 6, консольная команда | `uarts_transmit_veshanie_pd(addr, …)` | `addr` / 0x41 + 12 байт |
| консольная команда `SPISOK` | `cm7core.c:214` | 0xFF / 0x49 + `pu_small_info_t`, saddr = `local+1` |

---

## 9. Тайминги, контроль кольца, ошибки

* `uarts_send_ring()` вызывается из `timers_on_1sec()` (main.c:253) — раз в
  секунду одна линия, значит каждая линия получает PING раз в 3 с.
  PING адресован самому отправителю (daddr = saddr = local), поэтому соседи его
  только транслируют, а отправитель снимает по признаку `saddr == local`.
* `rcv_ring_tick[line]` обновляется любым кадром с `saddr == local`
  (вернувшимся по кольцу).
* `uart4_uart7_ring_check()` вызывается из `timers_on_1min()` — фактически раз
  в 59 с (main.c:281). Если после последнего широковещательного кадра
  (`uart4_uart7_snd_ff_tick`, ставится только для `packet[0] == 0xFF`) **ни одна**
  из линий 4/7 не видела своего кадра — ставится `FAULT_PU_RING_CMD` (117),
  иначе снимается.
* `uarts_ring_init()` вызывается при входе в дежурный режим и в `uarts_init()`.
* `FAULT_PU_RING_CMD` / `FAULT_PU_RING_AUDIO` (118) сворачиваются в
  `faults_small` и уходят в INFO_RES.

Контроль построен на «своих» кадрах, а не на таймауте ожидания: нет
подтверждений/ретраев на уровне кадра, надёжность — CRC + фильтр дублей.

---

## 10. Нюансы и известные расхождения

1. **Двойная семантика байтов 2–3.** В кольце это ID, у ПД — длина. Любой код,
   который читает `packet[2..3]` как длину на кольце, ошибается; на границе с ПД
   значение принудительно перезаписывается (`uarts.c:581`, `cm7core.c:300`).
2. **PING всегда с ID = 1** (`create_piv_packet_cmd` кладёт `1 + cmd_data_len`).
   Для кольца это не важно (PING снимается до фильтра дублей), но PING,
   адресованный панели извне, после первых кадров будет отброшен фильтром
   дублей как «старый» ID.
3. **Фильтр дублей общий для всех отправителей** (`count_rx_id`), хотя
   `count_tx_id` у каждой панели свой. Кадры разных панелей с разными ID
   сравниваются между собой; при разнице меньше 100 валидный кадр может быть
   отброшен. В коде есть закомментированная таблица «номер линии по адресу» —
   вероятно, заготовка под фильтр по отправителю.
4. **Ответы для линии I2C2 не отправляются:** `uarts_transmit_piv()` умеет
   только 4/5/7/8. Если `pd_rcv()` получил кадр с линии 9 и формирует ответ,
   вызов `uarts_transmit_piv(9, …)` ничего не делает — ответ теряется.
   По I2C2 ответ ПД забирает мастер CM4 в том же обмене
   (`PD_I2C_SendReceive()`), а не CM7.
5. **`pd_audio_data.id` не заполняется** в `uarts_transmit_veshanie_pd()`
   (uarts.c:664): поле `id` остаётся неинициализированным, в кадр уходят
   «мусорные» 4 байта. Если ПД начнёт использовать ID — это надо исправить.
6. **Мёртвый код:** `uart4_uart7_rcv_spisok_pu()` (uarts.c:476) не вызывается
   (вызов закомментирован в `uart4_uart7_rcv`), поэтому ответы SPISOK не
   изучаются; `PU_CMD_RING_CMD_CHECK` (0x37) не используется.
7. **Нет проверки длины в `uart4_uart7_rcv()`**: функция сразу читает CRC по
   `packet_len - 2`. Для UART4/7 это безопасно (прерывание отбрасывает кадры
   короче 7 байт), но прямой вызов из `CORE_CMD_PD_I2C_RECV` такой гарантии
   не даёт.
8. **Транзитные кадры не дедуплицируются** — так и задумано (иначе кадр не
   пройдёт кольцо), но широковещательный кадр, пришедший с двух сторон,
   обрабатывается один раз, а второй экземпляр отбрасывается и дальше не
   передаётся.
9. **Передача без очереди:** `HAL_UART_AbortTransmit()` перед каждым кадром
   означает, что при плотном потоке предыдущий кадр может быть оборван.
10. **Максимум 127 байт на приём:** номер линии пишется в `rx_buf[Size]`;
    кадр длиной 128 байт запишет байт за границу массива.
11. `pd_info_cmd_data_t` и `pd_spiskok_pu_item_t` в `pd_data.h` не соответствуют
    фактическим данным (27 байт INFO_RES; 6 байт на элемент списка) — при
    доработках ориентироваться на `pu_get_info()` и `pu_small_info_t`.

---

## 11. Как добавить новую команду

1. Код — в `Common/pd_data.h` (рядом с остальными `PD_CMD_*`).
   Ответ = `cmd + 0x30` (запрос 0x4x → ответ 0x7x), ошибка = `0x80 | ответ`.
2. Ветка в `pd_rcv()` (`pd.c`): проверить `packet_len` (для кольца — по полной
   длине кадра, не по полю длины!), разобрать данные по смещениям, ответить
   `pd_transmit_piv(packet[1], packet[0], PD_CMD_..._RES, …)` — тогда ответ
   уйдёт в ту же линию кольца; для кадра от ПД (`huart_number == 0`) — в CM4.
3. Если команда широковещательная (0xFF) — ветка в `switch (packet[4])` внутри
   `uart4_uart7_rcv()`; помнить, что после обработки кадр всё равно уйдёт в
   `uart4_uart7_snd_others()`.
4. Если нужна рассылка — `uart4_uart7_snd_piv(daddr, saddr, cmd, data, len)`:
   она сама увеличит `count_tx_id`, соберёт кадр и отправит во все линии.
5. Обновить этот документ и, при необходимости, `I2C2_RING.md`.

---

## 12. Источники

| Тема | Файл |
|---|---|
| Линии, шлюз, команды кольца | `CM7/Core/Src/uarts.c`, `CM7/Core/Inc/uarts.h` |
| Диспетчер команд ПД | `CM7/Core/Src/pd.c`, `CM7/Core/Inc/pd.h` |
| Коды команд и структуры | `Common/pd_data.h` |
| Сборка/CRC/разбор ПИВ | `Common/common.c`, `Common/common.h` |
| CRC16 | `util/math_util.c` |
| Инициализация UART и таймеры | `CM7/Core/Src/main.c`, `CM7/Core/Src/stm32h7xx_hal_msp.c` |
| CM4-шлюз, PD-кадры | `CM7/Core/Src/cm7core.c`, `Common/shared_memory.h`, `CM4/Core/Src/cm4core.c`, `CM4/Core/Src/uart6.c` |
| Третья линия I2C2 | `I2C2_RING.md`, `CM4/Core/Src/pd_i2c.c` |
| Протокол ПД (сторона ModulInterface) | `../ModulInterface_v3.8 p2p/Core/PD_protocol/ProtocolShared.h` |
