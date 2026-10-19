:; d=$(dirname -- "$0"); exec "$d/../build-web.sh" "$@"; exit 1 # ramo sh (o cmd ignora a linha 1, que comeca com ":")
@echo off
rem Wrapper cross-platform das tasks do Zed (.zed/tasks.json) para o build web.
rem No Linux/macOS roda como script sh (build-web.sh); no Windows, como .cmd
rem (build-web.bat).
call "%~dp0..\build-web.bat" %*
