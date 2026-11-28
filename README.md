# Keil Assistant + Keil MDK: как связать расширение с проектом RS-PU v4

Практическая записка: как открыть проект `pu_new.uvprojx` в VS Code через расширение
**Keil Assistant**, собирать и прошивать оба ядра STM32H747 (CM4/CM7), не меняя
структуру проекта. Описан сценарий **«ничего не менять»** — рабочей папкой VS Code
остаётся `MDK-ARM`, проект подхватывается автоматически при старте.

---

## 1. Окружение (проверено на этой машине)

| Компонент | Значение |
|---|---|
| Keil MDK | `C:\Keil_v5\UV4\UV4.exe` |
| Компилятор | ARMCLANG **V6.24**, `C:\Keil_v5\ARM\ARMCLANG\Bin` |
| VS Code | `C:\Program Files\Microsoft VS Code` |
| Расширение Keil Assistant | `cl.keil-assistant` **1.7.0** |
| Расширение C/C++ | `ms-vscode.cpptools` **1.35.6** |
| Языковой пакет | русский (`ms-ceintl.vscode-language-pack-ru`) |
| Прошивальщик | ST-Link (`STLink\ST-LINKIII-KEIL_SWO.dll`) |

## 2. Где что лежит

```
D:\DA\2_RS_PU\RS-PU_v4\                  <- рабочая папка VS Code (этот README)
└── RS-PU_v4\                            <- корень исходников
    ├── CM4\Core\{Inc,Src}               <- код ядра Cortex-M4
    ├── CM7\Core\{Inc,Src}               <- код ядра Cortex-M7
    ├── Common\, Drivers\, util\, ...    <- общий код и HAL/CMSIS
    ├── .cubemx_snapshot\MDK-ARM\        <- СНИМОК CubeMX (копия .uvprojx!) — не открывать
    └── MDK-ARM\                         <- РАБОЧАЯ ПАПКА ДЛЯ VS CODE
        ├── pu_new.uvprojx               <- проект Keil, 2 таргета в одном файле
        ├── pu_new.uvoptx                <- отладочные/прошивочные настройки (ST-Link)
        └── .vscode\                     <- создаётся расширением
            ├── c_cpp_properties.json    <- генерируется автоматически
            ├── keil-assistant.log       <- лог расширения
            └── uv4.log                  <- лог последней сборки/прошивки
```

Таргеты проекта:

