@echo off
rem Executa o jogo a partir da raiz do projeto (os assets usam caminhos relativos).
rem Se bin\game.exe ainda nao existir, compila antes (o build.bat cuida do MinGW).
cd /d "%~dp0"

if not exist "bin\game.exe" (
  echo bin\game.exe nao encontrado - compilando...
  set "NO_PAUSE=1"
  call build.bat
  if not exist "bin\game.exe" (
    echo.
    echo Nao foi possivel compilar o jogo. Veja as mensagens acima.
    pause
    exit /b 1
  )
)

start "" "bin\game.exe"
