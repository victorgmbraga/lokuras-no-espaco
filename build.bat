@echo off
rem Build do Trabalho 4 (SDL 1.2, MinGW i686 + msvcrt)
rem Toolchain e SDL 1.2 instalados em C:\Users\victor.braga\dev
set MINGW=C:\Users\victor.braga\dev\extract\mingw32\bin
set SDL=C:\Users\victor.braga\dev\sdl12
cd /d "%~dp0"

"%MINGW%\g++.exe" -std=gnu++11 -O2 -w -Dmain=SDL_main ^
  -I"%SDL%\include" -I"%SDL%\include\SDL" ^
  AccObject.cpp Animation.cpp Audio.cpp Camera.cpp Earth.cpp FollowerObject.cpp ^
  GameObject.cpp HUD.cpp InputManager.cpp main.cpp Moon.cpp Planet.cpp PlanetRed.cpp ^
  Point.cpp SDLBase.cpp Sprite.cpp StateGame.cpp StateManager.cpp StateMenu.cpp ^
  StateWinLose.cpp Text.cpp TileMap.cpp TileSet.cpp Timer.cpp ^
  -o Trabalho4.exe ^
  -L"%SDL%\lib" -lmingw32 -lSDLmain -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL ^
  -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
  echo Build OK: Trabalho4.exe
) else (
  echo Build FALHOU com erro %ERRORLEVEL%
)
pause
