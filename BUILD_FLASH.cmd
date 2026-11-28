@echo off
rem ==========================================================================
rem  BUILD_FLASH.cmd - универсальная сборка и прошивка проекта Keil uVision
rem
rem  Файл кладётся в корень проекта (папка, внутри которой есть *.uvprojx)
rem  и переносится в любой другой проект без правок.
rem
rem  Что делает:
rem    1) находит .uvprojx в своей папке и ниже, читает таргеты и МК;
rem    2) находит ST-LINK, к которому подключён именно этот МК
rem       (пробы с другим семейством или без цели пропускает);
rem    3) собирает таргеты через Keil uVision (-b / -r);
rem    4) прошивает их этим ST-LINK (uVision -f, с verify);
rem    5) сбрасывает и запускает МК.
rem
rem  Какие таргеты берутся по умолчанию:
rem    - если таргет один - он;
rem    - если несколько - все, помеченные в Keil "Selected for Batch Build";
rem    - иначе текущий таргет проекта (из .uvoptx), иначе первый.
rem
rem  Ключи:
rem    /rebuild         полная пересборка (UV4 -r) вместо инкрементальной (-b)
rem    /buildonly       только собрать
rem    /flashonly       только прошить (без сборки)
rem    /list            показать проект, таргеты и ST-LINK, ничего не делая
rem    /project:ИМЯ     выбрать проект, если в дереве их несколько
rem    /target:ИМЯ      собрать/прошить только этот таргет
rem    /all             все таргеты проекта
rem    /sn:XXXX         конкретный ST-LINK (Serial Number)
rem    /pause           пауза в конце (при запуске из проводника - сама)
rem    /nopause         без паузы
rem    справка          - ключ /? (этот текст)
rem
rem  Коды возврата: 0 - успех, 1 - окружение/аргументы/проект,
rem                 2 - ST-LINK, 3 - сборка, 4 - прошивка
rem ==========================================================================

setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

rem запуск двойным щелчком (cmd /c ...) - в конце нужна пауза
set "CMDL=!cmdcmdline!"
echo "!CMDL!" | findstr /i /c:"/c" >nul
if not errorlevel 1 set "NEED_PAUSE=1"

rem ------------------------------ умолчания --------------------------------
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "TMPD=%TEMP%\keil_build_flash"
set "DO_BUILD=1"
set "DO_FLASH=1"
set "DO_LIST=0"
set "MODE=-b"
set "MODENAME=build"
set "ALL_TARGETS=0"
set "WANT_SN="
set "WANT_PROJECT="
set "WANT_TARGET="
set "PAUSE_AT_END=0"
set "UVOPTX_PATCHED=0"
set "FOUND_SN="
set "FOUND_N=0"
set "FOUND_ALL="
set "TARGETS_N=0"
set "TGT_ALL="
set "FAMILY="
set "DEVNAME="
set "SEQ="
set "SEQLIST="
set "SEQSHOW="
set "SEQCNT=0"
set "CANDS=0"
set "PROJ="
set "UVOPTX="
set "RC=0"

rem ------------------------------- аргументы --------------------------------
:args
if "%~1"=="" goto args_done
set "A=%~1"
if /i "%A%"=="/?" goto usage_ok
if /i "%A%"=="/help" goto usage_ok
if /i "%A%"=="-h" goto usage_ok
if /i "%A%"=="/list" goto arg_list
if /i "%A%"=="/rebuild" goto arg_rebuild
if /i "%A%"=="/buildonly" goto arg_buildonly
if /i "%A%"=="/flashonly" goto arg_flashonly
if /i "%A%"=="/all" goto arg_all
if /i "%A%"=="/pause" goto arg_pause
if /i "%A%"=="/nopause" goto arg_nopause
if /i "%A%"=="/sn" goto arg_sn_word
if /i "%A%"=="/project" goto arg_project_word
if /i "%A%"=="/target" goto arg_target_word
set "PRE4=!A:~0,4!"
set "PRE8=!A:~0,8!"
set "PRE9=!A:~0,9!"
if /i "!PRE4!"=="/sn:" goto arg_sn
if /i "!PRE9!"=="/project:" goto arg_project
if /i "!PRE8!"=="/target:" goto arg_target
echo [ОШИБКА] неизвестный ключ: %A%
goto usage_err

:arg_list
set "DO_LIST=1"
shift
goto args

:arg_rebuild
set "MODE=-r"
set "MODENAME=rebuild"
shift
goto args

:arg_buildonly
set "DO_FLASH=0"
shift
goto args

