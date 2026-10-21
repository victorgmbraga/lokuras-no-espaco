#!/bin/sh
# Build do jogo no Linux (SDL 1.2 do sistema) -> bin/game
# No macOS este script delega para o build-macos.sh (SDL 1.2 via Homebrew +
# sdl12-compat, com as bibliotecas 1.2 compiladas em tools/sdl12-macos).
#
# Equivalente Linux do build.bat. Os arquivos de lib\ sao import libs do MinGW
# (Windows) e nao servem aqui: no Linux o SDL 1.2 (SDL, SDL_image, SDL_gfx,
# SDL_ttf e SDL_mixer, com headers) vem do prefixo local tools/sdl12 - criado
# pelo tools/setup-sdl12.sh, sem root - ou do gerenciador de pacotes.
#
# O compilador e resolvido em cascata:
#   1) variavel de ambiente CXX, se definida (aceita flags, ex.: CXX="ccache g++")
#   2) g++ no PATH
#   3) clang++ no PATH
# Se o SDL 1.2 nao estiver disponivel, o script explica como baixa-lo para
# dentro do projeto (sem root) ou instala-lo no sistema, e pergunta antes de
# executar.
#
# Saida: exit 0 = build ok | exit 1 = falha.

set -u

cd "$(dirname "$0")" || exit 1

# No macOS o SDL 1.2 vem do sdl12-compat + bibliotecas compiladas por
# tools/setup-sdl12-macos.sh; o build tem flags e empacotamento proprios.
if [ "$(uname -s)" = "Darwin" ]; then
	exec ./build-macos.sh "$@"
fi

# Mesmas flags do build.bat, sem o que e exclusivo do Windows (-Dmain=SDL_main,
# -mwindows, -lmingw32/-lSDLmain e -static-libgcc/-static-libstdc++: no Linux
# nao existe WinMain e as bibliotecas sao as .so do sistema).
# A ordem dos -l importa: quem usa a SDL vem antes dela propria.
CFLAGS="-std=gnu++11 -O2 -w"
LIBS="-lSDL_image -lSDL_gfx -lSDL_ttf -lSDL_mixer -lSDL -lm"

# --- Compilador ------------------------------------------------------------

if [ -z "${CXX:-}" ]; then
	for candidato in g++ clang++; do
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
Nenhum compilador C++ encontrado (g++ ou clang++).
Instale um deles e rode de novo:
  Debian/Ubuntu/Mint: sudo apt install build-essential
  Fedora:             sudo dnf install gcc-c++
  Arch:               sudo pacman -S base-devel
FIM
	fi
	exit 1
fi

# --- SDL 1.2 ---------------------------------------------------------------

# SDL 1.2 baixado para dentro do projeto pelo tools/setup-sdl12.sh tem
# prioridade sobre o do sistema (assim da para compilar sem instalar nada);
# SDL12_PREFIX aponta para outro prefixo, se voce ja tiver um.
SDL12_CFLAGS=""
SDL12_LDFLAGS=""

