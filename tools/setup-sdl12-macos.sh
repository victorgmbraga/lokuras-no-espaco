#!/bin/sh
# ---------------------------------------------------------------------------
# setup-sdl12-macos.sh - SDL 1.2 local (sem root) para compilar no macOS
#
# O macOS nao traz o SDL 1.2 e o Homebrew nao empacota mais o SDL_image,
# SDL_ttf e SDL_mixer da API 1.2. Este script baixa e compila as cinco
# bibliotecas usadas pelo jogo e instala tudo em tools/sdl12-macos/:
#
#   - sdl12-compat 1.2.78 (a API da SDL 1.2 sobre a SDL 2 do Homebrew)
#   - SDL_image    1.2.12  (PNG e JPG, com o backend comum libpng/libjpeg)
#   - SDL_ttf      2.0.11  (fontes TrueType)
#   - SDL_mixer    1.2.12  (OGG e WAV)
#   - SDL_gfx      2.0.27  (rotozoom e primitivas de desenho)
#
# O SDL_image e compilado com --disable-imageio: o backend ImageIO do macOS
# devolve superficies pretas com o sdl12-compat (as imagens do jogo somem).
#
# Tambem copia do Homebrew a SDL 2 (sdl2-compat ou sdl2) e, quando existe, a
# SDL 3 que o sdl2-compat usa: o sdl12-compat as carrega por dlopen em runtime.
#
# Nada e instalado no sistema: o prefixo fica dentro do projeto e o
# build-macos.sh o usa automaticamente. Requer Xcode Command Line Tools
# (clang/make) e Homebrew (para SDL 2, libpng, jpeg-turbo, freetype, libogg,
# libvorbis, pkg-config e cmake).
#
# Para desfazer: rm -rf tools/sdl12-macos
# Saida: exit 0 = prefixo pronto | exit 1 = falha.
# ---------------------------------------------------------------------------

set -u

raiz="$(cd "$(dirname "$0")/.." && pwd)" || exit 1
prefixo="$raiz/tools/sdl12-macos"
log="$raiz/tools/sdl12-macos-build.log"
# Marca que o SDL_image foi compilado com o backend comum. Prefixos antigos
# (backend ImageIO) nao tem o arquivo e sao recompilados automaticamente.
marcador="$prefixo/.sdl-image-common-backend"

versao_sdl12="1.2.78"
versao_image="1.2.12"
versao_ttf="2.0.11"
versao_mixer="1.2.12"
versao_gfx="2.0.27"

# SHA-256 dos arquivos baixados (verificados antes de compilar).
hash_sdl12="bdc5277cf3a8ae18c7493a7d4d0867c53439a9e5a661133dcf1f2eb0b9ddca4d"
hash_image="0b90722984561004de84847744d566809dbb9daf732a9e503b91a1b5a84e5699"
hash_ttf="724cd895ecf4da319a3ef164892b72078bd92632a5d812111261cde248ebcdb7"
hash_mixer="1644308279a975799049e4826af2cfc787cad2abb11aa14562e402521f86992a"
hash_gfx="dfb15ac5f8ce7a4952dc12d2aed9747518c5e6b335c0e31636d23f93c630f419"

# Alvo minimo do macOS. Ajuda a manter o binario compativel com Macs mais
# antigos; o Homebrew pode exigir uma versao maior em runtime.
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-11.0}"

falhar()
{
	echo "Erro: $1" >&2
	if [ -s "$log" ]; then
		echo "Ultimas linhas de $log:" >&2
		tail -40 "$log" >&2
	fi
	exit 1
}

executar()
{
	descricao="$1"
	shift
	printf '  %s...\n' "$descricao"
	if ! "$@" >> "$log" 2>&1; then
		falhar "$descricao"
	fi
}

