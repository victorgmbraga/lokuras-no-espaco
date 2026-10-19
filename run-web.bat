@echo off
rem Executa o jogo no navegador: garante o build e sobe um servidor http.
rem
rem O build WebAssembly carrega o .wasm e o .data por fetch, e navegadores
rem bloqueiam isso em file:// - por isso a pasta bin\web precisa ser servida
rem por http. Feche esta janela para encerrar o servidor.
rem
rem Uso: run-web.bat [porta]      (padrao: 8000)
setlocal EnableExtensions
cd /d "%~dp0"

set "PORT=%~1"
if not defined PORT set "PORT=8000"
set "URL=http://127.0.0.1:%PORT%/"

if not exist bin\web\index.html (
  set "NO_PAUSE=1"
  call build-web.bat
  if not exist bin\web\index.html exit /b 1
)

set "PYTHON="
for /f "delims=" %%i in ('where python 2^>nul') do if not defined PYTHON set "PYTHON=%%i"
if not defined PYTHON (
  echo Python 3 nao encontrado: sirva a pasta bin\web com outro servidor http.
  exit /b 1
)

echo Servindo bin\web em %URL% ^(feche esta janela para encerrar^)...
start "" "%URL%"
"%PYTHON%" -m http.server %PORT% --bind 127.0.0.1 --directory bin\web