detectar_sdl12()
{
	local base prefixo diretorio

	SDL12_CFLAGS=""
	SDL12_LDFLAGS=""
	prefixo="${SDL12_PREFIX:-tools/sdl12}"

	if [ ! -d "$prefixo" ]; then
		if [ -n "${SDL12_PREFIX:-}" ]; then
			echo "SDL12_PREFIX=$SDL12_PREFIX nao existe - usando o SDL 1.2 do sistema" >&2
		fi
		return
	fi

	case "$prefixo" in
		/*) base="$prefixo" ;;
		*) base="$PWD/$prefixo" ;;
	esac

	for diretorio in "$base"/usr/lib/*-linux-gnu "$base"/usr/lib "$base"/usr/lib64; do
		if [ -e "$diretorio/libSDL.so" ]; then
			SDL12_CFLAGS="-I$base/usr/include"
			# O rpath precisa ser do tipo antigo (DT_RPATH, via
			# --disable-new-dtags): o moderno (DT_RUNPATH) so vale para as
			# dependencias diretas do executavel, e as libs do prefixo
			# trazem as suas proprias (libmikmod, libmad, ...) no mesmo
			# diretorio.
			SDL12_LDFLAGS="-L$diretorio -Wl,-rpath,$diretorio -Wl,--disable-new-dtags"
			return
		fi
	done

	echo "Prefixo $prefixo nao tem libSDL.so - usando o SDL 1.2 do sistema" >&2
}

detectar_sdl12

# Compila e linka um programa minimo que usa uma funcao de cada biblioteca: se
# passar, o jogo tambem compila. Evita despejar dezenas de erros do compilador
# quando o SDL 1.2 (ou o SDL_gfx, ou o SDL_mixer...) nao esta instalado.
# O programa de teste nunca e executado - so compilado e linkado.
sdl_disponivel()
{
	local dir

	# sem TMPDIR utilizavel: deixa o build normal reportar o que estiver faltando
	dir="$(mktemp -d "${TMPDIR:-/tmp}/lokuras-sdl12.XXXXXX" 2>/dev/null)" || return 0

	cat > "$dir/teste.cpp" <<'FIM'
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

int main()
{
	SDL_Surface *sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, 8, 8, 32, 0, 0, 0, 0);
	SDL_Surface *zoom = rotozoomSurface(sprite, 0.0, 1.0, 0);

	printf("%p %s %s %s %s\n", (void *)zoom, SDL_GetError(), IMG_GetError(),
	       TTF_GetError(), Mix_GetError());

	SDL_FreeSurface(sprite);
	SDL_FreeSurface(zoom);
	return 0;
}
FIM

	$CXX $CFLAGS $SDL12_CFLAGS "$dir/teste.cpp" -o "$dir/teste" $SDL12_LDFLAGS $LIBS >/dev/null 2>&1
	local disponivel=$?

	rm -rf "$dir"
	return $disponivel
}

# Nome dos pacotes varia entre distros (e entre versoes da mesma distro); os do
# Debian/Ubuntu foram conferidos. Se o gerenciador recusar o nome, instale o
# pacote equivalente do SDL 1.2.
instalar()
{
	if command -v apt-get >/dev/null 2>&1; then
		echo "$SUDO apt-get install -y libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev libsdl-gfx1.2-dev"
	elif command -v dnf >/dev/null 2>&1; then
		echo "$SUDO dnf install -y sdl12-compat-devel SDL_image-devel SDL_ttf-devel SDL_mixer-devel SDL_gfx-devel"
	elif command -v pacman >/dev/null 2>&1; then
		echo "$SUDO pacman -S --needed sdl12-compat sdl_image sdl_ttf sdl_mixer sdl_gfx"
	elif command -v zypper >/dev/null 2>&1; then
		echo "$SUDO zypper install libSDL-devel libSDL_image-devel libSDL_ttf-devel libSDL_mixer-devel libSDL_gfx-devel"
	fi
}

SUDO="sudo"
if [ "$(id -u)" = 0 ] || ! command -v sudo >/dev/null 2>&1; then
	SUDO=""
fi

if ! sdl_disponivel; then
	comando="$(instalar)"

	cat <<'FIM'

As bibliotecas de desenvolvimento do SDL 1.2 nao foram encontradas.
O SDL versionado em lib\ e do MinGW e nao serve no Linux. Duas opcoes:

  1) sem instalar nada no sistema (Debian, Ubuntu, Mint): baixa os pacotes do
     SDL 1.2 e extrai dentro do projeto, em tools/sdl12 -

       ./tools/setup-sdl12.sh

  2) instalar no sistema pelo gerenciador de pacotes -
FIM

	if [ -n "$comando" ]; then
		printf '\n     %s\n' "$comando"
	else
		printf '\n     (nao identifiquei o gerenciador de pacotes da sua distro)\n'
	fi

	acao=""
	if [ -f tools/setup-sdl12.sh ] && command -v apt-get >/dev/null 2>&1; then
		acao="local"
	elif [ -n "$comando" ]; then
		acao="sistema"
	fi

	if [ -z "$acao" ]; then
		printf '\nInstale os pacotes do SDL 1.2 e rode o build.sh de novo.\n'
		exit 1
	fi

	if [ ! -t 0 ]; then
		printf '\nExecute um dos comandos acima e rode o build.sh de novo.\n'
		exit 1
	fi

	if [ "$acao" = "local" ]; then
		printf '\nBaixar o SDL 1.2 para tools/sdl12 agora (sem root)? [s/N] '
	else
		printf '\nExecutar o comando de instalacao agora (pede senha do sudo)? [s/N] '
	fi

	resposta=""
	read resposta
	case "$resposta" in
		[sSyY]*) ;;
		*) exit 1 ;;
	esac

	echo
	if [ "$acao" = "local" ]; then
		sh tools/setup-sdl12.sh || exit 1
		detectar_sdl12
	else
		sh -c "$comando" || exit 1
	fi

	if ! sdl_disponivel; then
		echo "O SDL 1.2 continua indisponivel - veja as mensagens acima."
		exit 1
	fi
fi

# --- Build -----------------------------------------------------------------

mkdir -p bin

echo "Compilando: $CXX ..."
$CXX $CFLAGS $SDL12_CFLAGS -Isrc src/*.cpp -o bin/game $SDL12_LDFLAGS $LIBS
status=$?

if [ $status -eq 0 ]; then
	echo "Build OK: bin/game"
	exit 0
fi

echo "Build FALHOU com erro $status"
exit 1