baixar()
{
	url="$1"
	destino="$2"
	esperado="$3"

	printf '  baixando %s...\n' "$(basename "$destino")"
	if command -v curl >/dev/null 2>&1; then
		curl -fsSL --retry 3 --retry-delay 1 -o "$destino" "$url" >> "$log" 2>&1 || return 1
	elif command -v wget >/dev/null 2>&1; then
		wget -q --tries=3 -O "$destino" "$url" >> "$log" 2>&1 || return 1
	else
		echo "Nem curl nem wget estao instalados." >&2
		return 1
	fi

	obtido="$(shasum -a 256 "$destino" | cut -d' ' -f1)"
	if [ "$obtido" != "$esperado" ]; then
		echo "SHA-256 nao confere para $(basename "$destino"):" >&2
		echo "  esperado: $esperado" >&2
		echo "  obtido:   $obtido" >&2
		return 1
	fi
}

# --- Runtime da SDL 2 ------------------------------------------------------

# O sdl12-compat nao linka a SDL 2: ele a carrega por dlopen, procurando
# @loader_path/libSDL2-2.0.0.dylib (e alguns fallbacks). No Apple Silicon o
# dyld nao procura /opt/homebrew/lib pelo nome, entao a copia para o prefixo
# local e o que faz o bin/game achar a SDL 2. O sdl2-compat (que implementa a
# API 2.0 sobre a SDL 3) carrega a SDL 3 do mesmo jeito, por isso ela tambem
# vai para o prefixo.
instalar_sdl2_runtime()
{
	origem=""
	for formula in sdl2-compat sdl2; do
		prefixo_formula="$(brew --prefix "$formula" 2>/dev/null || true)"
		[ -n "$prefixo_formula" ] || continue
		for candidato in \
			"$prefixo_formula/lib/libSDL2-2.0.0.dylib" \
			"$prefixo_formula/lib/libSDL2-2.0.dylib"; do
			if [ -e "$candidato" ]; then
				origem="$candidato"
				break
			fi
		done
		[ -n "$origem" ] && break
	done

	if [ -z "$origem" ]; then
		echo "Nao encontrei a SDL 2 para copiar para $prefixo/lib." >&2
		echo "Instale com: brew install sdl2-compat" >&2
		return 1
	fi

	mkdir -p "$prefixo/lib" || return 1
	# O destino pode ser um link de uma execucao anterior; apague antes.
	rm -f "$prefixo/lib/libSDL2-2.0.0.dylib" "$prefixo/lib/libSDL2-2.0.dylib"
	cp "$origem" "$prefixo/lib/libSDL2-2.0.0.dylib" || return 1
	ln -s libSDL2-2.0.0.dylib "$prefixo/lib/libSDL2-2.0.dylib" || return 1

	prefixo_sdl3="$(brew --prefix sdl3 2>/dev/null || true)"
	origem_sdl3=""
	if [ -n "$prefixo_sdl3" ]; then
		for candidato in \
			"$prefixo_sdl3/lib/libSDL3.0.dylib" \
			"$prefixo_sdl3/lib/libSDL3.dylib"; do
			if [ -e "$candidato" ]; then
				origem_sdl3="$candidato"
				break
			fi
		done
	fi
	if [ -n "$origem_sdl3" ]; then
		rm -f "$prefixo/lib/libSDL3.dylib" "$prefixo/lib/libSDL3.0.dylib"
		cp "$origem_sdl3" "$prefixo/lib/libSDL3.0.dylib" || return 1
		ln -s libSDL3.0.dylib "$prefixo/lib/libSDL3.dylib" || return 1
	fi

	return 0
}

# --- Verificacoes iniciais -------------------------------------------------

[ "$(uname -s)" = "Darwin" ] || {
	echo "Este script e para macOS. No Linux use tools/setup-sdl12.sh."
	exit 1
}

for ferramenta in clang++ make curl brew; do
	if ! command -v "$ferramenta" >/dev/null 2>&1; then
		echo "Ferramenta nao encontrada: $ferramenta"
		case "$ferramenta" in
			clang++|make)
				echo "Instale o Xcode Command Line Tools: xcode-select --install"
				;;
			brew)
				echo "Instale o Homebrew: https://brew.sh"
				;;
		esac
		exit 1
	fi
