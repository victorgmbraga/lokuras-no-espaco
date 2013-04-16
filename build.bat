@echo off
rem Build do Trabalho 1 (SDL 1.2, MinGW i686 + msvcrt)
rem Toolchain e SDL 1.2 instalados em C:\Users\victor.braga\dev
set MINGW=C:\Users\victor.braga\dev\extract\mingw32\bin
set SDL=C:\Users\victor.braga\dev\sdl12
cd /d "%~dp0"

"%MINGW%\g++.exe" -std=gnu++11 -O2 -w -Dmain=SDL_main ^
  -I"%SDL%\include" -I"%SDL%\include\SDL" ^
  GameManager.cpp main.cpp SDLBase.cpp Sprite.cpp ^
  -o Trabalho1.exe ^
  -L"%SDL%\lib" -lmingw32 -lSDLmain -lSDL_image -lSDL ^
  -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
  echo Build OK: Trabalho1.exe
) else (
  echo Build FALHOU com erro %ERRORLEVEL%
)
pause
