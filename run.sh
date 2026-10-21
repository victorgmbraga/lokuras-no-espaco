#!/bin/sh
# Executa o jogo a partir da raiz do projeto (os assets usam caminhos relativos).
# Se bin/game ainda nao existir, compila antes (o build.sh cuida do SDL 1.2;
# no macOS ele delega para o build-macos.sh).

set -u

cd "$(dirname "$0")" || exit 1

if [ ! -x bin/game ]; then
	echo "bin/game nao encontrado - compilando..."
	if ! ./build.sh; then
		echo
		echo "Nao foi possivel compilar o jogo. Veja as mensagens acima."
		exit 1
	fi
fi

exec ./bin/game "$@"
