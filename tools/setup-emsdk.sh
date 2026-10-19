#!/bin/sh
# setup-emsdk.sh - instala o Emscripten (emsdk) usado pelo build-web.sh
#
# Chamado pelo build-web.sh quando o em++ nao e encontrado (ou rode na mao).
# Baixa o emsdk (Git + ~1,3 GB do compilador WebAssembly) para tools/emsdk,
# dentro do projeto. Nada e instalado no sistema e nao e preciso sudo.
#
# Saida: exit 0 = pronto para uso | exit 1 = cancelado ou falha.

set -u

cd "$(dirname "$0")/.." || exit 1

EMSDK_VERSION="6.0.12"
EMSDK_DIR="tools/emsdk"
REPO_URL="https://github.com/emscripten-core/emsdk.git"

if [ -x "$EMSDK_DIR/upstream/emscripten/em++" ]; then
	echo "Emscripten ja instalado em $EMSDK_DIR"
	exit 0
fi

if ! command -v git >/dev/null 2>&1; then
	echo "O Git nao foi encontrado no PATH."
	echo "Instale o Git e rode de novo, ou baixe o emsdk manualmente em"
	echo "https://emscripten.org/docs/getting_started/downloads.html"
	exit 1
fi

cat <<FIM

O Emscripten (em++) nao foi encontrado no sistema.

Este script pode baixar o Emscripten automaticamente:
  - emsdk $EMSDK_VERSION (compilador C/C++ -> WebAssembly)
  - download: ~1,3 GB (~2,5 GB depois de instalado)
  - origem:   GitHub - emscripten-core/emsdk
  - destino:  $EMSDK_DIR

FIM

printf 'Baixar e instalar agora? [s/N] '
resposta=""
read resposta
case "$resposta" in
	[sSyY]*) ;;
	*) echo "Cancelado pelo usuario."; exit 1 ;;
esac

if [ -d "$EMSDK_DIR" ]; then
	echo "Removendo instalacao anterior incompleta..."
	rm -rf "$EMSDK_DIR"
fi

echo
echo "Baixando o emsdk (Git)..."
git clone --depth 1 "$REPO_URL" "$EMSDK_DIR" || exit 1

echo
echo "Instalando o SDK $EMSDK_VERSION (download do compilador, pode demorar)..."
( cd "$EMSDK_DIR" && ./emsdk install "$EMSDK_VERSION" ) || exit 1

echo
echo "Ativando o SDK $EMSDK_VERSION..."
( cd "$EMSDK_DIR" && ./emsdk activate "$EMSDK_VERSION" ) || exit 1

if [ ! -x "$EMSDK_DIR/upstream/emscripten/em++" ]; then
	echo "em++ nao encontrado apos a instalacao." >&2
	exit 1
fi

echo
echo "Emscripten instalado em $EMSDK_DIR"
exit 0