done

brew_prefix="$(brew --prefix)" || exit 1

# Prefixo antigo (SDL_image com ImageIO) deixa as imagens pretas no jogo.
# Avisa antes de recompilar tudo para o usuario saber por que demora.
if [ -e "$prefixo/lib/libSDL_image.dylib" ] && [ ! -e "$marcador" ]; then
	echo "Prefixo SDL 1.2 antigo detectado (SDL_image com backend ImageIO)."
	echo "Recompilando o SDL_image com o backend comum para corrigir as imagens pretas..."
fi

# O prefixo fica pronto quando headers e as cinco bibliotecas existem e o
# SDL_image usa o backend comum (marcador).
if [ -e "$prefixo/include/SDL/SDL.h" ] &&
   [ -e "$prefixo/include/SDL/SDL_image.h" ] &&
   [ -e "$prefixo/include/SDL/SDL_rotozoom.h" ] &&
   [ -e "$prefixo/include/SDL/SDL_ttf.h" ] &&
   [ -e "$prefixo/include/SDL/SDL_mixer.h" ] &&
   [ -e "$prefixo/lib/libSDL.dylib" ] &&
   [ -e "$prefixo/lib/libSDL_image.dylib" ] &&
   [ -e "$prefixo/lib/libSDL_gfx.dylib" ] &&
   [ -e "$prefixo/lib/libSDL_ttf.dylib" ] &&
   [ -e "$prefixo/lib/libSDL_mixer.dylib" ] &&
   [ -e "$marcador" ]; then
	instalar_sdl2_runtime || exit 1
	echo "SDL 1.2 local ja esta em tools/sdl12-macos (runtime da SDL 2 conferido)"
	echo "(para baixar de novo: rm -rf tools/sdl12-macos)"
	exit 0
fi

# --- Dependencias do Homebrew ----------------------------------------------

formulas="sdl2-compat libpng jpeg-turbo freetype libogg libvorbis pkg-config cmake"
faltando=""
for formula in $formulas; do
	if ! brew list --formula "$formula" >/dev/null 2>&1; then
		faltando="$faltando $formula"
	fi
done

if [ -n "$faltando" ]; then
	echo "As formulas do Homebrew necessarias nao estao instaladas:"
	for formula in $faltando; do
		echo "  $formula"
	done

	if [ ! -t 0 ]; then
		echo
		echo "Instale com: brew install$faltando"
		exit 1
	fi

	printf '\nInstalar agora com o Homebrew? [s/N] '
	resposta=""
	read resposta
	case "$resposta" in
		[sSyY]*) ;;
		*) exit 1 ;;
	esac

	# shellcheck disable=SC2086
	brew install $faltando || exit 1
fi

for ferramenta in pkg-config cmake; do
	if ! command -v "$ferramenta" >/dev/null 2>&1; then
		echo "Ferramenta nao encontrada: $ferramenta"
		echo "Instale com: brew install $ferramenta"
		exit 1
	fi
done

# --- Build -----------------------------------------------------------------

tmp="$(mktemp -d "${TMPDIR:-/tmp}/lokuras-sdl12-macos.XXXXXX")" || {
	echo "Nao foi possivel criar um diretorio temporario"
	exit 1
}

concluido=0
limpar()
{
	rm -rf "$tmp"
	if [ "$concluido" -ne 1 ]; then
		rm -rf "$prefixo"
	fi
}
trap limpar EXIT
trap 'exit 1' INT TERM

mkdir -p "$prefixo" "$raiz/tools"
rm -f "$marcador"
: > "$log"

jobs="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

printf '\nBaixando e compilando o SDL 1.2 em tools/sdl12-macos (log: tools/sdl12-macos-build.log)\n'

