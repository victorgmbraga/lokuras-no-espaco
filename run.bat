@echo off
rem Executa o jogo a partir da raiz do projeto (os assets usam caminhos relativos)
cd /d "%~dp0"
start "" "bin\game.exe"
