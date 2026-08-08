@echo off
title GPA7 G-03T USB Firmware Flashing Helper (MedFlow)
echo =======================================================================
echo     GPA7 G-03T USB Firmware Flashing Helper (USBDL Mode)
echo =======================================================================
echo.
echo Step 1: Searching for latest compiled BIN file in project root...

set LATEST_BIN=
for /f "delims=" %%F in ('dir /b /o-d "%~dp0..\GPA7_Pill_*.bin" 2^>nul') do (
    set LATEST_BIN=%%F
    goto :found
)

:found
if "%LATEST_BIN%"=="" (
    echo [ERROR] No GPA7_Pill_*.bin found in project root!
    echo Please run build script first.
    pause
    exit /b 1
)

echo [FOUND] Latest Firmware Binary: %LATEST_BIN%
echo Path: "%~dp0..\%LATEST_BIN%"
echo.
echo Step 2: Checking USB Connection (USBDL Mode)...
echo Please ensure G-03T Type-C cable is connected to PC USB port.
echo.
echo Step 3: Launching Generalplus USB Burner Tool...
python "%~dp0flash_usb.py" "%~dp0..\%LATEST_BIN%"
pause