executar "baixar sdl12-compat" \
	baixar "https://github.com/libsdl-org/sdl12-compat/releases/download/release-$versao_sdl12/sdl12-compat-$versao_sdl12.tar.gz" \
	"$tmp/sdl12-compat.tar.gz" "$hash_sdl12"
executar "baixar SDL_image" \
	baixar "https://www.libsdl.org/projects/SDL_image/release/SDL_image-$versao_image.tar.gz" \
	"$tmp/SDL_image.tar.gz" "$hash_image"
executar "baixar SDL_ttf" \
	baixar "https://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-$versao_ttf.tar.gz" \
	"$tmp/SDL_ttf.tar.gz" "$hash_ttf"
executar "baixar SDL_mixer" \
	baixar "https://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-$versao_mixer.tar.gz" \
	"$tmp/SDL_mixer.tar.gz" "$hash_mixer"
executar "baixar SDL_gfx" \
	baixar "https://www.ferzkopp.net/Software/SDL_gfx-2.0/SDL_gfx-$versao_gfx.tar.gz" \
	"$tmp/SDL_gfx.tar.gz" "$hash_gfx"

for pacote in "sdl12-compat" "SDL_image" "SDL_ttf" "SDL_mixer" "SDL_gfx"; do
	tar -C "$tmp" -xzf "$tmp/$pacote.tar.gz" >> "$log" 2>&1 || falhar "extrair $pacote"
done

# --- sdl12-compat -----------------------------------------------------------

executar "configurar sdl12-compat" \
	cmake -S "$tmp/sdl12-compat-$versao_sdl12" -B "$tmp/build-sdl12" \
		-DCMAKE_INSTALL_PREFIX="$prefixo" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" \
		-DSDL12_COMPAT_INSTALL_SDL2_CONFIG=OFF
executar "compilar sdl12-compat" \
	cmake --build "$tmp/build-sdl12" --parallel "$jobs"
executar "instalar sdl12-compat" \
	cmake --install "$tmp/build-sdl12"

# --- Bibliotecas que usam o sdl-config do prefixo ---------------------------

cflags_base="-O2 -Wno-error=implicit-function-declaration -Wno-incompatible-function-pointer-types"
ldflags_base="-L$prefixo/lib -L$brew_prefix/lib -Wl,-rpath,$prefixo/lib"
cppflags_base="-I$brew_prefix/include"
for formula in libpng jpeg-turbo freetype libogg libvorbis; do
	prefixo_formula="$(brew --prefix "$formula")" || falhar "localizar $formula"
	cppflags_base="$cppflags_base -I$prefixo_formula/include"
	ldflags_base="$ldflags_base -L$prefixo_formula/lib"
done
# O freetype guarda os headers em include/freetype2.
cppflags_base="$cppflags_base -I$(brew --prefix freetype)/include/freetype2"

# SDL_image: so PNG e JPG (os assets do jogo), ligados diretamente para nao
# depender de dlopen em runtime. --disable-imageio forca o backend comum
# (libpng/libjpeg): o backend ImageIO do macOS cria superficies pretas com o
# sdl12-compat, o que deixava todas as imagens do jogo pretas.
cd "$tmp/SDL_image-$versao_image" || falhar "entrar em SDL_image"
executar "configurar SDL_image" \
	env PATH="$prefixo/bin:$PATH" \
		CPPFLAGS="$cppflags_base" \
		CFLAGS="$cflags_base" \
		LDFLAGS="$ldflags_base" \
		./configure --prefix="$prefixo" --disable-sdltest \
			--disable-imageio \
			--disable-tif --disable-webp \
			--disable-jpg-shared --disable-png-shared
executar "compilar SDL_image" make -j"$jobs"
executar "instalar SDL_image" make install
cd "$raiz" || exit 1

