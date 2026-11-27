# =============================================================================
#  bump_version.ps1 - bump the last component of VERSION by Step (default +1)
#
#  VERSION lives in MDK-ARM\pu_new.uvprojx, field <Define> of BOTH targets:
#      Target 'pu_new_CM4' : ... VERSION=\"2.0.0.0\"
#      Target 'pu_new_CM7' : ... VERSION=\"2.0.0.0\"
#
#  Example: 2.0.0.0 -> 2.0.0.1 -> 2.0.0.2
#  Nothing else in .uvprojx is touched; the file stays valid XML.
#
#  Usage - from the folder where the project is opened:
#      cd D:\DA\2_RS_PU\RS-PU_v4\RS-PU_v4\MDK-ARM
#      ..\bump_version.ps1              (or: powershell -File ..\bump_version.ps1)
#
#  Options:
#      -DryRun            показать, что изменится, ничего не писать
#      -Target CM4|CM7    обновить только одну цель (по умолчанию Both)
#      -Step N            прибавить N (по умолчанию 1)
#      -ProjectPath FILE  путь к .uvprojx (по умолчанию MDK-ARM\pu_new.uvprojx
#                         относительно самого скрипта)
#
#  Safety: on any surprise (no file, VERSION not found, unexpected number of
#  matches) nothing is written and exit code is 1. A backup
#  pu_new.uvprojx.bak_bump_version is created before writing.
#
#  NOTE: keep Keil uVision closed - it locks the project file.
# =============================================================================
[CmdletBinding()]
param(
    [string] $ProjectPath = '',
    [int]    $Step        = 1,
    [ValidateSet('Both', 'CM4', 'CM7')]
    [string] $Target      = 'Both',
    [switch] $DryRun
)

$ErrorActionPreference = 'Stop'

# --- resolve paths ------------------------------------------------------------
# $PSScriptRoot нельзя использовать в значениях param() по умолчанию
# (в Windows PowerShell 5.1 он там ещё пуст), поэтому считаем здесь.
$scriptDir = $PSScriptRoot
if (-not $scriptDir) { $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path }
if (-not $scriptDir) { $scriptDir = (Get-Location).Path }

if (-not $ProjectPath) {
    $ProjectPath = Join-Path $scriptDir 'MDK-ARM\pu_new.uvprojx'
}
elseif (-not [System.IO.Path]::IsPathRooted($ProjectPath)) {
    # относительный путь пользователя - от текущего каталога терминала
    $ProjectPath = Join-Path (Get-Location).Path $ProjectPath
}

$ProjectPath = [System.IO.Path]::GetFullPath($ProjectPath)

if (-not (Test-Path -LiteralPath $ProjectPath)) {
    Write-Host "[ERROR] project not found: $ProjectPath" -ForegroundColor Red
    exit 1
}

# --- read .uvprojx as text ----------------------------------------------------
try {
    $bytes = [System.IO.File]::ReadAllBytes($ProjectPath)
}
catch {
    Write-Host "[ERROR] cannot read (is Keil uVision still open?): $ProjectPath" -ForegroundColor Red
    exit 1
}

$enc  = New-Object System.Text.UTF8Encoding($false)
$text = $enc.GetString($bytes)

# --- which targets to process -------------------------------------------------
switch ($Target) {
    'CM4'   { $wanted = @('CORE_CM4') }
    'CM7'   { $wanted = @('CORE_CM7') }
    default { $wanted = @('CORE_CM4', 'CORE_CM7') }
}

# Regex groups:
#   pre  - 'VERSION=\"' + numeric prefix without the last number
#   last - the last number (this one is incremented)
#   post - closing quote, optionally escaped
$rx = [regex]'(?<pre>VERSION=\\{0,1}"(?:(?:\d+\.)*))(?<last>\d+)(?<post>\\?"?)'

$eol     = "`n"
$lines   = $text -split "`n"
$out     = New-Object System.Collections.Generic.List[string]
$found   = 0
$changed = $false

foreach ($line in $lines) {

    $work = $line

    if ($work.Contains('<Define>')) {

        $isCM4 = $work.Contains('CORE_CM4')
        $isCM7 = $work.Contains('CORE_CM7')

        $wantThis = (($wanted -contains 'CORE_CM4') -and $isCM4) -or
                    (($wanted -contains 'CORE_CM7') -and $isCM7)

        if ($wantThis -and $rx.IsMatch($work)) {

            $m       = $rx.Match($work)
            $pre     = $m.Groups['pre'].Value
            $lastNum = [int]$m.Groups['last'].Value
            $post    = $m.Groups['post'].Value
            $newNum  = $lastNum + $Step

            if ($isCM7) { $label = 'pu_new_CM7' } else { $label = 'pu_new_CM4' }

            $verPrefix = $pre -replace '^VERSION=\\?"', ''

            Write-Host ("{0,-12} VERSION {1}{2} -> {1}{3}" -f $label, $verPrefix, $lastNum, $newNum) -ForegroundColor Yellow

            $work = $work.Remove($m.Index, $m.Length).Insert($m.Index, ($pre + $newNum + $post))

            $found++
            if ($Step -ne 0) { $changed = $true }
        }
    }

    $out.Add($work)
}

# --- validation ---------------------------------------------------------------
if ($Target -eq 'Both') { $expected = 2 } else { $expected = 1 }

if ($found -ne $expected) {
    Write-Host ""
    Write-Host ("[ERROR] VERSION occurrences found: {0}, expected: {1}. File NOT modified." -f $found, $expected) -ForegroundColor Red
    Write-Host ("Path: {0}" -f $ProjectPath)
    Write-Host "Check manually: Options for Target -> C/C++ -> Define"
    exit 1
}

if ($DryRun -or (-not $changed)) {
    Write-Host ""
    Write-Host "[OK] dry run / step 0 - file not modified" -ForegroundColor Cyan
    exit 0
}

# --- write --------------------------------------------------------------------
$backup = $ProjectPath + '.bak_bump_version'
Copy-Item -LiteralPath $ProjectPath -Destination $backup -Force

$newText = [string]::Join($eol, $out)
[System.IO.File]::WriteAllText($ProjectPath, $newText, $enc)

Write-Host ""
Write-Host ("[OK] updated occurrences: {0}" -f $found) -ForegroundColor Green
Write-Host ("Project: {0}" -f $ProjectPath)
Write-Host ("Backup : {0}" -f $backup)
Write-Host "Rebuild both targets (CM4 and CM7) so the new version lands in firmware." -ForegroundColor Cyan
exit 0
