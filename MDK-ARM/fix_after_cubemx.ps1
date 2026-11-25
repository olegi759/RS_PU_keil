# fix_after_cubemx.ps1
# Восстанавливает файлы, которыми управляет STM32CubeMX, после генерации кода и пересобирает оба ядра.
# CubeMX при генерации: заново добавляет в проект уже включённые файлы (main.c, HAL rcc/flash/gpio,
# system_*.c) -> дубли и L6200E; кладёт main.c ОБОИХ ядер в каждую цель; теряет часть include-путей;
# и удаляет ручной код, находящийся вне блоков USER CODE (MX_GPIO_Init, MPU_Config, hqspi и т.д.).
# Поэтому после каждой генерации запускайте этот скрипт: он вернёт проверенные файлы и пересоберёт проект.
# Запуск:  powershell -ExecutionPolicy Bypass -File .\fix_after_cubemx.ps1
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)   # ...\RS-PU_v1 good 1eth
$snap = Join-Path $root '.cubemx_snapshot'
if (-not (Test-Path $snap)) { Write-Host "нет снимка .cubemx_snapshot" -ForegroundColor Red; exit 1 }
Write-Host "=== Восстановление файлов проекта после CubeMX ==="
Get-ChildItem $snap -Recurse -File | ForEach-Object {
    $rel = $_.FullName.Substring($snap.Length + 1)
    $dst = Join-Path $root $rel
    Copy-Item $_.FullName $dst -Force
    Write-Host ("  " + $rel)
}
$uv4  = 'C:\Keil_v5\UV4\UV4.exe'
$proj = Join-Path $root 'MDK-ARM\pu_new.uvprojx'
$dir  = Join-Path $root 'MDK-ARM'
$ok = $true
foreach ($tgt in @('pu_new_CM4','pu_new_CM7')) {
    $log = Join-Path $dir ("fix_" + $tgt + ".log")
    & $uv4 -r $proj -t $tgt -j0 -o $log | Out-Null
    $code = $LASTEXITCODE
    $txt  = Get-Content $log -ErrorAction SilentlyContinue
    $size = ($txt | Select-String 'Program Size').Line
    $err  = ($txt | Select-String 'Error\(s\)').Line
    Remove-Item $log -Force -ErrorAction SilentlyContinue
    Write-Host ("  [" + $tgt + "] exit=" + $code + "  " + $size + "  " + $err)
    if ($code -ne 0 -or $err -notmatch '0 Error') { $ok = $false }
}
if ($ok) { Write-Host "=== СБОРКА OK: 0 ошибок, 0 предупреждений ===" -ForegroundColor Green }
else { Write-Host "=== СБОРКА С ОШИБКАМИ ===" -ForegroundColor Red; exit 1 }
