@echo off
rem Build do jogo (SDL 1.2, MinGW i686 + msvcrt)
rem Toolchain e SDL 1.2 instalados em C:\Users\victor.braga\dev
set MINGW=C:\Users\victor.braga\dev\extract\mingw32\bin
set SDL=C:\Users\victor.braga\dev\sdl12
cd /d "%~dp0"

if not exist bin mkdir bin

"%MINGW%\g++.exe" -std=gnu++11 -O2 -w -Dmain=SDL_main ^
  -I"%SDL%\include" -I"%SDL%\include\SDL" -I"src" ^
  src\AccObject.cpp src\Animation.cpp src\Audio.cpp src\Camera.cpp src\Earth.cpp ^
  src\FollowerObject.cpp src\GameObject.cpp src\HUD.cpp src\InputManager.cpp src\main.cpp ^
  src\Moon.cpp src\Planet.cpp src\PlanetRed.cpp src\Point.cpp src\SDLBase.cpp ^
  src\Sprite.cpp src\StateGame.cpp src\StateManager.cpp src\StateMenu.cpp ^
  src\StateWinLose.cpp src\Text.cpp src\TileMap.cpp src\TileSet.cpp src\Timer.cpp ^
  -o bin\game.exe ^
  -L"%SDL%\lib" -lmingw32 -lSDLmain -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL ^
  -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
  echo Build OK: bin\game.exe
) else (
  echo Build FALHOU com erro %ERRORLEVEL%
)
pause
