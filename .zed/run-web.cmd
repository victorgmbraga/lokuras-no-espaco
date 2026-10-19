:; d=$(dirname -- "$0"); exec "$d/../run-web.sh" "$@"; exit 1 # ramo sh (o cmd ignora a linha 1, que comeca com ":")
@echo off
rem Wrapper cross-platform das tasks do Zed (.zed/tasks.json) para rodar o jogo
rem web. No Linux/macOS roda como script sh (run-web.sh); no Windows, como
rem .cmd (run-web.bat).
call "%~dp0..\run-web.bat" %*
