#!/bin/sh
# Build do jogo para o navegador (WebAssembly + Emscripten) -> bin/web/
#
# O jogo e o mesmo src/ do desktop; o que muda:
#   - web/compat/SDL: API da SDL 1.2 implementada sobre a SDL 2 dos ports do
#     Emscripten (o Emscripten nao oferece mais o port da SDL 1.2);
#   - web/compat/sdl12_web.cpp: a implementacao dessa camada;
#   - web/shell.html: a pagina que hospeda o canvas e o IDBFS do recorde.
#
# O Emscripten e resolvido em cascata:
#   1) variavel de ambiente EMXX, se definida (caminho do em++)
#   2) tools/emsdk (baixado pelo tools/setup-emsdk.sh, sem root)
#   3) em++ no PATH (emsdk ativado no sistema)
# Usamos em++ (e nao emcc) porque o link e de C++: o emcc nao puxa a libc++
# e o link termina em "undefined symbol: operator delete(void*)".
# Se nao achar, o script explica as opcoes e pergunta antes de baixar/instalar.
#
# Saida: exit 0 = build ok | exit 1 = falha.

set -u

cd "$(dirname "$0")" || exit 1

OUT_DIR="bin/web"
OUT_HTML="$OUT_DIR/index.html"

# Desenvolvimento no Emscripten baixado para o projeto: o emsdk_env.sh coloca
# no PATH o node/python/em++ que vieram junto (silencioso - so afeta este
# script).
if [ -f tools/emsdk/emsdk_env.sh ]; then
	# shellcheck disable=SC1091
	. tools/emsdk/emsdk_env.sh >/dev/null 2>&1 || true
fi

# --- Emscripten (em++) ------------------------------------------------------

EMCC_BIN=""
if [ -n "${EMXX:-}" ] && [ -x "$EMXX" ]; then
	EMCC_BIN="$EMXX"
elif [ -x tools/emsdk/upstream/emscripten/em++ ]; then
	EMCC_BIN="tools/emsdk/upstream/emscripten/em++"
elif command -v em++ >/dev/null 2>&1; then
	EMCC_BIN="$(command -v em++)"
fi

if [ -z "$EMCC_BIN" ]; then
	cat <<'FIM'

O Emscripten (em++) nao foi encontrado. Duas opcoes:

  1) sem instalar nada no sistema: baixa o emsdk para dentro do projeto, em
     tools/emsdk (~1,3 GB de download, ~2,5 GB depois de instalado) -

       ./tools/setup-emsdk.sh

  2) instalar o emsdk no sistema (https://emscripten.org/docs/getting_started/downloads.html)
     e deixar o em++ no PATH.
FIM

	if [ -f tools/setup-emsdk.sh ]; then
		if [ ! -t 0 ]; then
			printf '\nRode ./tools/setup-emsdk.sh e execute o build-web.sh de novo.\n'
			exit 1
		fi

		printf '\nBaixar e instalar o Emscripten agora, dentro do projeto? [s/N] '
		resposta=""
		read resposta
		case "$resposta" in
			[sSyY]*) ;;
			*) exit 1 ;;
		esac

		echo
		sh tools/setup-emsdk.sh || exit 1
		EMCC_BIN="tools/emsdk/upstream/emscripten/em++"
	else
		printf '\nInstale o Emscripten e execute o build-web.sh de novo.\n'
		exit 1
	fi
fi

# --- Build -----------------------------------------------------------------

mkdir -p "$OUT_DIR"

# Flags do em++: mesma linguagem/otimizacao do build do desktop, a pagina do
# jogo e os ports (SDL 2 + image/ttf/mixer/gfx). -lidbfs.js da o /save
# persistente (IndexedDB); web/pre.js monta/sincroniza esse diretorio.
echo "Compilando para WebAssembly: $EMCC_BIN ..."
"$EMCC_BIN" -std=gnu++11 -O2 -w -Isrc -Iweb/compat \
	src/*.cpp web/compat/sdl12_web.cpp \
	-o "$OUT_HTML" \
	-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=png,jpg \
	-sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=ogg \
	-sUSE_SDL_GFX=2 \
	-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB -sSTACK_SIZE=1MB \
	-sENVIRONMENT=web -lidbfs.js \
	--pre-js web/pre.js \
	--shell-file web/shell.html \
	--preload-file img@img --preload-file sfx@sfx \
	--preload-file map@map --preload-file ttf@ttf

status=$?

if [ $status -eq 0 ]; then
	echo "Build OK: $OUT_HTML"
	echo "Sirva a pasta bin/web por http (ex.: ./run-web.sh) e abra no navegador."
	exit 0
fi

echo "Build FALHOU com erro $status"
exit 1
