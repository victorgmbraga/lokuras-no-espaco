@echo off
rem Build do jogo para o navegador (WebAssembly + Emscripten) -> bin\web\
rem
rem O jogo e o mesmo src/ do desktop; o que muda:
rem   - web\compat\SDL: API da SDL 1.2 implementada sobre a SDL 2 dos ports do
rem     Emscripten (o Emscripten nao oferece mais o port da SDL 1.2);
rem   - web\compat\sdl12_web.cpp: a implementacao dessa camada;
rem   - web\shell.html: a pagina que hospeda o canvas e o IDBFS do recorde.
rem
rem O Emscripten (em++) e resolvido em cascata:
rem   1) variavel de ambiente EMXX, se definida (caminho do em++)
rem   2) tools\emsdk\upstream\emscripten\em++.exe (baixado pelo build ou pelo
rem      tools\setup-emsdk.ps1)
rem   3) em++ no PATH (emsdk ativado no sistema)
rem Se nao achar, chama o tools\setup-emsdk.ps1 (baixa o emsdk para tools\emsdk).
rem Usamos em++ (e nao emcc) porque o link e de C++: o emcc nao puxa a libc++
rem e o link termina em "undefined symbol: operator delete(void*)".
rem Defina NO_PAUSE=1 para nao aguardar tecla ao final (usado pelo run-web.bat).
setlocal EnableExtensions
cd /d "%~dp0"

rem Ambiente do emsdk baixado para o projeto (node/python/em++ no PATH).
if exist "tools\emsdk\emsdk_env.bat" call "tools\emsdk\emsdk_env.bat" >nul 2>&1

set "EMCC_BIN="
if defined EMXX if exist "%EMXX%" set "EMCC_BIN=%EMXX%"
if not defined EMCC_BIN if exist "tools\emsdk\upstream\emscripten\em++.exe" set "EMCC_BIN=%~dp0tools\emsdk\upstream\emscripten\em++.exe"
if not defined EMCC_BIN if exist "tools\emsdk\upstream\emscripten\em++.bat" set "EMCC_BIN=%~dp0tools\emsdk\upstream\emscripten\em++.bat"
if not defined EMCC_BIN for /f "delims=" %%i in ('where em++.exe 2^>nul') do if not defined EMCC_BIN set "EMCC_BIN=%%i"
if not defined EMCC_BIN for /f "delims=" %%i in ('where em++.bat 2^>nul') do if not defined EMCC_BIN set "EMCC_BIN=%%i"
if not defined EMCC_BIN for /f "delims=" %%i in ('where em++ 2^>nul') do if not defined EMCC_BIN set "EMCC_BIN=%%i"

if defined EMCC_BIN goto :compilar

:bootstrap
echo.
echo Emscripten (em++) nao encontrado. Iniciando configuracao automatica...
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\setup-emsdk.ps1"
if exist "tools\emsdk\emsdk_env.bat" call "tools\emsdk\emsdk_env.bat" >nul 2>&1
if exist "tools\emsdk\upstream\emscripten\em++.exe" set "EMCC_BIN=%~dp0tools\emsdk\upstream\emscripten\em++.exe"
if not defined EMCC_BIN if exist "tools\emsdk\upstream\emscripten\em++.bat" set "EMCC_BIN=%~dp0tools\emsdk\upstream\emscripten\em++.bat"
if defined EMCC_BIN goto :compilar
echo.
echo Nao foi possivel configurar o Emscripten automaticamente.
echo Baixe o emsdk em https://emscripten.org/docs/getting_started/downloads.html
echo e rode ".\emsdk install latest" + ".\emsdk activate latest" na pasta baixada
echo (ou defina a variavel de ambiente EMXX apontando para o em++).
echo.
if not defined NO_PAUSE pause
exit /b 1

:compilar
if not exist bin\web mkdir bin\web

rem A lista de fontes e explicita porque o cmd nao expande curingas.
set "FONTES=src\AccObject.cpp src\Animation.cpp src\Audio.cpp src\Camera.cpp src\Earth.cpp src\FollowerObject.cpp src\GameObject.cpp src\HUD.cpp src\InputManager.cpp src\main.cpp src\Moon.cpp src\Planet.cpp src\PlanetRed.cpp src\Point.cpp src\SDLBase.cpp src\Sprite.cpp src\StateGame.cpp src\StateManager.cpp src\StateMenu.cpp src\StateWinLose.cpp src\Text.cpp src\TileMap.cpp src\TileSet.cpp src\Timer.cpp src\Turret.cpp web\compat\sdl12_web.cpp"

rem Flags: mesma linguagem/otimizacao do desktop + a pagina do jogo e os ports
rem (SDL 2 + image/ttf/mixer/gfx). O -lidbfs.js da o /save persistente
rem (IndexedDB); web\pre.js monta/sincroniza esse diretorio.
"%EMCC_BIN%" -std=gnu++11 -O2 -w -Isrc -Iweb\compat ^
  %FONTES% ^
  -o bin\web\index.html ^
  -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=png,jpg ^
  -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=ogg ^
  -sUSE_SDL_GFX=2 ^
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=1MB ^
  -sENVIRONMENT=web -lidbfs.js ^
  --pre-js web\pre.js ^
  --shell-file web\shell.html ^
  --preload-file img@img --preload-file sfx@sfx ^
  --preload-file map@map --preload-file ttf@ttf

if %ERRORLEVEL% EQU 0 (
  echo Build OK: bin\web\index.html
  echo Sirva a pasta bin\web por http - ex.: run-web.bat - e abra no navegador.
) else (
  echo Build FALHOU com erro %ERRORLEVEL%
)
if not defined NO_PAUSE pause
exit /b 0
