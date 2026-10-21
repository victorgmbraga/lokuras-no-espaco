#!/bin/sh
# Build do jogo no macOS (SDL 1.2 via sdl12-compat + Homebrew) -> bin/game
#
# Equivalente macOS do build.sh (Linux) e do build.bat (Windows). O macOS nao
# traz o SDL 1.2 e o Homebrew nao empacota mais o SDL_image/SDL_ttf/SDL_mixer
# da API 1.2; o tools/setup-sdl12-macos.sh compila as cinco bibliotecas em
# tools/sdl12-macos (sem root). Este script usa esse prefixo automaticamente.
#
# Uso:
#   ./build-macos.sh         compila -> bin/game
#   ./build-macos.sh --app   compila e cria dist/Lokuras no Espaco.app
#
# O compilador e resolvido em cascata:
#   1) variavel de ambiente CXX, se definida (aceita flags, ex.: CXX="ccache clang++")
#   2) clang++ no PATH
#   3) g++ no PATH
# O prefixo SDL 1.2 e resolvido em cascata:
#   1) variavel SDL12_PREFIX, se definida
#   2) tools/sdl12-macos (criado pelo tools/setup-sdl12-macos.sh)
#   3) sdl-config do sistema/Homebrew/MacPorts (com image/ttf/mixer/gfx juntos)
#
# Saida: exit 0 = build ok | exit 1 = falha.

set -u

cd "$(dirname "$0")" || exit 1

criar_app=0
for argumento in "$@"; do
	case "$argumento" in
		--app) criar_app=1 ;;
		-h|--help)
			sed -n '2,22p' "$0" | sed 's/^# \{0,1\}//'
			exit 0
			;;
		*)
			echo "Opcao desconhecida: $argumento (use --help)" >&2
			exit 1
			;;
	esac
done

[ "$(uname -s)" = "Darwin" ] || {
	echo "Este script e para macOS. No Linux use ./build.sh."
	exit 1
}

# Mesmas flags do build.sh/build.bat, sem o que e exclusivo do Windows e com
# o alvo minimo de macOS. -Wno-c++11-narrowing e especifico do clang e entra
# so quando ele for o compilador (o g++ do macOS costuma ser o proprio clang).
CFLAGS="-std=gnu++11 -O2 -w"
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-11.0}"

# --- Compilador ------------------------------------------------------------

if [ -z "${CXX:-}" ]; then
	for candidato in clang++ g++; do
		if command -v "$candidato" >/dev/null 2>&1; then
			CXX="$candidato"
			break
		fi
	done
fi

if [ -z "${CXX:-}" ] || ! command -v "${CXX%% *}" >/dev/null 2>&1; then
	if [ -n "${CXX:-}" ]; then
		echo "Compilador nao encontrado: $CXX (variavel CXX)."
	else
		cat <<'FIM'
Nenhum compilador C++ encontrado (clang++ ou g++).
Instale o Xcode Command Line Tools:
  xcode-select --install
FIM
	fi
	exit 1
fi

if $CXX -dM -E -x c++ /dev/null 2>/dev/null | grep -q '__clang__'; then
	CFLAGS="$CFLAGS -Wno-c++11-narrowing"
fi

# --- SDL 1.2 ---------------------------------------------------------------

SDL12_CFLAGS=""
SDL12_LDFLAGS=""
SDL12_EXTRA_LIBS="-lSDL_image -lSDL_gfx -lSDL_ttf -lSDL_mixer"
SDL12_SDL_LIBS="-lSDLmain -lSDL -Wl,-framework,Cocoa"
SDL12_USANDO_LOCAL=0
SDL12_BASE=""