# SDL_ttf: o showfont/glfont de teste usa iconv; -liconv no LDFLAGS resolve.
cd "$tmp/SDL_ttf-$versao_ttf" || falhar "entrar em SDL_ttf"
executar "configurar SDL_ttf" \
	env PATH="$prefixo/bin:$PATH" \
		CPPFLAGS="$cppflags_base" \
		CFLAGS="$cflags_base" \
		LDFLAGS="$ldflags_base -liconv" \
		./configure --prefix="$prefixo" --disable-sdltest
executar "compilar SDL_ttf" make -j"$jobs"
executar "instalar SDL_ttf" make install
cd "$raiz" || exit 1

# SDL_mixer: OGG + WAV, com o vorbis ligado diretamente (sem dlopen).
cd "$tmp/SDL_mixer-$versao_mixer" || falhar "entrar em SDL_mixer"
executar "configurar SDL_mixer" \
	env PATH="$prefixo/bin:$PATH" \
		CPPFLAGS="$cppflags_base" \
		CFLAGS="$cflags_base" \
		LDFLAGS="$ldflags_base" \
		./configure --prefix="$prefixo" --disable-sdltest \
			--disable-music-mod --disable-music-midi \
			--disable-music-flac --disable-music-mp3 \
			--disable-music-cmd --disable-music-ogg-shared
executar "compilar e instalar SDL_mixer" make install-hdrs install-lib
cd "$raiz" || exit 1

# SDL_gfx: MMX nao existe em Apple Silicon; compile so a biblioteca.
cd "$tmp/SDL_gfx-$versao_gfx" || falhar "entrar em SDL_gfx"
executar "configurar SDL_gfx" \
	env PATH="$prefixo/bin:$PATH" \
		CPPFLAGS="$cppflags_base" \
		CFLAGS="$cflags_base" \
		LDFLAGS="$ldflags_base" \
		./configure --prefix="$prefixo" --disable-sdltest --disable-mmx
executar "compilar SDL_gfx" make libSDL_gfx.la
executar "instalar SDL_gfx" make install-libLTLIBRARIES install-libSDL_gfxincludeHEADERS install-pkgconfigDATA
cd "$raiz" || exit 1

# --- Runtime da SDL 2 -------------------------------------------------------

printf '  copiando SDL 2 (e SDL 3, se houver) para o prefixo...\n'
instalar_sdl2_runtime || falhar "copiar o runtime da SDL 2"

# --- Verificacao final -----------------------------------------------------

for header in SDL/SDL.h SDL/SDL_image.h SDL/SDL_rotozoom.h SDL/SDL_ttf.h SDL/SDL_mixer.h; do
	[ -e "$prefixo/include/$header" ] || falhar "header ausente: $header"
done
for lib in libSDL.dylib libSDL_image.dylib libSDL_gfx.dylib libSDL_ttf.dylib libSDL_mixer.dylib libSDL2-2.0.0.dylib; do
	[ -e "$prefixo/lib/$lib" ] || falhar "biblioteca ausente: $lib"
done

# Confere o binario, nao so a configuracao: com ImageIO o jogo carrega as
# imagens, mas todos os pixels ficam pretos.
if nm -u "$prefixo/lib/libSDL_image.dylib" 2>/dev/null |
   grep -q 'CGImageSourceCreateWithDataProvider'; then
	falhar "SDL_image ainda usa o backend ImageIO (imagens pretas)"
fi
touch "$marcador" || falhar "criar marcador do backend do SDL_image"

concluido=1
trap - EXIT INT TERM
rm -rf "$tmp"

cat <<FIM

SDL 1.2 local pronto em tools/sdl12-macos
  headers: tools/sdl12-macos/include/SDL
  libs:    tools/sdl12-macos/lib (inclui a SDL 2 e a SDL 3 de runtime)
  SDL_image: backend comum (libpng/libjpeg, sem ImageIO)
  espaco:  $(du -sh "$prefixo" | cut -f1) em disco

O build-macos.sh usa esse prefixo automaticamente (sem sudo e sem mexer no
sistema). Para desfazer: rm -rf tools/sdl12-macos
FIM
exit 0