:arg_flashonly
set "DO_BUILD=0"
shift
goto args

:arg_all
set "ALL_TARGETS=1"
shift
goto args

:arg_pause
set "PAUSE_AT_END=1"
set "NEED_PAUSE=1"
shift
goto args

:arg_nopause
set "PAUSE_AT_END=0"
set "NEED_PAUSE=0"
shift
goto args

:arg_sn
set "WANT_SN=!A:~4!"
shift
if "!WANT_SN!"=="" goto usage_err
goto args

:arg_sn_word
shift
set "WANT_SN=%~1"
shift
if "!WANT_SN!"=="" goto usage_err
goto args

:arg_project
set "WANT_PROJECT=!A:~9!"
shift
if "!WANT_PROJECT!"=="" goto usage_err
goto args

:arg_project_word
shift
set "WANT_PROJECT=%~1"
shift
if "!WANT_PROJECT!"=="" goto usage_err
goto args

:arg_target
set "WANT_TARGET=!A:~8!"
shift
if "!WANT_TARGET!"=="" goto usage_err
goto args

:arg_target_word
shift
set "WANT_TARGET=%~1"
shift
if "!WANT_TARGET!"=="" goto usage_err
goto args

:args_done
if "%DO_BUILD%"=="0" if "%DO_FLASH%"=="0" if not "%DO_LIST%"=="1" goto usage_err

rem --------------------------- поиск инструментов ---------------------------
if defined UV4 if exist "%UV4%" goto uv4_found
set "UV4="
for %%C in ("%ProgramFiles%\Keil_v5\UV4\UV4.exe" "%ProgramW6432%\Keil_v5\UV4\UV4.exe" "C:\Keil_v5\UV4\UV4.exe") do if not defined UV4 if exist "%%~C" set "UV4=%%~C"
:uv4_found
if not defined UV4 (
    echo [ОШИБКА] не найден UV4.exe ^(Keil uVision^).
    echo          Установите MDK или задайте переменную окружения UV4.
    set "RC=1"
    goto finish
)

if defined STLINK if exist "%STLINK%" goto stlink_found
set "STLINK="
for %%C in ("C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" "C:\Program Files\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe") do if not defined STLINK if exist "%%~C" set "STLINK=%%~C"
:stlink_found
if not defined STLINK (
    echo [ОШИБКА] не найден ST-LINK_CLI.exe ^(утилита STM32 ST-LINK Utility^).
    echo          Установите утилиту или задайте переменную окружения STLINK.
    set "RC=1"
    goto finish
)

if exist "%TMPD%" rd /s /q "%TMPD%" >nul 2>&1
mkdir "%TMPD%" >nul 2>&1

tasklist /fi "imagename eq UV4.exe" 2>nul | findstr /i "UV4.exe" >nul
if not errorlevel 1 echo [i] внимание: Keil uVision уже запущен; если в нём открыт этот проект - лучше закрыть

rem ---------------------------- поиск проекта ------------------------------
echo.
echo === поиск проекта Keil ===
for /r "%ROOT%" %%F in (*.uvprojx) do call :consider_project "%%~fF"

if "%CANDS%"=="0" (
    echo [ОШИБКА] в "%ROOT%" и ниже не найдено ни одного *.uvprojx
    set "RC=1"
    goto finish
)
if "%CANDS%"=="1" set "PROJ=!CAND1!"
if not defined PROJ (
    echo [ОШИБКА] найдено несколько проектов, уточните ключом /project:ИМЯ
    call :list_candidates
    set "RC=1"
    goto finish
)
for %%F in ("%PROJ%") do if exist "%%~dpnF.uvoptx" set "UVOPTX=%%~dpnF.uvoptx"
echo   [OK] проект: %PROJ%

rem --------------------------- таргеты проекта -----------------------------
powershell -NoProfile -ExecutionPolicy Bypass -Command "try{[xml]$x=[IO.File]::ReadAllText('%PROJ%'); $o=@(); foreach($t in $x.Project.Targets.Target){$c=$t.TargetOption.TargetCommonOption; $o+=($t.TargetName+'|'+$c.Device+'|'+$c.SelectedForBatchBuild+'|'+$c.OutputDirectory+'|'+$c.OutputName)}; [IO.File]::WriteAllLines('%TMPD%\targets.txt',$o); exit 0}catch{exit 1}"
if errorlevel 1 (
    echo [ОШИБКА] не удалось прочитать таргеты из проекта
    set "RC=1"
    goto finish
)
if not exist "%TMPD%\targets.txt" (
    echo [ОШИБКА] проект не содержит таргетов
    set "RC=1"
    goto finish
)

