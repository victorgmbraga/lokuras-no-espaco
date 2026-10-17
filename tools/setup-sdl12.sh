#!/bin/sh
# ---------------------------------------------------------------------------
# setup-sdl12.sh - SDL 1.2 local (sem root) para compilar no Linux
#
# Baixa os pacotes do SDL 1.2 (headers + libs) e as dependencias que faltam no
# sistema, e extrai tudo em tools/sdl12/ - nada e instalado no sistema e nao e
# preciso sudo. Usa apenas apt-get (modo --print-uris), curl/wget e dpkg-deb.
#
# O build.sh usa esse prefixo automaticamente quando ele existe.
# Para desfazer: rm -rf tools/sdl12
#
# Funciona em distros com apt/dpkg (Debian, Ubuntu, Mint e derivados); nas
# outras, instale os pacotes do SDL 1.2 pelo gerenciador de pacotes.
#
# Saida: exit 0 = prefixo pronto | exit 1 = falha.
# ---------------------------------------------------------------------------

set -u

raiz="$(cd "$(dirname "$0")/.." && pwd)" || exit 1
prefixo="$raiz/tools/sdl12"
pacotes="libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev libsdl-gfx1.2-dev"

for ferramenta in apt-get dpkg-deb; do
	if ! command -v "$ferramenta" >/dev/null 2>&1; then
		echo "Este script precisa do apt-get e do dpkg-deb (Debian, Ubuntu, Mint e derivados)."
		echo "Na sua distro, instale os pacotes do SDL 1.2 pelo gerenciador de pacotes."
		exit 1
	fi
done

if [ -e "$prefixo/usr/include/SDL/SDL.h" ]; then
	echo "SDL 1.2 local ja esta em tools/sdl12"
	echo "(para baixar de novo: rm -rf tools/sdl12)"
	exit 0
fi

tmp="$(mktemp -d "${TMPDIR:-/tmp}/lokuras-sdl12.XXXXXX")" || {
	echo "Nao foi possivel criar um diretorio temporario"
	exit 1
}
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' INT TERM

baixar()
{
	if command -v curl >/dev/null 2>&1; then
		curl -fsSL --retry 3 --retry-delay 1 -o "$2" "$1"
	elif command -v wget >/dev/null 2>&1; then
		wget -q --tries=3 -O "$2" "$1"
	else
		echo "Nem curl nem wget estao instalados para baixar os pacotes." >&2
		return 1
	fi
}

verificar_hash()
{
	case "$2" in
		MD5Sum) md5sum "$1" | cut -d' ' -f1 ;;
		SHA1)   sha1sum "$1" | cut -d' ' -f1 ;;
		SHA256) sha256sum "$1" | cut -d' ' -f1 ;;
		SHA512) sha512sum "$1" | cut -d' ' -f1 ;;
		*)      echo "" ;;
	esac
}

# O apt informa, sem instalar nada, quais .deb seriam baixados: sao os pacotes
# do SDL 1.2 e as dependencias que ainda faltam nesta maquina.
echo "Consultando os pacotes do SDL 1.2 (apt-get --print-uris)..."
if ! apt-get install --print-uris -y --no-install-recommends $pacotes > "$tmp/lista.txt" 2> "$tmp/erro.txt"; then
	echo "apt-get falhou:" >&2
	sed 's/^/  /' "$tmp/erro.txt" >&2
	exit 1
fi

mkdir -p "$tmp/debs"
baixados=0
bytes=0

while read -r uri nome tamanho hash; do
	case "$uri" in
		\'*.deb\') ;;
		*) continue ;;
	esac

	uri="${uri#\'}"
	uri="${uri%\'}"
	tipo="${hash%%:*}"
	valor="${hash#*:}"
	destino="$tmp/debs/$nome"

	if ! baixar "$uri" "$destino"; then
		echo "Falha ao baixar: $uri" >&2
		exit 1
	fi

	obtido="$(wc -c < "$destino" | tr -d ' ')"
	if [ "$obtido" != "$tamanho" ]; then
		echo "Download incompleto: $nome ($obtido de $tamanho bytes)" >&2
		exit 1
	fi

	esperado="$(verificar_hash "$destino" "$tipo")"
	if [ -n "$esperado" ] && [ "$esperado" != "$valor" ]; then
		echo "Arquivo corrompido: $nome" >&2
		exit 1
	fi

	baixados=$((baixados + 1))
	bytes=$((bytes + tamanho))
done < "$tmp/lista.txt"

if [ "$baixados" -eq 0 ]; then
	echo "Nada para baixar: o SDL 1.2 ja deve estar instalado no sistema."
	echo "Rode ./build.sh (o prefixo tools/sdl12 nao e necessario nesse caso)."
	exit 1
fi

echo "Baixados $baixados pacotes ($((bytes / 1024)) KB). Extraindo em tools/sdl12..."

rm -rf "$prefixo"
mkdir -p "$prefixo"

for deb in "$tmp/debs"/*.deb; do
	if ! dpkg-deb -x "$deb" "$prefixo"; then
		echo "Falha ao extrair $deb" >&2
		rm -rf "$prefixo"
		exit 1
	fi
done

# So headers e libs interessam: fora documentacao e manuais dos pacotes
rm -rf "$prefixo/usr/share/doc" "$prefixo/usr/share/man"

# Mesma verificacao que o build.sh faz para usar o prefixo: headers + libSDL.so
libdir=""
for dir in "$prefixo"/usr/lib/*-linux-gnu "$prefixo"/usr/lib "$prefixo"/usr/lib64; do
	if [ -e "$dir/libSDL.so" ]; then
		libdir="$dir"
		break
	fi
done

if [ ! -e "$prefixo/usr/include/SDL/SDL.h" ] || [ -z "$libdir" ]; then
	echo "Extracao incompleta: headers ou libSDL.so nao apareceram em tools/sdl12" >&2
	rm -rf "$prefixo"
	exit 1
fi

cat <<FIM

SDL 1.2 local pronto em tools/sdl12
  headers: tools/sdl12/usr/include/SDL
  libs:    ${libdir#"$raiz"/}
  espaco:  $(du -sh "$prefixo" | cut -f1) em disco

O build.sh usa esse prefixo automaticamente (sem sudo e sem mexer no sistema).
Para desfazer: rm -rf tools/sdl12
FIM
exit 0
