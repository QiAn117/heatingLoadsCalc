@echo off
chcp 65001 >nul
setlocal
rem ================================================================
rem  One-click build & run (Windows + MinGW)
rem  Usage: double-click this file, or run "run.bat" in 03-Code
rem  The script cd's to its own folder, compiles, then runs.
rem  If envelope.txt / weather.txt are missing, it copies a
rem  standard dataset from ..\04-Data\01_clean\ automatically.
rem  (Kept ASCII-only on purpose: UTF-8 Chinese comments break cmd.)
rem ================================================================
cd /d "%~dp0"

echo == Build ==
gcc -std=c99 -Wall -O2 *.c -o app.exe -lm
if errorlevel 1 (
    echo Build failed: please make sure MinGW gcc is installed and in PATH.
    pause
    exit /b 1
)

if not exist envelope.txt (
    if exist "..\04-Data\01_clean\envelope.txt" (
        copy /y "..\04-Data\01_clean\envelope.txt" . >nul
        copy /y "..\04-Data\01_clean\weather.txt" . >nul
        echo == No input data in this folder; copied ..\04-Data\01_clean ==
    ) else (
        echo [Warning] envelope.txt / weather.txt not found.
        echo           Copy one dataset from 04-Data into this folder, or run app.exe there.
        pause
        exit /b 1
    )
)

echo == Built OK. Running... (try 1 2 3 4 5 6 7 8 0) ==
app.exe
pause