set "NT=0"
for /f "usebackq delims=" %%L in ("%TMPD%\targets.txt") do (
    set /a NT+=1
    set "TGT!NT!=%%L"
)
if "%NT%"=="0" (
    echo [ОШИБКА] проект не содержит таргетов
    set "RC=1"
    goto finish
)

rem МК определяем по первому таргету
for /f "tokens=1-5 delims=|" %%A in ("!TGT1!") do set "DEVSTR=%%B"
for /f "tokens=1 delims=:" %%D in ("!DEVSTR!") do set "DEVNAME=%%D"
if "!DEVNAME!"=="" set "DEVNAME=!DEVSTR!"
rem семейство: STM32H747IITx -^> STM32H7, STM32F103RC -^> STM32F1
set "FAMILY=!DEVNAME:~0,7!"

echo.
echo === проект ===
echo   файл    : %PROJ%
echo   МК      : !DEVNAME!   ^(семейство !FAMILY!^)
echo   таргетов: %NT%

set "I=0"
:tbl_loop
set /a I+=1
if !I! GTR %NT% goto tbl_done
for /f "tokens=1-2 delims=|" %%A in ("!TGT%I%!") do echo   - %%A   ^(%%B^)
goto tbl_loop
:tbl_done

rem --------------------------- выбор таргетов ------------------------------
if defined WANT_TARGET goto sel_target
if "%ALL_TARGETS%"=="1" goto sel_all

rem 1) таргеты, помеченные "Selected for Batch Build"
set "I=0"
:b_loop
set /a I+=1
if !I! GTR %NT% goto b_done
for /f "tokens=1-5 delims=|" %%A in ("!TGT%I%!") do if "%%C"=="1" call :add_seq "%%A"
goto b_loop
:b_done
if defined SEQ goto sel_done

rem 2) текущий таргет из .uvoptx
call :read_current_target
if defined CURT call :add_seq "!CURT!"
if defined SEQ goto sel_done

rem 3) первый таргет
for /f "tokens=1 delims=|" %%A in ("!TGT1!") do call :add_seq "%%A"
goto sel_done

:sel_all
set "I=0"
:a_loop
set /a I+=1
if !I! GTR %NT% goto sel_done
for /f "tokens=1 delims=|" %%A in ("!TGT%I%!") do call :add_seq "%%A"
goto a_loop

:sel_target
set "I=0"
:t_loop
set /a I+=1
if !I! GTR %NT% goto t_done
for /f "tokens=1 delims=|" %%A in ("!TGT%I%!") do call :match_target "%%A"
goto t_loop
:t_done
if not defined SEQ (
    echo [ОШИБКА] таргет "%WANT_TARGET%" не найден в проекте
    set "RC=1"
    goto finish
)

:sel_done
if not defined SEQ (
    echo [ОШИБКА] не удалось выбрать таргеты
    set "RC=1"
    goto finish
)
echo   выбрано : %SEQSHOW%

rem ------------------------- режим только список ---------------------------
if not "%DO_LIST%"=="1" goto list_skip
echo.
echo === ST-LINK на этом ПК ===
call :scan_probes
set "RC=0"
goto finish
:list_skip

rem ---------------------- поиск ST-LINK под этот МК ------------------------
if not "%DO_FLASH%"=="1" goto work_build
echo.
echo === поиск ST-LINK, к которому подключён !DEVNAME! ===
call :scan_probes
if errorlevel 1 (
    set "RC=2"
    goto finish
)
call :ensure_uvoptx_sn

rem ------------------------------ сборка -----------------------------------
:work_build
if not "%DO_BUILD%"=="1" goto work_flash
echo.
echo === сборка ^(%MODENAME%^) ===
for %%T in (%SEQ%) do (
    call :build_one %%T
    if errorlevel 1 (
        set "RC=3"
        goto finish
    )
)

rem ------------------------------ прошивка ---------------------------------
:work_flash
if not "%DO_FLASH%"=="1" goto work_done
echo.
echo === прошивка через ST-LINK !FOUND_SN! ===
for %%T in (%SEQ%) do (
    call :flash_one %%T
    if errorlevel 1 (
        set "RC=4"
        goto finish
    )
)

echo.
echo === сброс и запуск МК ===
call :reset_run

:work_done
set "RC=0"
goto finish

rem ============================== подпрограммы ==============================

