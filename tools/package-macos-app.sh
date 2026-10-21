#!/bin/sh
# Empacota o build macOS em um app bundle -> dist/Lokuras no Espaco.app
#
# O bundle leva o binario (bin/game), os assets, as licencas e TODAS as
# bibliotecas dinamicas nao-sistema dentro de Contents/Frameworks (via
# dylibbundler), com os caminhos reescritos e assinatura ad-hoc. Assim o app
# abre por duplo clique e nao depende de tools/sdl12-macos nem do Homebrew.
#
# Rode depois do build:
#   ./build-macos.sh --app
#
# Saida: exit 0 = bundle pronto | exit 1 = falha.

set -u

raiz="$(cd "$(dirname "$0")/.." && pwd)" || exit 1
cd "$raiz" || exit 1

[ "$(uname -s)" = "Darwin" ] || {
	echo "Este script e para macOS."
	exit 1
}

app="$raiz/dist/Lokuras no Espaço.app"
game="$raiz/bin/game"
log="$raiz/tools/package-macos.log"

falhar()
{
	echo "Erro: $1" >&2
	if [ -s "$log" ]; then
		echo "Ultimas linhas de $log:" >&2
		tail -40 "$log" >&2
	fi
	exit 1
}

[ -x "$game" ] || {
	echo "bin/game nao encontrado. Rode ./build-macos.sh primeiro."
	exit 1
}

if ! command -v dylibbundler >/dev/null 2>&1; then
	cat <<'FIM'
dylibbundler nao encontrado (ele copia as bibliotecas para dentro do .app).
Instale com:

  brew install dylibbundler

ou rode o build sem empacotar: ./build-macos.sh
FIM
	if [ -t 0 ] && command -v brew >/dev/null 2>&1; then
		printf '\nInstalar o dylibbundler agora? [s/N] '
		resposta=""
		read resposta
		case "$resposta" in
			[sSyY]*) brew install dylibbundler || exit 1 ;;
			*) exit 1 ;;
		esac
	else
		exit 1
	fi
fi

# --- Prefixo do SDL 1.2 usado no build -------------------------------------

