#!/bin/sh
# Executa o jogo no navegador: garante o build e sobe um servidor estatico.
#
# O build WebAssembly carrega o .wasm e o .data por fetch, e navegadores
# bloqueiam isso em file:// - por isso a pasta bin/web precisa ser servida por
# http. Ctrl+C encerra o servidor.
#
# Uso: ./run-web.sh [porta]      (padrao: 8000; a variavel PORT tambem vale)

set -u

cd "$(dirname "$0")" || exit 1

PORT="${1:-${PORT:-8000}}"
URL="http://127.0.0.1:$PORT/"

if [ ! -f bin/web/index.html ]; then
	./build-web.sh || exit 1
fi

if command -v python3 >/dev/null 2>&1; then
	PYTHON=python3
elif command -v python >/dev/null 2>&1; then
	PYTHON=python
else
	echo "Python 3 nao encontrado: sirva a pasta bin/web com outro servidor http." >&2
	exit 1
fi

echo "Servindo bin/web em $URL (Ctrl+C encerra)..."
if command -v xdg-open >/dev/null 2>&1; then
	( sleep 1; xdg-open "$URL" >/dev/null 2>&1 ) &
elif command -v open >/dev/null 2>&1; then
	( sleep 1; open "$URL" >/dev/null 2>&1 ) &
fi

exec "$PYTHON" -m http.server "$PORT" --bind 127.0.0.1 --directory bin/web