rem --- найденный .uvprojx: фильтр и учёт (аргумент: путь) -------------------
:consider_project
set "PF=%~1"
echo "!PF!" | findstr /i /C:".cubemx_snapshot" /C:"\backup" /C:"template" /C:".git" >nul
if not errorlevel 1 exit /b 0
if defined WANT_PROJECT (
    echo "!PF!" | findstr /i /C:"!WANT_PROJECT!" >nul
    if errorlevel 1 exit /b 0
)
set /a CANDS+=1
set "CAND!CANDS!=!PF!"
exit /b 0

rem --- перечислить найденные проекты ---------------------------------------
:list_candidates
set "I=0"
:lc_loop
set /a I+=1
if !I! GTR %CANDS% goto lc_done
call :show_cand !I!
goto lc_loop
:lc_done
exit /b 0

:show_cand
echo   - "!CAND%~1!"
exit /b 0

rem --- добавить таргет в последовательность (без дублей) --------------------
:add_seq
set "TN=%~1"
if "%TN%"=="" exit /b 0
echo "!SEQLIST!" | findstr /i /C:"[%TN%]" >nul
if not errorlevel 1 exit /b 0
set "SEQLIST=!SEQLIST! [%TN%]"
set "SEQ=!SEQ! "%TN%""
set "SEQSHOW=!SEQSHOW! %TN%"
set /a SEQCNT+=1
exit /b 0

rem --- совпадение с /target: -----------------------------------------------
:match_target
if /i "%~1"=="%WANT_TARGET%" call :add_seq "%~1"
exit /b 0

rem --- текущий таргет из .uvoptx -------------------------------------------
:read_current_target
set "CURT="
if not defined UVOPTX exit /b 0
set "CTF=%TMPD%\current_target.txt"
if exist "%CTF%" del /f /q "%CTF%" >nul 2>&1
powershell -NoProfile -ExecutionPolicy Bypass -Command "try{[xml]$u=[IO.File]::ReadAllText('%UVOPTX%'); $n=$u.SelectSingleNode('//Target[TargetOption/OPTFL/IsCurrentTarget=''1'']/TargetName'); if($n){[IO.File]::WriteAllText('%CTF%',$n.InnerText)}}catch{}"
if exist "%CTF%" for /f "usebackq delims=" %%A in ("%CTF%") do set "CURT=%%A"
exit /b 0

rem --- опрос всех ST-LINK и выбор нужного ----------------------------------
:scan_probes
set "FOUND_SN="
set "FOUND_N=0"
set "FOUND_ALL="
set "TARGETS_N=0"
set "TGT_ALL="
if defined WANT_SN goto scan_forced
set "LISTF=%TMPD%\stlink_list.txt"
"%STLINK%" -List >"%LISTF%" 2>&1
if not exist "%LISTF%" (
    echo [ОШИБКА] ST-LINK_CLI.exe не отработал
    exit /b 1
)
for /f "usebackq tokens=2 delims=:" %%A in (`findstr /C:"SN:" "%LISTF%"`) do call :probe_one "%%A"
if not "%FOUND_N%"=="0" goto scan_pick
if "%TARGETS_N%"=="1" (
    echo   [i] семейство не совпало, но цель одна - берём её
    set "FOUND_SN=%TGT_ALL%"
    set "FOUND_N=1"
)
:scan_pick
if "%FOUND_N%"=="0" (
    echo.
    echo [ОШИБКА] ни один ST-LINK не видит МК !DEVNAME!
    echo          проверьте питание платы и кабель SWD
    exit /b 1
)
if not "%FOUND_N%"=="1" (
    echo.
    echo [ОШИБКА] найдено несколько подходящих ST-LINK:%FOUND_ALL%
    echo          укажите нужный: BUILD_FLASH.cmd /sn:^<SerialNumber^>
    exit /b 1
)
echo   [OK] выбран ST-LINK %FOUND_SN%
exit /b 0

:scan_forced
echo   [i] используется заданный ST-LINK %WANT_SN%
call :probe_one "%WANT_SN%"
if "%FOUND_N%"=="0" if "%TARGETS_N%"=="1" (
    set "FOUND_SN=%TGT_ALL%"
    set "FOUND_N=1"
)
if "%FOUND_N%"=="0" (
    echo.
    echo [ОШИБКА] ST-LINK %WANT_SN% не видит МК !DEVNAME!
    exit /b 1
)
echo   [OK] выбран ST-LINK %FOUND_SN%
exit /b 0

