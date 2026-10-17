:; d=$(dirname -- "$0"); exec "$d/../build.sh" "$@"; exit 1 # ramo sh (o cmd ignora a linha 1, que comeca com ":")
@echo off
rem Wrapper cross-platform das tasks do Zed (.zed/tasks.json).
rem No Linux/macOS roda como script sh (build.sh); no Windows, como .cmd (build.bat).
rem O CRLF no fim da linha 1 nao atrapalha o sh: o "#" comenta o "\r".
call "%~dp0..\build.bat" %*