| Target | Ядро | Device | Выходная папка | Старт в памяти |
|---|---|---|---|---|
| `pu_new_CM4` | Cortex-M4 | `STM32H747IITx:CM4` | `MDK-ARM\pu_new_CM4\` | `0x08100000` |
| `pu_new_CM7` | Cortex-M7 | `STM32H747IITx:CM7` | `MDK-ARM\pu_new_CM7\` | `0x08000000` |

У обоих `CreateHexFile = 1` и After-Build шаг `fromelf`, который дополнительно делает `.bin` из `.axf`; прошивальщик ST-Link.

## 3. Настройки VS Code, с которыми всё работает

Файл `%APPDATA%\Code\User\settings.json` (пользовательские настройки, не проектные):

```jsonc
{
    "gitlens.graph.details.location": "auto",
    "editor.inlineSuggest.enabled": true,
    "continue.showInlineTip": false,
    "KeilAssistant.Project.ExcludeList": [
        "template.uvproj",
        "template.uvprojx"
    ],
    "KeilAssistant.MDK.Uv4Path": "C:\\Keil_v5\\UV4\\UV4.exe",
    "C_Cpp.default.intelliSenseMode": "clang-arm",
    "C_Cpp.default.compilerPath": ""
}
```

Что здесь принципиально:

- **`KeilAssistant.MDK.Uv4Path`** — единственная обязательная настройка расширения.
  У неё область `machine`, поэтому она задаётся **только в пользовательских настройках**
  и не работает из `.vscode\settings.json` проекта.
- **`C_Cpp.default.intelliSenseMode: clang-arm`** — расширение генерирует конфигурации
  C/C++ с `intelliSenseMode: "${default}"`, и именно это значение подставляется.
  Без него разбор ARMCLANG-кода будет некорректным.
- **`C_Cpp.default.compilerPath: ""`** — пустая строка, ни на что не влияет, но лучше
  просто удалить ключ, чтобы не путаться.
- `KeilAssistant.C51.Uv4Path` не нужен: проект ARM, а не C51.

> `.vscode\settings.json` в проекте не создаётся и не нужен. Всё, что перечислено,
> живёт в пользовательских настройках.

## 4. Почему рабочая папка — именно `MDK-ARM`

Расширение при активации сканирует **только корень рабочей папки, без рекурсии**
(`readdirSync` одного уровня + фильтр `\.uvproj[x]?$`), а `.vscode\c_cpp_properties.json`
и `uv4.log` создаёт **рядом с `.uvprojx`**.

Отсюда:

| Что открыто | Проект загрузится сам? | IntelliSense получит include-пути? |
|---|---|---|
| `D:\DA\2_RS_PU\RS-PU_v4` (эта папка) | **нет**, `.uvprojx` вложен на 2 уровня | нет |
| `...\RS-PU_v4` (корень исходников) | **нет**, `.uvprojx` в `MDK-ARM` | нет (файл уйдёт в `MDK-ARM\.vscode`) |
| `...\RS-PU_v4\MDK-ARM` | **да** | **да** |

Поэтому: всегда открывать `...\RS-PU_v4\MDK-ARM`. Это и есть сценарий «ничего не менять».

## 5. Первичная привязка (один раз)

1. Закрыть Keil uVision (он одноэкземплярный, может держать `.uvprojx`).
2. `Файл → Открыть папку…` → `D:\DA\2_RS_PU\RS-PU_v4\RS-PU_v4\MDK-ARM`
   (из терминала: `code "D:\DA\2_RS_PU\RS-PU_v4\RS-PU_v4\MDK-ARM"`).
3. Проверить `Ctrl+,` → поиск `keil`: `KeilAssistant.MDK.Uv4Path` = `C:\Keil_v5\UV4\UV4.exe`.
4. В панели Explorer появится раздел **Keil uVision Project** с узлом `pu_new`.
   Если раздела нет — `Ctrl+Shift+P` → **Open keil uVision project** → выбрать
   `MDK-ARM\pu_new.uvprojx` → на вопрос `switch workspace ?` ответить **Later**.
5. Убедиться, что создался `MDK-ARM\.vscode\c_cpp_properties.json` с конфигурациями
   `pu_new_CM4` и `pu_new_CM7`.

Всё. Дальше при каждом старте VS Code проект подхватывается сам.

## 6. Ежедневный цикл: собрать → прошить

1. **Выбрать таргет.** В разделе Keil uVision Project у узла `pu_new` нажать inline-кнопку
   **Switch Target** (или правый клик → Switch Target) → в списке выбрать `pu_new_CM4`
   или `pu_new_CM7`.
2. **Собрать.** На строке таргета — inline-кнопки:
   - ▶ **Build** — инкрементальная сборка;
   - ↻ **Rebuild** — полная пересборка.
3. **Прошить.** ↓ **Download** на той же строке таргета (ST-Link должен быть подключён).
4. **Проверить результат** в `MDK-ARM\.vscode\uv4.log` (перезаписывается каждым запуском).


## 7. Генерация `.bin` (fromelf)

В обоих таргетах (`pu_new_CM4`, `pu_new_CM7`) включён шаг **Options for Target →
User → After Build/Rebuild → Run User Program #1**:

```
$K\ARM\ARMCLANG\Bin\fromelf.exe --bin --output "pu_new_CM4\@L.bin" "!L"
```

(для CM7 — соответственно `pu_new_CM7\@L.bin`). Ключевые последовательности uVision:

| Код | Что подставляется |
|---|---|
| `$K` | каталог установки Keil (`C:\Keil_v5`) |
| `@L` | `OutputName` таргета (`pu_new_CM4` / `pu_new_CM7`) |
| `!L` | путь к собранному `.axf` (`pu_new_CM4\pu_new_CM4.axf`) |

Рабочий каталог команды — `MDK-ARM`, поэтому результат появляется рядом с `.axf`:

| Таргет | Файл | Адрес загрузки |
|---|---|---|
| `pu_new_CM7` | `MDK-ARM\pu_new_CM7\pu_new_CM7.bin` | `0x08000000` |
| `pu_new_CM4` | `MDK-ARM\pu_new_CM4\pu_new_CM4.bin` | `0x08100000` |

`--bin` выгружает сплошной образ от начала IROM таргета до последнего байта
(промежутки заполняются нулями); адрес в файле не хранится, поэтому при прошивке
его нужно указывать явно: CM7 — `0x08000000`, CM4 — `0x08100000`.

Шаг выполняется при каждом **Build/Rebuild** — в том числе при сборке из VS Code
через Keil Assistant и из командной строки `UV4 -b`, даже если перекомпилировать
нечего: `.bin` всегда соответствует текущему `.axf`.

`*.bin` попадает под правило `.gitignore` — это артефакт сборки, в репозиторий он
не коммитится (в отличие от `.hex`, для которых сделано исключение).

## 8. Switch target - кнопка рядом с pu_new_CM4



## 10. `VERSION` задавать в keil в target options в define


## 12. Прошивка двух ядер

1. Прошить **сначала CM7** (`0x08000000`), затем **CM4** (`0x08100000`) — так работает
   dual-core boot на H747.
2. Сборка и прошивка — **отдельные действия**: Build не прошивает, Download не собирает.
3. Одна сборка = один таргет. «Собрать оба одной кнопкой» расширение не умеет.
4. Успех в `uv4.log`:

```
Load "pu_new_CM7\\pu_new_CM7.axf"
Erase Done.Programming Done.Verify OK.Flash Load finished at HH:MM:SS
```

## 13. `BUILD_FLASH.cmd` - универсальная сборка и прошивка

Батник кладётся в корень любого проекта Keil (папка, внутри которой есть `*.uvprojx`)
и переносится между проектами без правок: он сам находит проект, таргеты и МК,
подбирает ST-LINK с этой платой, собирает и прошивает.

```bat
BUILD_FLASH.cmd                      :: сборка + прошивка "рабочих" таргетов
BUILD_FLASH.cmd /rebuild             :: полная пересборка + прошивка
BUILD_FLASH.cmd /buildonly           :: только собрать
BUILD_FLASH.cmd /flashonly           :: только прошить
BUILD_FLASH.cmd /list                :: показать проект, таргеты, ST-LINK и выйти
BUILD_FLASH.cmd /target:pu_new_CM7   :: только один таргет
BUILD_FLASH.cmd /sn:48FF6F064988885256281767
```

Что определяется автоматически:

| Что | Как |
|---|---|
| проект | `*.uvprojx` в папке батника и ниже; снимки `.cubemx_snapshot`, `backup`, `template` пропускаются. Если проектов несколько - `/project:ИМЯ` |
| таргеты | из `.uvprojx`: по умолчанию все с `Selected for Batch Build`, иначе текущий таргет из `.uvoptx`, иначе первый |
| МК | поле `<Device>` таргета: `STM32H747IITx:CM4` -> МК `STM32H747IITx`, семейство `STM32H7` |
| ST-LINK | `ST-LINK_CLI -List`, затем подключение к каждой пробе: семейство в ответе должно содержать семейство МК (`STM32H7` -> `STM32H7xx`). Пробы без цели и с другим семейством пропускаются |

Поэтому здесь он собирает и шьёт `pu_new_CM4` и `pu_new_CM7` (порядок - как в
`.uvprojx`: CM4, затем CM7), а в `ModulInterface_v6 p2p` возьмёт единственный таргет
на STM32F103RC и выберет уже другой ST-LINK - хотя в `.uvoptx` того проекта записан
чужой SN.

Перед прошивкой найденный SN прописывается в `.uvoptx` проекта (uVision иначе шьёт
адаптер, записанный в проекте), после прошивки файл восстанавливается из копии.
Сборка и прошивка - штатные `UV4 -b`/`-r` и `UV4 -f`, то же, что кнопки Build/Rebuild/
Download в Keil.

Ключи: `/rebuild`, `/buildonly`, `/flashonly`, `/list`, `/project:ИМЯ`, `/target:ИМЯ`,
`/all`, `/sn:XXXX` (или `/sn XXXX`), `/pause`, `/nopause`, `/?`. Значения с `=` тоже
понимаются, но cmd режет аргумент по `=`, поэтому надёжнее двоеточие.

Коды возврата: `0` - успех, `1` - окружение/аргументы/проект, `2` - ST-LINK,
`3` - сборка, `4` - прошивка. Логи сборки/прошивки пишутся в `%TEMP%\keil_build_flash\`
(чтобы не мусорить в репозитории), путь печатается в конце.
## 14. Чек-лист «всё связано»

- [ ] Рабочая папка VS Code — `...\RS-PU_v4\MDK-ARM`
- [ ] В Explorer есть раздел **Keil uVision Project** с узлом `pu_new`
- [ ] `KeilAssistant.MDK.Uv4Path` указывает на `C:\Keil_v5\UV4\UV4.exe`
- [ ] `MDK-ARM\.vscode\c_cpp_properties.json` содержит `pu_new_CM4` и `pu_new_CM7`
- [ ] Switch Target переключает таргет, inline-кнопки Build/Rebuild/Download работают
- [ ] `Rebuild` даёт `0 Error(s), 0 Warning(s)` и свежий `.hex` в папке таргета
- [ ] Build/Rebuild обновляет `.bin` рядом с `.axf` (шаг `fromelf` в After Build)
- [ ] `BUILD_FLASH.cmd` из корня проекта сам находит `.uvprojx`, МК и ST-LINK, собирает и прошивает таргеты
- [ ] `Download` пишет `Flash Load finished` и прошивает **выбранное** ядро
- [ ] Keil uVision закрыт во время сборки из VS Code
- [ ] Горячие клавиши `F7`/`Ctrl+Alt+F7`/`Ctrl+Alt+D` не используются