rem --- опрос одной пробы (аргумент: Serial Number) --------------------------
:probe_one
set "PSN=%~1"
set "PSN=%PSN: =%"
if "%PSN%"=="" exit /b 0
rem отбрасываем строки вида "N/A (Old ST-LINK firmware...)" - это не Serial Number
set "SNOK=1"
for /f "delims=0123456789abcdefABCDEF" %%X in ("!PSN!") do set "SNOK=0"
if "!SNOK!"=="0" (
    echo   [-] !PSN! : пропуск ^(это не Serial Number^)
    exit /b 0
)
set "PF=%TMPD%\probe_%PSN%.txt"
set "FAM="
"%STLINK%" -c SN=%PSN% SWD >"%PF%" 2>&1
if errorlevel 1 "%STLINK%" -c SN=%PSN% SWD UR >"%PF%" 2>&1
for /f "usebackq tokens=2 delims=:" %%F in (`findstr /C:"Device family:" "%PF%"`) do if not defined FAM set "FAM=%%F"
if defined FAM if "!FAM:~0,1!"==" " set "FAM=!FAM:~1!"
findstr /C:"Device ID: 0x" "%PF%" >nul
if errorlevel 1 goto probe_skip
set /a TARGETS_N+=1
set "TGT_ALL=%PSN%"
if not defined FAMILY goto probe_match
if not defined FAM goto probe_match
echo "%FAM%" | findstr /i /C:"!FAMILY!" >nul
if errorlevel 1 goto probe_other
:probe_match
echo   [+] %PSN% : %FAM% - совпадает
set "FOUND_SN=%PSN%"
set /a FOUND_N+=1
set "FOUND_ALL=%FOUND_ALL% %PSN%"
exit /b 0
:probe_other
echo   [-] %PSN% : %FAM% - другое семейство
exit /b 0
:probe_skip
echo   [-] %PSN% : нет цели
exit /b 0

rem --- прописать SN в .uvoptx на время прошивки ----------------------------
:ensure_uvoptx_sn
if not defined UVOPTX (
    echo   [i] .uvoptx не найден - SN в проекте не правим
    exit /b 0
)
if not exist "%UVOPTX%.bak" copy /y "%UVOPTX%" "%UVOPTX%.bak" >nul
powershell -NoProfile -ExecutionPolicy Bypass -Command "$p='%UVOPTX%'; $t=[IO.File]::ReadAllText($p); $t=[regex]::Replace($t,'-U[0-9A-Fa-f]{24}','-U%FOUND_SN%'); [IO.File]::WriteAllText($p,$t)"
findstr /C:"-U%FOUND_SN%" "%UVOPTX%" >nul
if errorlevel 1 (
    del /f /q "%UVOPTX%.bak" >nul 2>&1
    echo   [i] в .uvoptx нет записи "-U^<SN^>": выберите адаптер в Keil ^(Options for Target / Debug^)
    exit /b 0
)
set "UVOPTX_PATCHED=1"
echo   [i] в .uvoptx прописан ST-Link %FOUND_SN% ^(исходный файл будет восстановлен^)
exit /b 0

rem --- сборка одного таргета (аргумент: имя таргета) ------------------------
:build_one
set "TG=%~1"
set "BLOG=%TMPD%\build_%TG%.log"
if exist "%BLOG%" del /f /q "%BLOG%" >nul 2>&1
echo.
echo --- сборка %TG% ^(%MODENAME%^) ---
"%UV4%" %MODE% "%PROJ%" -t %TG% -j0 -o "%BLOG%"
set "BRC=!ERRORLEVEL!"
if not exist "%BLOG%" (
    echo [ОШИБКА] uVision не создал лог сборки
    exit /b 1
)
for /f "usebackq delims=" %%L in (`findstr /C:"Program Size" /C:"Error" "%BLOG%"`) do echo     %%L
if !BRC! GEQ 2 (
    echo [ОШИБКА] сборка %TG% завершилась с ошибками ^(код !BRC!^)
    findstr /C:"error:" /C:"Error:" "%BLOG%"
    exit /b 1
)
exit /b 0

