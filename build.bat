@echo off
rem Build do jogo (SDL 1.2, MinGW i686 + msvcrt)
rem SDL 1.2 (headers e libs) versionado em lib\; nao e preciso instalar o SDL.
rem O compilador MinGW e resolvido em cascata:
rem   1) variavel de ambiente MINGW (pasta bin), se definida
rem   2) caminho personalizado abaixo (ajuste para o seu MinGW, se tiver um)
rem   3) pasta instalada pelo auto-bootstrap (%LOCALAPPDATA%\projeto-idj\mingw32\bin)
rem   4) g++ no PATH
rem   5) auto-bootstrap via tools\setup-mingw.ps1 (download do winlibs i686)
setlocal EnableExtensions
cd /d "%~dp0"

set "GXX="
if not defined MINGW set "MINGW=C:\Users\victor.braga\dev\extract\mingw32\bin"
if exist "%MINGW%\g++.exe" set "GXX=%MINGW%\g++.exe"
if not defined GXX if exist "%LOCALAPPDATA%\projeto-idj\mingw32\bin\g++.exe" set "GXX=%LOCALAPPDATA%\projeto-idj\mingw32\bin\g++.exe"
if not defined GXX for /f "delims=" %%i in ('where g++ 2^>nul') do if not defined GXX set "GXX=%%i"

if not defined GXX goto :bootstrap
goto :compilar

:bootstrap
echo.
echo MinGW (g++) nao encontrado no sistema. Iniciando configuracao automatica...
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\setup-mingw.ps1"
if exist "%LOCALAPPDATA%\projeto-idj\mingw32\bin\g++.exe" set "GXX=%LOCALAPPDATA%\projeto-idj\mingw32\bin\g++.exe"
if defined GXX goto :compilar
echo.
echo Nao foi possivel configurar o MinGW automaticamente.
echo Baixe manualmente em:
echo   https://github.com/brechtsanders/winlibs_mingw/releases/tag/16.2.0posix-14.0.0-msvcrt-r2
echo   (winlibs-i686-posix-dwarf-gcc-16.2.0-mingw-w64msvcrt-14.0.0-r2.zip)
echo Extraia o zip e defina a variavel de ambiente MINGW para a pasta "mingw32\bin".
echo.
pause
exit /b 1

:compilar
if not exist bin mkdir bin

"%GXX%" -std=gnu++11 -O2 -w -Dmain=SDL_main ^
  -I"lib\include" -I"lib\include\SDL" -I"src" ^
  src\AccObject.cpp src\Animation.cpp src\Audio.cpp src\Camera.cpp src\Earth.cpp ^
  src\FollowerObject.cpp src\GameObject.cpp src\HUD.cpp src\InputManager.cpp src\main.cpp ^
  src\Moon.cpp src\Planet.cpp src\PlanetRed.cpp src\Point.cpp src\SDLBase.cpp ^
  src\Sprite.cpp src\StateGame.cpp src\StateManager.cpp src\StateMenu.cpp ^
  src\StateWinLose.cpp src\Text.cpp src\TileMap.cpp src\TileSet.cpp src\Timer.cpp ^
  -o bin\game.exe ^
  -L"lib\lib" -lmingw32 -lSDLmain -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL ^
  -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
  echo Build OK: bin\game.exe
) else (
  echo Build FALHOU com erro %ERRORLEVEL%
)
pause
exit /b 0