detectar_sdl12()
{
	SDL12_CFLAGS=""
	SDL12_LDFLAGS=""
	SDL12_SDL_LIBS="-lSDLmain -lSDL -Wl,-framework,Cocoa"
	SDL12_USANDO_LOCAL=0
	SDL12_BASE=""

	prefixo="${SDL12_PREFIX:-tools/sdl12-macos}"
	case "$prefixo" in
		/*) base="$prefixo" ;;
		*) base="$PWD/$prefixo" ;;
	esac

	if [ -e "$base/include/SDL/SDL.h" ] && [ -e "$base/lib/libSDL.dylib" ]; then
		SDL12_BASE="$base"
		SDL12_CFLAGS="-I$base/include -I$base/include/SDL -D_THREAD_SAFE"
		SDL12_LDFLAGS="-L$base/lib -Wl,-rpath,$base/lib"
		SDL12_USANDO_LOCAL=1
		return
	fi

	if [ -n "${SDL12_PREFIX:-}" ]; then
		echo "SDL12_PREFIX=$SDL12_PREFIX nao tem SDL 1.2 - procurando no sistema" >&2
	fi

	if command -v sdl-config >/dev/null 2>&1; then
		prefixo_sistema="$(sdl-config --prefix 2>/dev/null || true)"
		if [ -n "$prefixo_sistema" ] &&
		   [ -e "$prefixo_sistema/include/SDL/SDL.h" ] &&
		   [ -e "$prefixo_sistema/include/SDL/SDL_image.h" ] &&
		   [ -e "$prefixo_sistema/include/SDL/SDL_rotozoom.h" ] &&
		   [ -e "$prefixo_sistema/include/SDL/SDL_ttf.h" ] &&
		   [ -e "$prefixo_sistema/include/SDL/SDL_mixer.h" ]; then
			SDL12_CFLAGS="$(sdl-config --cflags) -I$prefixo_sistema/include"
			SDL12_SDL_LIBS="$(sdl-config --libs)"
			return
		fi
	fi
}

detectar_sdl12

# Compila e linka um programa minimo que usa uma funcao de cada biblioteca:
# se passar, o jogo tambem compila. O programa de teste nunca e executado.
sdl_disponivel()
{
	local dir

	dir="$(mktemp -d "${TMPDIR:-/tmp}/lokuras-sdl12-macos.XXXXXX" 2>/dev/null)" || return 0

	cat > "$dir/teste.cpp" <<'FIM'
#include <cstdio>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	SDL_Surface *sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, 8, 8, 32, 0, 0, 0, 0);
	SDL_Surface *zoom = rotozoomSurface(sprite, 0.0, 1.0, 0);

	printf("%p %s %s %s %s\n", (void *)zoom, SDL_GetError(), IMG_GetError(),
	       TTF_GetError(), Mix_GetError());

	SDL_FreeSurface(sprite);
	SDL_FreeSurface(zoom);
	return 0;
}
FIM

	$CXX $CFLAGS $SDL12_CFLAGS "$dir/teste.cpp" -o "$dir/teste" \
		$SDL12_LDFLAGS $SDL12_EXTRA_LIBS $SDL12_SDL_LIBS -lm -liconv >/dev/null 2>&1
	local disponivel=$?

	rm -rf "$dir"
	return $disponivel
}

if ! sdl_disponivel; then
	if [ "$SDL12_USANDO_LOCAL" -eq 1 ]; then
		cat <<FIM

O prefixo local $SDL12_BASE esta incompleto ou nao linka.
Para refazer:

  rm -rf tools/sdl12-macos
  ./tools/setup-sdl12-macos.sh
FIM
		exit 1
	fi

	cat <<'FIM'

O SDL 1.2 (SDL, SDL_image, SDL_gfx, SDL_ttf e SDL_mixer) nao foi encontrado.
As libs de lib/ sao do MinGW e nao servem no macOS. A opcao mais simples e
compilar tudo dentro do projeto, sem root:

  ./tools/setup-sdl12-macos.sh

Isso baixa e compila sdl12-compat + SDL_image/SDL_ttf/SDL_mixer/SDL_gfx em
tools/sdl12-macos (requer Xcode Command Line Tools e Homebrew).
FIM

	if [ ! -f tools/setup-sdl12-macos.sh ]; then
		exit 1
	fi

	if [ ! -t 0 ]; then
		printf '\nRode ./tools/setup-sdl12-macos.sh e execute o build-macos.sh de novo.\n'
		exit 1
	fi

	printf '\nBaixar e compilar o SDL 1.2 agora, dentro do projeto? [s/N] '
	resposta=""
	read resposta
	case "$resposta" in
		[sSyY]*) ;;
		*) exit 1 ;;
	esac

	echo
	sh tools/setup-sdl12-macos.sh || exit 1
	detectar_sdl12

	if ! sdl_disponivel; then
		echo "O SDL 1.2 continua indisponivel - veja as mensagens acima."
		exit 1
	fi
fi

# O sdl12-compat carrega a SDL 2 por dlopen em runtime; o teste de compilacao
# acima nao cobre essa dependencia. Sem a copia local o jogo mostra
# "sdl12-compat: Failed loading SDL2 library." e aborta no macOS.
if [ "$SDL12_USANDO_LOCAL" -eq 1 ] && [ ! -e "$SDL12_BASE/lib/libSDL2-2.0.0.dylib" ]; then
	cat <<FIM

O sdl12-compat carrega a SDL 2 em runtime, mas ela nao esta em:
  $SDL12_BASE/lib/libSDL2-2.0.0.dylib

Rode ./tools/setup-sdl12-macos.sh de novo para copia-la (e a SDL 3,
se necessario), ou copie a SDL 2 para:
  $SDL12_BASE/lib/libSDL2-2.0.0.dylib
FIM
	exit 1
fi

# O SDL_image 1.2.12 do macOS usa ImageIO por padrao, que deixa todas as
# imagens pretas com o sdl12-compat. O setup grava um marcador depois de
# compilar com o backend comum (libpng/libjpeg); prefixos antigos nao tem.
prefixo_padrao="$PWD/tools/sdl12-macos"
if [ "$SDL12_USANDO_LOCAL" -eq 1 ] &&
   [ "$SDL12_BASE" = "$prefixo_padrao" ] &&
   [ ! -e "$SDL12_BASE/.sdl-image-common-backend" ]; then
	cat <<FIM

O SDL_image deste prefixo foi compilado com o backend ImageIO do macOS,
que carrega as imagens mas deixa todos os pixels pretos. Para corrigir:

  ./tools/setup-sdl12-macos.sh
FIM

	if [ -t 0 ] && [ -f tools/setup-sdl12-macos.sh ]; then
		printf '\nRecompilar o SDL_image com o backend comum agora? [s/N] '
		resposta=""
		read resposta
		case "$resposta" in
			[sSyY]*) ;;
			*) exit 1 ;;
		esac
		echo
		sh tools/setup-sdl12-macos.sh || exit 1
		detectar_sdl12
	else
		exit 1
	fi
fi

# --- Build -----------------------------------------------------------------

mkdir -p bin

echo "Compilando: $CXX ..."
if [ "$SDL12_USANDO_LOCAL" -eq 1 ]; then
	echo "SDL 1.2: $SDL12_BASE (local)"
else
	echo "SDL 1.2: prefixo do sistema"
fi

$CXX $CFLAGS $SDL12_CFLAGS -Isrc src/*.cpp -o bin/game \
	$SDL12_LDFLAGS $SDL12_EXTRA_LIBS $SDL12_SDL_LIBS -lm -liconv
status=$?

if [ $status -ne 0 ]; then
	echo "Build FALHOU com erro $status"
	exit 1
fi

echo "Build OK: bin/game"
echo "Execute: ./run.sh (ou ./bin/game a partir da raiz do projeto)"

if [ "$criar_app" -eq 1 ]; then
	echo
	sh tools/package-macos-app.sh || exit 1
fi

exit 0
