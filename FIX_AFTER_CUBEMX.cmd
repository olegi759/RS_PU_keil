@echo off
chcp 65001 >nul
echo === Восстановление проекта RS-PU_v1 после генерации CubeMX ===
powershell -ExecutionPolicy Bypass -File "%~dp0MDK-ARM\fix_after_cubemx.ps1"
echo.
pause

