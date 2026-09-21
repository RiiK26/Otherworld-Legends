@echo off
REM Run this script to start the game and inject the cheat on Windows

set "SCRIPT_DIR=%~dp0"
set "DLL_PATH="
set "INJECTOR_EXE="
set "GAME_EXE=Otherworld Legends.exe"
set "APP_ID=1761380"

if exist "%SCRIPT_DIR%Otherworld-Legends.dll" if exist "%SCRIPT_DIR%injector.exe" (
    set "DLL_PATH=%SCRIPT_DIR%Otherworld-Legends.dll"
    set "INJECTOR_EXE=%SCRIPT_DIR%injector.exe"
) else if exist "%SCRIPT_DIR%..\build\release\Otherworld-Legends.dll" if exist "%SCRIPT_DIR%..\build\release\injector.exe" (
    set "DLL_PATH=%SCRIPT_DIR%..\build\release\Otherworld-Legends.dll"
    set "INJECTOR_EXE=%SCRIPT_DIR%..\build\release\injector.exe"
) else (
    echo Error: Could not find Otherworld-Legends.dll and injector.exe
    pause
    exit /b 1
)

echo Starting Otherworld-Legends...
start steam://rungameid/%APP_ID%

echo Waiting for game to load...
timeout /t 5 /nobreak

echo Injecting Otherworld-Legends.dll...
"%INJECTOR_EXE%" "%GAME_EXE%" "%DLL_PATH%"

echo Injection complete!
pause