prefixo="${SDL12_PREFIX:-tools/sdl12-macos}"
case "$prefixo" in
	/*) sdl_base="$prefixo" ;;
	*) sdl_base="$raiz/$prefixo" ;;
esac
if [ ! -e "$sdl_base/lib/libSDL.dylib" ]; then
	sdl_base=""
	if command -v sdl-config >/dev/null 2>&1; then
		sdl_base="$(sdl-config --prefix 2>/dev/null || true)"
	fi
fi
[ -n "$sdl_base" ] || {
	echo "Nao encontrei o prefixo do SDL 1.2 usado no build."
	echo "Rode ./build-macos.sh (ou defina SDL12_PREFIX)."
	exit 1
}

brew_prefix="$(command -v brew >/dev/null 2>&1 && brew --prefix || true)"

tmp="$(mktemp -d "${TMPDIR:-/tmp}/lokuras-app-macos.XXXXXX")" || exit 1
concluido=0
limpar()
{
	rm -rf "$tmp"
	if [ "$concluido" -ne 1 ]; then
		rm -rf "$app"
	fi
}
trap limpar EXIT
trap 'exit 1' INT TERM
: > "$log"

# --- Estrutura do bundle ---------------------------------------------------

echo "Empacotando: $app"
rm -rf "$app"
mkdir -p "$app/Contents/MacOS" "$app/Contents/Frameworks" "$app/Contents/Resources"

cp "$game" "$app/Contents/MacOS/game" || falhar "copiar bin/game"
cp -R img sfx map ttf "$app/Contents/Resources/" || falhar "copiar assets"
cp LICENSE CREDITS.md "$app/Contents/Resources/" || falhar "copiar licencas"
if [ -d lib/LICENSES ]; then
	cp -R lib/LICENSES "$app/Contents/Resources/licencas" || falhar "copiar licencas das libs"
fi

# O jogo carrega os assets por caminho relativo e grava o lokuras.rec no
# diretorio atual. O launcher roda a partir de ~/Library/Application Support
# (gravável, inclusive com o app em /Applications) e cria links para os assets
# que ficam dentro do bundle.
cat > "$app/Contents/MacOS/Lokuras" <<'FIM'
#!/bin/sh
dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
data="$HOME/Library/Application Support/Lokuras no Espaco"
if ! mkdir -p "$data" 2>/dev/null; then
	data="$dir/../Resources"
fi
for item in img sfx map ttf; do
	if [ ! -e "$data/$item" ]; then
		rm -f "$data/$item"
		ln -s "$dir/../Resources/$item" "$data/$item" 2>/dev/null || true
	fi
done
cd "$data" || exit 1
exec "$dir/game" "$@"
FIM
chmod +x "$app/Contents/MacOS/Lokuras"

cat > "$app/Contents/Info.plist" <<FIM
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>pt_BR</string>
	<key>CFBundleDisplayName</key>
	<string>Lokuras no Espaço</string>
	<key>CFBundleExecutable</key>
	<string>Lokuras</string>
	<key>CFBundleIconFile</key>
	<string>game</string>
	<key>CFBundleIdentifier</key>
	<string>br.com.victorgmbraga.lokuras-no-espaco</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>Lokuras</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>LSApplicationCategoryType</key>
	<string>public.app-category.arcade-games</string>
	<key>LSMinimumSystemVersion</key>
	<string>${MACOSX_DEPLOYMENT_TARGET:-11.0}</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
FIM

# Icone: recorta o game.jpg em um quadrado e gera o .icns com as ferramentas
# do proprio macOS. Se falhar, o app so fica com o icone generico.
iconset="$tmp/game.iconset"
mkdir -p "$iconset"
if command -v sips >/dev/null 2>&1 && command -v iconutil >/dev/null 2>&1 &&
   sips -s format png -c 720 720 game.jpg --out "$tmp/icon-base.png" >/dev/null 2>&1; then
	for tamanho in 16 32 128 256 512; do
		sips -z "$tamanho" "$tamanho" "$tmp/icon-base.png" \
			--out "$iconset/icon_${tamanho}x${tamanho}.png" >/dev/null 2>&1
		sips -z $((tamanho * 2)) $((tamanho * 2)) "$tmp/icon-base.png" \
			--out "$iconset/icon_${tamanho}x${tamanho}@2x.png" >/dev/null 2>&1
	done
	iconutil -c icns "$iconset" -o "$app/Contents/Resources/game.icns" >> "$log" 2>&1 ||
		echo "Aviso: nao foi possivel gerar o icone (o app usa o icone padrao)."
fi

# --- Bibliotecas dentro do app ---------------------------------------------

echo "Copiando bibliotecas para Contents/Frameworks..."
set -- -od -b -ns \
	-x "$app/Contents/MacOS/game" \
	-d "$app/Contents/Frameworks" \
	-p "@executable_path/../Frameworks" \
	-s "$sdl_base/lib"
[ -n "$brew_prefix" ] && set -- "$@" -s "$brew_prefix/lib"
[ -d /opt/local/lib ] && set -- "$@" -s /opt/local/lib

if ! dylibbundler "$@" < /dev/null >> "$log" 2>&1; then
	falhar "dylibbundler nao conseguiu copiar as bibliotecas"
fi

# O sdl12-compat carrega a SDL 2 por dlopen (ela nao aparece no otool -L do
# jogo). Copie a SDL 2 para o lado do libSDL-1.2.0.dylib; o sdl12-compat
# procura em @loader_path/libSDL2-2.0.0.dylib. O Homebrew atual entrega a API
# 2.0 pelo sdl2-compat, que tambem carrega a SDL 3 por dlopen (como
# libSDL3.dylib): as duas precisam entrar no bundle.
sdl2=""
for candidato in \
	"$sdl_base/lib/libSDL2-2.0.0.dylib" \
	"$sdl_base/lib/libSDL2-2.0.dylib" \
	"$brew_prefix/lib/libSDL2-2.0.0.dylib" \
	"$brew_prefix/lib/libSDL2-2.0.dylib" \
	"$brew_prefix/opt/sdl2-compat/lib/libSDL2-2.0.0.dylib" \
	"$brew_prefix/opt/sdl2/lib/libSDL2-2.0.0.dylib" \
	"/opt/local/lib/libSDL2-2.0.0.dylib"; do
	if [ -n "$candidato" ] && [ -e "$candidato" ]; then
		sdl2="$candidato"
		break
	fi
done

if [ -n "$sdl2" ]; then
	destino_sdl2="$app/Contents/Frameworks/libSDL2-2.0.0.dylib"
	rm -f "$destino_sdl2"
	cp "$sdl2" "$destino_sdl2" || falhar "copiar SDL 2"
	install_name_tool -id "@rpath/libSDL2-2.0.0.dylib" "$destino_sdl2" >> "$log" 2>&1 ||
		falhar "ajustar o id da SDL 2"

	# O rpath do sdl2-compat do Homebrew aponta para fora do bundle. Troque
	# por @loader_path, onde a SDL 3 fica ao lado da SDL 2.
	for rpath in $(otool -l "$destino_sdl2" 2>/dev/null |
		awk '/LC_RPATH/{flag=1} flag && /path /{print $2; flag=0}'); do
		install_name_tool -delete_rpath "$rpath" "$destino_sdl2" >> "$log" 2>&1 || true
	done
	install_name_tool -add_rpath "@loader_path" "$destino_sdl2" >> "$log" 2>&1 || true

	sdl3=""
	for candidato in \
		"$sdl_base/lib/libSDL3.dylib" \
		"$sdl_base/lib/libSDL3.0.dylib" \
		"$brew_prefix/lib/libSDL3.dylib" \
		"$brew_prefix/lib/libSDL3.0.dylib" \
		"$brew_prefix/opt/sdl3/lib/libSDL3.dylib" \
		"/opt/local/lib/libSDL3.dylib"; do
		if [ -n "$candidato" ] && [ -e "$candidato" ]; then
			sdl3="$candidato"
			break
		fi
	done

	if [ -n "$sdl3" ]; then
		destino_sdl3="$app/Contents/Frameworks/libSDL3.dylib"
		rm -f "$destino_sdl3"
		cp "$sdl3" "$destino_sdl3" || falhar "copiar SDL 3"
		install_name_tool -id "@rpath/libSDL3.dylib" "$destino_sdl3" >> "$log" 2>&1 ||
			falhar "ajustar o id da SDL 3"
	else
		echo "Aviso: SDL 3 nao encontrada para copiar; o sdl2-compat pode nao carregar."
	fi
else
	echo "Aviso: SDL 2 nao encontrada para copiar; o app pode precisar do Homebrew."
fi

# --- Versao minima real do bundle ------------------------------------------

# As bibliotecas do Homebrew podem exigir um macOS mais novo que o alvo usado
# no jogo. O LSMinimumSystemVersion tem de refletir a maior exigencia entre
# todos os Mach-O do bundle, senao o Finder deixa abrir e o dyld falha.
versoes=""
for binario in "$app/Contents/MacOS/game" "$app"/Contents/Frameworks/*.dylib; do
	[ -e "$binario" ] || continue
	versao="$(otool -l "$binario" 2>/dev/null |
		awk '/LC_BUILD_VERSION/{flag=1} flag && /minos/{print $2; exit}')"
	[ -n "$versao" ] && versoes="$versoes$versao
"
done
if [ -n "$versoes" ]; then
	versao_minima="$(printf '%s' "$versoes" | sort -t. -k1,1n -k2,2n -k3,3n | tail -n 1)"
	plutil -replace LSMinimumSystemVersion -string "$versao_minima" \
		"$app/Contents/Info.plist" >> "$log" 2>&1 ||
		echo "Aviso: nao foi possivel ajustar o LSMinimumSystemVersion."
else
	versao_minima="${MACOSX_DEPLOYMENT_TARGET:-11.0}"
fi

# --- Verificacao e assinatura ad-hoc ---------------------------------------

# Nenhuma dependencia deve continuar apontando para /opt/homebrew, /usr/local
# ou para o prefixo local: tudo que nao e do sistema tem de estar no bundle.
for binario in "$app/Contents/MacOS/game" "$app"/Contents/Frameworks/*.dylib; do
	[ -e "$binario" ] || continue
	if otool -L "$binario" | tail -n +2 | awk '{print $1}' |
	   grep -Eq '^(/opt/|/usr/local/|/private/|/Users/)'; then
		echo "Dependencia externa encontrada em $binario:" >&2
		otool -L "$binario" | tail -n +2 | grep -E '^(/opt/|/usr/local/|/private/|/Users/)' >&2
		exit 1
	fi
done

# install_name_tool invalida a assinatura; no Apple Silicon o binario precisa
# ser assinado (mesmo que ad-hoc) para o dyld carregar.
for binario in "$app/Contents/MacOS/game" "$app"/Contents/Frameworks/*.dylib; do
	[ -e "$binario" ] || continue
	codesign --force --sign - "$binario" >> "$log" 2>&1 ||
		falhar "assinar $binario"
done

# A assinatura do bundle inteiro pode falhar quando o executavel principal e
# um script (e o jogo real esta em Contents/MacOS/game). Nesse caso os
# binarios ja estao assinados e o app continua executavel localmente.
codesign --force --sign - "$app" >> "$log" 2>&1 ||
	echo "Aviso: assinatura do bundle ignorada (binarios ja assinados)."

echo
echo "App pronto: ${app#"$raiz"/}"
echo "Tamanho: $(du -sh "$app" | cut -f1)"
echo "macOS minimo (bibliotecas incluidas): $versao_minima"
echo "Abra com: open \"${app#"$raiz"/}\""

concluido=1
trap - EXIT INT TERM
rm -rf "$tmp"
exit 0
