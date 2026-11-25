@echo off
chcp 65001 >nul
powershell -ExecutionPolicy Bypass -File "%~dp0generate_and_fix.ps1"
pause