rem --- прошивка одного таргета (аргумент: имя таргета) ----------------------
:flash_one
set "TG=%~1"
set "FLOG=%TMPD%\flash_%TG%.log"
if exist "%FLOG%" del /f /q "%FLOG%" >nul 2>&1
echo.
echo --- прошивка %TG% ---
"%UV4%" -f "%PROJ%" -t %TG% -j0 -o "%FLOG%"
set "FRC=!ERRORLEVEL!"
if not exist "%FLOG%" (
    echo [ОШИБКА] uVision не создал лог прошивки
    exit /b 1
)
for /f "usebackq delims=" %%L in (`findstr /C:"Load " /C:"Flash Load finished" /C:"rror" "%FLOG%"`) do echo     %%L
if !FRC! GEQ 2 (
    echo [ОШИБКА] uVision вернул код !FRC!
    exit /b 1
)
findstr /C:"Flash Load finished" "%FLOG%" >nul
if errorlevel 1 (
    echo [ОШИБКА] прошивка %TG% не подтверждена ^(нет "Flash Load finished"^)
    exit /b 1
)
findstr /C:"Verify OK" "%FLOG%" >nul
if errorlevel 1 echo     [i] verify в логе не найден
exit /b 0

rem --- сброс и запуск -------------------------------------------------------
:reset_run
set "RF=%TMPD%\run.txt"
"%STLINK%" -c SN=%FOUND_SN% SWD -Rst -Run >"%RF%" 2>&1
if errorlevel 1 (
    echo   [i] предупреждение: сброс/запуск вернул ошибку
    exit /b 0
)
echo   [OK] МК сброшен и запущен
exit /b 0

rem --- очистка временных файлов и восстановление .uvoptx --------------------
:cleanup
if "%UVOPTX_PATCHED%"=="1" if defined UVOPTX if exist "%UVOPTX%.bak" (
    copy /y "%UVOPTX%.bak" "%UVOPTX%" >nul
    del /f /q "%UVOPTX%.bak" >nul 2>&1
    echo   [i] .uvoptx восстановлен
)
if exist "%TMPD%\probe_*.txt" del /f /q "%TMPD%\probe_*.txt" >nul 2>&1
if exist "%TMPD%\targets.txt" del /f /q "%TMPD%\targets.txt" >nul 2>&1
if exist "%TMPD%\stlink_list.txt" del /f /q "%TMPD%\stlink_list.txt" >nul 2>&1
if exist "%TMPD%\current_target.txt" del /f /q "%TMPD%\current_target.txt" >nul 2>&1
if exist "%TMPD%\run.txt" del /f /q "%TMPD%\run.txt" >nul 2>&1
exit /b 0

rem ------------------------------- справка ---------------------------------
:usage_ok
set "SKIP_SUMMARY=1"
echo.
echo BUILD_FLASH.cmd - сборка и прошивка проекта Keil ^(любой проект, любой STM32^)
echo.
echo   BUILD_FLASH.cmd [ключи]        запускать из корня проекта
echo.
echo   /rebuild         полная пересборка вместо инкрементальной
echo   /buildonly       только собрать
echo   /flashonly       только прошить
echo   /list            показать проект, таргеты и ST-LINK, ничего не делать
echo   /project:ИМЯ     выбрать проект, если в дереве их несколько
echo   /target:ИМЯ      только этот таргет
echo   /all             все таргеты ^(по умолчанию - помеченные для batch build^)
echo   /sn:XXXX         конкретный ST-LINK ^(Serial Number^)
echo   /pause /nopause  пауза в конце / без неё
echo.
echo Примеры:
echo   BUILD_FLASH.cmd
echo   BUILD_FLASH.cmd /rebuild
echo   BUILD_FLASH.cmd /list
echo   BUILD_FLASH.cmd /flashonly /target:pu_new_CM7
set "RC=0"
set "DO_BUILD=0"
set "DO_FLASH=0"
goto finish

:usage_err
set "SKIP_SUMMARY=1"
echo.
echo BUILD_FLASH.cmd - сборка и прошивка проекта Keil
echo   ключи: /rebuild /buildonly /flashonly /list /project:ИМЯ
echo          /target:ИМЯ /all /sn:XXXX /pause /nopause /?
set "RC=1"
goto finish

rem ------------------------------- финал -----------------------------------
:finish
call :cleanup
if defined SKIP_SUMMARY (
    if "%NEED_PAUSE%"=="1" pause
    exit /b !RC!
)
echo.
if "!RC!"=="0" (
    echo === ГОТОВО ===
) else (
    echo === ОШИБКА ^(код !RC!^) ===
)
if defined PROJ echo     проект : %PROJ%
if defined SEQ echo     таргеты: %SEQSHOW%
if defined FOUND_SN echo     ST-LINK: %FOUND_SN%
echo     логи   : %TMPD%
if "%NEED_PAUSE%"=="1" pause
exit /b !RC!
