# Lokuras no Espaço

![CI](https://github.com/victorgmbraga/lokuras-no-espaco/actions/workflows/ci.yml/badge.svg)
![Gameplay de Lokuras no Espaço](game.jpg)

**Jogue no navegador:** <https://victorgmbraga.github.io/lokuras-no-espaco/>

Jogo 2D em **C++ com SDL 1.2**, desenvolvido para a disciplina **Introdução ao Desenvolvimento de Jogos (IDJ)** da Universidade de Brasília (UnB/CIC, Turma A, 2013 — Profª. Carla Denise Castanho). O mesmo código compila para **Windows**, **Linux**, **macOS** e **navegador (WebAssembly)**.

Este repositório reconstrói todo o **fluxo de desenvolvimento da disciplina** em um histórico Git linear: cada entrega (trabalho **TR** ou exercício **EX**) virou um commit, partindo do game loop básico (TR01) até a game engine completa com máquina de estados (TR04), que dá origem ao jogo **Lokuras no Espaço**.

## O jogo

Você pilota a nave **Hyperion**, perdida no espaço e caçada por alienígenas. Sua única esperança é voltar para a **Terra** — mas o caminho é cheio de planetas, torretas automáticas, uma lua indesejável e um UFO teimosamente em seu encalço.

- **Objetivo (modo travessia, `4`):** alcançar a Terra para vencer. A seta amarela na borda da tela aponta a direção dela quando está fora do campo de visão. Chegar à Terra **sem levar nenhum dano** rende a conquista "INTACTO". A nave começa com **100** de vida (barra + contador no HUD).
- **Modo pontos (`5`):** sem Terra, Lua ou gravidade — **sobreviva 3:00** para vencer ("VOCE SOBREVIVEU!", +500). A 30 s do fim aparece o aviso "FALTAM 30 S!". É o modo score attack: recorde separado por dificuldade.
- **UFO (chefe em ondas):** caça a nave **sozinho** e acelera com o tempo (mas nunca passa da velocidade base da nave, para a fuga ser sempre possível). Tem **barra de vida própria** no HUD e muda de fase conforme apanha:
  - **Fase 1** (mais de 2/3 da vida): apenas persegue;
  - **Fase 2** (entre 1/3 e 2/3): passa a **atirar** projéteis na direção da nave (dano 6/8/10 no fácil/normal/difícil);
  - **Fase 3** (menos de 1/3): ainda mais rápido, atira em **leque de 3** e invoca planetas extras.

  Destruí-lo **não encerra a partida**: após ~6,5 s uma **nova onda** chega mais forte (mais vida e +3% de velocidade por onda, com aviso na tela). Uma **seta vermelha** na borda indica a direção do UFO fora da tela. O UFO derrotado sempre larga **combustível + escudo** no ponto da morte — caçá-lo vira decisão de risco/recompensa. Encostar custa **30 de vida** (com i-frames de 0,8 s); o escudo bloqueia.
- **Tiro:** `ESPAÇO` ou **Mouse 1** disparam na direção do bico da nave (cadência de ~4,5 tiros/s; uma linha curta no bico mostra a direção). Cada planeta vermelho cai com **2 tiros**; os tiros são **bloqueados pelos painéis da estação** — use-os como cobertura (o tiro que morre no painel deixa uma faísca).
- **Mapa aleatório:** a cada partida o mapa é gerado com **18 a 22 tiles de lado** (máximo de 22×22 = 1650×1650 px, maior que a tela), em duas camadas com parallax. Os painéis da **camada da frente não colidem com a nave** — ela os atravessa —, mas os planetas ricocheteiam neles e os tiros (seus e inimigos) param ali. A câmera persegue a nave com **lookahead** (antecipa a direção do movimento) e nunca mostra fora do mundo.
- **Início da partida:** a nave começa com **3 s de escudo** (anel visível) para ninguém morrer nos primeiros segundos; o mapa já começa com **2 a 4 planetas** e **14 a 18 asteroides pequenos** espalhados. A **Terra** e o **UFO** surgem em posições aleatórias, sempre longe da nave e um do outro.
- **Planetas:** os **vermelhos** (2 tiros, dano 5, 10 pts) surgem sozinhos em ritmo crescente (~4 s → ~1,3 s). Os **roxos** (novos: 4 tiros, deriva lenta, dano 8, 25 pts e **drop quase garantido**) aparecem em ~25% dos spawns. Colidir custa vida e destrói o planeta — e o abalroamento também pontua (metade do valor) e conta no combo. A lua orbita a Terra e custa **10**.
- **Asteroides pequenos:** já nascem com a partida, derivam devagar e quicam nas bordas do mundo. Encostar causa **2 de dano** (bem menos que o planeta vermelho), mas eles são **atravessáveis**: não bloqueiam a nave, não somem e não param tiros.
- **Torretas:** torres fixas ancoradas nos painéis do mapa (2 a 6, conforme a dificuldade). Giram na direção da nave e **só atiram com linha de visada livre** (painéis bloqueiam; tiro que morre no painel deixa uma faísca); caem com 2 tiros (30 pts, 40% de drop). Tiro inimigo (torres e UFO) causa 6/8/10 de dano no fácil/normal/difícil.
- **Minas de proximidade:** a partir dos 45 s, 15% dos spawns viram minas (limite de 4 ativas). Derivam devagar e quicam como planetas, mas **armam espoleta de 600 ms** quando a nave chega a 110 px (com "!" de aviso) e **explodem em área** (raio 130, dano 12). Uma explosão **detona as vizinhas** a 140 px — limpe de longe: 1 tiro rende 15 pts.
- **Pontuação e combo:** abates valem pontos (planeta 10/25, torreta 30, chefe 300×onda, abalroamento metade); abates emendados criam um **combo que multiplica em até ×3** (janela de 4 s no primeiro minuto e 3 s depois, exibido no HUD). Sobreviver rende +10 a cada 10 s; vencer soma +500.
- **Combustível:** a barra laranja abaixo da vida **drena com o tempo**; aos 25% aparece o aviso "COMBUSTIVEL BAIXO!" e, quando zera, a nave perde **3 de vida por segundo** ("SEM COMBUSTIVEL!"). Cristais **amarelos** reabastecem +40%.
- **Power-ups** surgem a cada 15–25 s e também **caem de abates** (chance por tipo). Cada tipo tem **cor e forma próprias** (marcador branco sobre o cristal; legenda no menu):
  - **Verde (vida, cruz):** recupera 20 de vida (até o limite de 100);
  - **Azul (escudo, anel):** 6 s de invulnerabilidade a planetas, lua, UFO e tiros (anel em volta da nave);
  - **Laranja (turbo, seta):** velocidade e aceleração **×1,6** por 5 s;
  - **Amarelo (combustível, losango):** reabastece +40% do tanque.
- **Invulnerabilidade:** após levar dano a nave fica **0,8 s intocável** (piscando) — vale para planetas, lua, UFO e tiros inimigos; nesse intervalo ela atravessa planetas sem destruí-los.
- **Tutorial dos primeiros 30 s:** dicas curtas na tela ensinam só o que falta (impulso, giro, tiro e o objetivo) — quem já fez a ação não recebe a dica.
- **Feedback de combate:** textos flutuantes de **pontos (+25)** e **dano (-30)** nos impactos, "!" sobre a nave quando há **tiro inimigo a menos de 180 px**, avisos grandes na tela ("UFO DESTRUÍDO!", "ONDA 2: o UFO voltou mais forte!", conquistas), barra de vida **fixa ou piscando** conforme o flash reduzido, explosão em **cadeia** ao matar o chefe, tremor de tela e motor da nave com **hum** proporcional à aceleração.
- **Acessibilidade:** tecla **F** alterna o **flash reduzido** (barras e nave param de piscar); volume de **música (+/-)** e de **efeitos (9/0)** separados (título da janela mostra os dois níveis); **M** continua como mudo geral.
- **Dificuldade (teclas 1/2/3 no menu):** FÁCIL / NORMAL / DIFÍCIL ajustam o ritmo dos spawns, a vida e a velocidade do chefe, a cadência **e o dano** dos tiros inimigos (6/8/10), o número de torretas, o dano recebido e o dreno do combustível. O menu mostra o resumo do que cada nível muda.
- **Conquistas (salvas):** ATERRISSAGEM, CACADOR DE UFO, EXTERMINADOR (50 planetas somando todas as partidas), INTACTO (vencer sem dano), SOBREVIVENTE (5 min numa partida), EM CHAMA (combo x3) e RESISTENCIA (vencer no modo pontos) — a lista aparece no menu e cada nova conquista é anunciada na hora.
- **Desafio do dia:** `?daily=1` no navegador (ou `--daily` no desktop) faz **todos jogarem o mesmo mapa e grava a melhor do dia** no ranking local (melhor por dia, últimos 7 dias); o menu mostra a sua melhor de hoje e convida a jogar.
- **Semente fixa / desafio do dia:** `?seed=N` no navegador faz **todos jogarem o mesmo mapa e posições**; `?daily=1` usa o dia atual como semente. No desktop: `game.exe --seed N` ou `--daily`.
- **Gravidade da Terra:** perto do destino, a nave sente uma "corrente" que a puxa — chegue com cuidado.
- **Recorde por modo e dificuldade:** a pontuação da partida (combo + sobrevivência + bônus de vitória) é salva em `lokuras.rec` no navegador/PC, com contadores acumulados e conquistas; o menu mostra o recorde do modo e dificuldade selecionados.
- **Câmera** com lookahead na direção do movimento e aproximação suave, presa aos limites do mundo; após o fim da partida ela fica livre (setas).

### Controles

| Entrada | Ação |
| --- | --- |
| `ENTER` | Iniciar o jogo (menu) / jogar de novo (vitória ou derrota) |
| `1` / `2` / `3` (no menu) | Dificuldade: fácil / normal / difícil (o recorde é salvo por modo e dificuldade) |
| `4` / `5` (no menu) | Modo: travessia (alcance a Terra) / pontos (sobreviva 3:00) |
| `W` / `S` | Acelerar / frear a nave (movimento acelerado com atrito) |
| `A` / `D` | Rotacionar a nave |
| `ESPAÇO` ou `Mouse 1` ou botão 0 do gamepad | Atirar na direção do bico da nave |
| `ESC` | Pausar o jogo; no menu encerra; nas telas finais volta ao menu |
| `H` | Mostrar/esconder as dicas do rodapé (somem sozinhas aos 12 s) |
| `F` | Alternar flash reduzido (sem piscadas de alerta) |
| `Q` (na pausa) | Voltar ao menu |
| `+` / `=` e `-` / `KP-` | Volume da música acima/abaixo (mostra no título da janela) |
| `9` / `0` | Volume dos efeitos abaixo/acima (mostra no título da janela) |
| `M` | Mudo (a qualquer momento) |
| `Alt+Enter` | Alternar entre janela e tela cheia (em qualquer estado) |
| Gamepad | Analógico esquerdo proporcional (acelera e gira); botão 0 atira |

Os planetas **não são criados pelo jogador**: a partida já começa com alguns espalhados pelo mapa e novos surgem automaticamente, em posições aleatórias, com frequência crescente. A Terra, o UFO e as torretas também são sorteados a cada partida — na web, `?seed=N` ou `?daily=1` fixam o sorteio para todos os jogadores.

A janela inicia em **1280×720** (16:9). O `Alt+Enter` alterna para tela cheia e volta para a janela sem reiniciar o jogo.

### Estados

`StateMenu` (título, escolha de dificuldade com 1/2/3, recorde por dificuldade e lista de conquistas) → `StateGame` (jogabilidade, pausável com ESC) → `StateWinLose` (tela de vitória ou derrota com estatísticas da partida).

Cada estado tem música tema; o estado de jogo tem `stateGame.ogg` e o SFX `boom.wav`. As trocas de estado usam **fade** (escurece, troca, clareia), e o loop roda com **dt real** limitado a 50 ms por frame, com alvo de ~60 FPS (sem busy-loop).

## Como compilar e executar

O mesmo `src/` compila para Windows (`build.bat`/`run.bat`), para Linux (`build.sh`/`run.sh`), para macOS (`build-macos.sh`/`run.sh`) e para o navegador (`build-web.bat`/`run-web.bat` e `build-web.sh`/`run-web.sh`), a partir da raiz do projeto.

### Windows

**Requisitos**

- Windows 10+;
- MinGW-w64 **i686** (msvcrt, posix) com `g++` — ou deixe o `build.bat` baixar automaticamente (veja abaixo).

As bibliotecas **SDL 1.2 + SDL_image + SDL_gfx + SDL_ttf + SDL_mixer** (headers e libs de importação do MinGW) já estão versionadas no próprio repositório, em `lib/` — não é preciso instalar o SDL para compilar no Windows.

O script `build.bat` gera `bin\game.exe` e resolve o compilador em cascata: variável de ambiente `MINGW` → `tools\mingw32` → `PATH`. Se o `g++` não for encontrado, ele chama `tools/setup-mingw.ps1`, que **oferece baixar o toolchain winlibs i686** (GCC 16.2.0, msvcrt, ~270 MB): download com retomada, verificação de **SHA-256** e extração em `tools\mingw32` — tudo dentro da pasta do projeto (apagar o projeto remove também o toolchain baixado).

Para usar um MinGW já existente, defina a variável de ambiente `MINGW`:

```bat
set MINGW=<caminho>\mingw32\bin
```

A linha de compilação usada é, resumidamente:

```bat
g++ -std=gnu++11 -O2 -w -Dmain=SDL_main -I"lib\include" -I"lib\include\SDL" -I"src" ^
  src\AccObject.cpp src\Animation.cpp src\Audio.cpp src\Camera.cpp src\Earth.cpp ^
  src\FollowerObject.cpp src\GameObject.cpp src\HUD.cpp src\InputManager.cpp src\main.cpp ^
  src\Moon.cpp src\Planet.cpp src\PlanetRed.cpp src\Point.cpp src\SDLBase.cpp ^
  src\Sprite.cpp src\StateGame.cpp src\StateManager.cpp src\StateMenu.cpp ^
  src\StateWinLose.cpp src\Text.cpp src\TileMap.cpp src\TileSet.cpp src\Timer.cpp src\Turret.cpp ^
  -o bin\game.exe ^
  -mwindows -L"lib\lib" -lmingw32 -lSDLmain -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL ^
  -static-libgcc -static-libstdc++
```

As DLLs de runtime necessárias estão versionadas em `bin/` (SDL.dll, SDL_image.dll, SDL_ttf.dll, SDL_mixer.dll e dependências como `libpng15-15`, `zlib1`, `libjpeg-8`, `libfreetype-6`, as bibliotecas de OGG `libogg-0`/`libvorbis-0`/`libvorbisfile-3` — o áudio do jogo é OGG — e as runtimes `libstdc++-6`/`libgcc_s_dw2-1` exigidas por ele), junto do executável. Depois de compilar, execute `run.bat` — que compila automaticamente caso `bin\game.exe` ainda não exista — ou `bin\game.exe` diretamente a partir da raiz do projeto, pois o jogo carrega os assets por caminhos relativos.

### Linux

**Requisitos**

- `g++` (ou `clang++`);
- **SDL 1.2** (SDL, SDL_image, SDL_gfx, SDL_ttf e SDL_mixer) — do sistema ou de um prefixo local, veja abaixo.

As libs de `lib/` são import libs do MinGW e valem apenas para o build Windows. No Linux o SDL 1.2 pode vir do sistema ou de um prefixo dentro do projeto, criado pelo `tools/setup-sdl12.sh`: ele baixa os pacotes `.deb` do SDL 1.2 (~13 MB de download, ~65 MB extraídos, mais as dependências que faltarem no sistema) e extrai em `tools/sdl12/` usando só `apt-get --print-uris`, `curl`/`wget` e `dpkg-deb` — nada é instalado no sistema e não é preciso `sudo`. Vale para Debian, Ubuntu, Mint e derivados; para desfazer, `rm -rf tools/sdl12`.

```sh
./tools/setup-sdl12.sh   # opcional: SDL 1.2 local, sem root (Debian/Ubuntu/Mint)
./build.sh               # compila para bin/game (usa tools/sdl12 se existir)
./run.sh                 # roda o jogo (compila antes, se necessário)
```

Se preferir o SDL do sistema, é o mesmo conjunto de pacotes que o script baixa (o `libsdl1.2-dev` atual do Debian/Ubuntu é o *sdl12-compat*, que implementa a API 1.2 sobre o SDL 2):

```sh
sudo apt install libsdl1.2-dev libsdl-image1.2-dev libsdl-ttf2.0-dev libsdl-mixer1.2-dev libsdl-gfx1.2-dev
```

O `build.sh` resolve o compilador em cascata (`CXX` → `g++` → `clang++`), prefere o prefixo local `tools/sdl12` quando ele existe (`SDL12_PREFIX` aponta para outro) e, se não achar o SDL 1.2 em lugar nenhum, explica as duas opções acima e pergunta antes de baixar ou instalar. A linha de compilação é a mesma do Windows sem o que é específico de lá (`-Dmain=SDL_main`, `-mwindows`, `-lmingw32`/`-lSDLmain` e `-static-libgcc`/`-static-libstdc++`); com o prefixo local entram também `-Itools/sdl12/usr/include`, `-Ltools/sdl12/usr/lib/<arch>` e um `-Wl,-rpath` para o executável achar essas libs sozinho (por isso o binário fica ligado ao prefixo: apagar `tools/sdl12` pede um `./build.sh` de novo):

```sh
g++ -std=gnu++11 -O2 -w -Isrc src/*.cpp -o bin/game \
  -lSDL_image -lSDL_gfx -lSDL_ttf -lSDL_mixer -lSDL -lm
```

### macOS

**Requisitos**

- macOS 11+ (Apple Silicon ou Intel) com **Xcode Command Line Tools** (`xcode-select --install`). O jogo usa alvo 11.0, mas o `.app` pode exigir uma versão maior por causa das bibliotecas do Homebrew — o empacotador grava a exigência real no `LSMinimumSystemVersion`;
- **Homebrew**, para SDL 2 (`sdl2-compat`) e SDL 3, libpng, jpeg-turbo, freetype, libogg, libvorbis, `pkg-config` e `cmake`;
- `dylibbundler` (opcional, só para gerar o `.app` autocontido).

O macOS não traz o SDL 1.2 e o Homebrew não empacota mais o SDL_image/SDL_ttf/SDL_mixer da API 1.2. O `tools/setup-sdl12-macos.sh` baixa e compila as cinco bibliotecas usadas pelo jogo — **sdl12-compat**, **SDL_image**, **SDL_ttf**, **SDL_mixer** e **SDL_gfx** — dentro de `tools/sdl12-macos/`, sem root. Na primeira execução ele oferece instalar as fórmulas que faltarem no Homebrew (~4,5 MB no prefixo local; para desfazer, `rm -rf tools/sdl12-macos`).

O **SDL_image** é compilado com `--disable-imageio`, usando o backend comum (libpng/libjpeg): o backend ImageIO do macOS carrega as imagens mas deixa todos os pixels pretos sob o sdl12-compat. Prefixos antigos são detectados e recompilados automaticamente pelo setup (ou pelo `build-macos.sh`, que oferece rodar o setup).

O `sdl12-compat` não linka a SDL 2: carrega `libSDL2-2.0.0.dylib` por `dlopen` em runtime. No Apple Silicon o dyld não procura `/opt/homebrew/lib` pelo nome, então o setup também copia a SDL 2 (e a SDL 3, usada pelo `sdl2-compat` do Homebrew) para `tools/sdl12-macos/lib`. Sem essa cópia o jogo exibe `sdl12-compat: Failed loading SDL2 library.` e aborta. Se o prefixo já existir, rode o setup de novo para ele completar a cópia.

```sh
./tools/setup-sdl12-macos.sh   # uma vez: SDL 1.2 local (sem root)
./build-macos.sh               # compila -> bin/game
./run.sh                       # roda o jogo (compila antes, se necessário)
./build-macos.sh --app         # compila e cria dist/Lokuras no Espaço.app
```

O `build.sh` detecta o macOS e delega para o `build-macos.sh`, então o `run.sh` e as tasks do Zed continuam valendo. O prefixo local é usado automaticamente (`SDL12_PREFIX` aponta para outro) e o compilador é resolvido em cascata (`CXX` → `clang++` → `g++`). A linha de compilação é a mesma do Linux, com `-Wno-c++11-narrowing` no clang e alvo mínimo `MACOSX_DEPLOYMENT_TARGET=11.0` (sobrescreva a variável se precisar):

```sh
clang++ -std=gnu++11 -O2 -w -Wno-c++11-narrowing -Isrc src/*.cpp -o bin/game \
  -Itools/sdl12-macos/include -Itools/sdl12-macos/include/SDL \
  -Ltools/sdl12-macos/lib -Wl,-rpath,tools/sdl12-macos/lib \
  -lSDL_image -lSDL_gfx -lSDL_ttf -lSDL_mixer -lSDLmain -lSDL \
  -Wl,-framework,Cocoa -lm -liconv
```

O `bin/game` fica ligado ao prefixo `tools/sdl12-macos` e às bibliotecas do Homebrew (como no Linux, o binário não é autocontido). Para gerar um app de duplo clique que não depende de nada disso, use `./build-macos.sh --app`: o bundle copia todas as bibliotecas não-sistema para `Contents/Frameworks` — inclusive a SDL 2 e a SDL 3, que o `sdl12-compat` e o `sdl2-compat` carregam em runtime —, reescreve os caminhos com o `dylibbundler`, gera o ícone a partir de `game.jpg` e assina ad-hoc — o resultado fica em `dist/Lokuras no Espaço.app` (~11 MB) e pode ser aberto com `open "dist/Lokuras no Espaço.app"`. O launcher mantém os assets dentro do bundle e grava o recorde em `~/Library/Application Support/Lokuras no Espaco/lokuras.rec`, então o app pode ser movido para `/Applications`. A assinatura é ad-hoc: se o `.app` for baixado de outro Mac, o Gatekeeper pode pedir **Ajustes → Privacidade e Segurança → Abrir Mesmo Assim** (ou clique com o botão direito → **Abrir**).

### Builds automáticos (GitHub Actions)

O workflow [`ci.yml`](.github/workflows/ci.yml) compila o jogo para **Linux**, **Windows** e **macOS** em todo push e pull request (e também pelo botão **Run workflow**, na aba Actions). Os pacotes ficam nos **Artifacts** do run:

| Artefato | Conteúdo |
| --- | --- |
| `lokuras-linux-x86_64` | `.tar.gz` com `game`, `jogar.sh`, assets e `LEIA-ME.txt` (binário dinâmico; requer o runtime do SDL 1.2) |
| `lokuras-windows-x86` | `.zip` com `game.exe`, DLLs, assets, `jogar.bat` e `LEIA-ME.txt` (não precisa instalar nada) |
| `lokuras-macos` | `.tar.gz` com `Lokuras no Espaço.app` (bibliotecas do SDL 1.2 e do Homebrew dentro de `Contents/Frameworks`, assinatura ad-hoc) |

Os três jobs de desktop rodam um smoke test de 5 s com os drivers dummy da SDL antes de publicar o pacote — se o jogo sair ou crashar, o job falha e o artefato não é gerado. O `build-windows` usa o MSYS2 com o MinGW i686, a mesma ABI (msvcrt) das import libs da SDL versionadas em `lib/`. O `build-macos` instala as dependências pelo Homebrew, compila o SDL 1.2 local com `tools/setup-sdl12-macos.sh` e empacota o `.app` com `./build-macos.sh --app`. O `build-linux` também roda o smoke test do desktop; o `build-web` compila o WebAssembly e, em `master`, publica o GitHub Pages.

### Navegador (WebAssembly)

O jogo também roda no navegador: o `src/` é compilado para **WebAssembly** com [Emscripten](https://emscripten.org/) e hospedado em uma página com canvas 1280×720. Não é preciso instalar nada para jogar — a pasta `bin/web/` é autossuficiente (HTML + JS + WASM + assets embutidos) e o recorde fica no **IndexedDB** do navegador. A versão publicada está em **<https://victorgmbraga.github.io/lokuras-no-espaco/>**.

**Requisitos**

- Emscripten (`emsdk`). O `build-web.bat`/`build-web.sh` baixam o emsdk para `tools/emsdk` automaticamente se o `em++` não for encontrado (~1,3 GB de download, ~2,5 GB extraído), do mesmo jeito que o `build.bat` faz com o MinGW. Também dá para instalar o emsdk no sistema e deixar o `em++` no `PATH`.

```sh
./tools/setup-emsdk.sh   # opcional: baixa/instala o emsdk em tools/emsdk (sem root)
./build-web.sh           # compila -> bin/web/index.html (+ .js/.wasm/.data)
./run-web.sh             # serve bin/web em http://127.0.0.1:8000/ e abre o navegador
```

```bat
tools\setup-emsdk.ps1    rem opcional: baixa/instala o emsdk em tools\emsdk
build-web.bat            rem compila -> bin\web\index.html (+ .js/.wasm/.data)
run-web.bat              rem serve bin\web em http://127.0.0.1:8000\ e abre o navegador
```

O build web precisa ser servido por **http** (o navegador bloqueia o fetch do `.wasm`/`.data` em `file://`): o `run-web.*` sobe um `python -m http.server` em `bin/web` — qualquer outro servidor estático também serve. A resolução do Emscripten é `EMXX` (variável de ambiente) → `tools/emsdk` → `PATH`; o link usa o `em++` (e não o `emcc`) para puxar a libc++ do C++.

**Como funciona**

- O Emscripten não oferece mais o port da **SDL 1.2**, então `web/compat/` reimplementa a parte da API 1.2 usada pelo jogo sobre a **SDL 2** dos ports `sdl2`, `sdl2_image`, `sdl2_ttf`, `sdl2_mixer` e `sdl2_gfx` — o `src/` continua exatamente o mesmo do desktop. A "tela" continua sendo uma surface de software; o `SDL_Flip` envia a surface para uma textura e desenha no canvas.
- `web/shell.html` é a página do jogo (canvas que se adapta à janela, tela de "Começar" que libera o áudio e controles) e `web/pre.js` monta o `/save` (**IDBFS**) onde o `lokuras.rec` é persistido.
- `StateManager` usa `emscripten_set_main_loop` (requestAnimationFrame) no navegador, no lugar do `while` + `SDL_Delay` do desktop — um loop bloqueante travaria a thread do navegador. O `dt`/`MAX_DT` e a lógica de estados são os mesmos.
- Imagens, sons, fontes e o mapa entram no `index.data` via `--preload-file`; não há dependência de arquivos externos.
- Para diagnosticar, `index.html?selftest=1` publica no título da página `LOKURAS_SELFTEST_PASS frames=N` depois de ~30 frames desenhados. Esse é o gancho usado pelo smoke test `tools/smoke-web.mjs` (também rodado pelo CI): com o `bin/web` servido por http, ele abre a página em um navegador headless (Edge/Chrome) via DevTools, espera o selftest, aperta ENTER, confere que o jogo continua desenhando quadros e que o `/save` (IDBFS) montou — ex.: `SMOKE_SCREENSHOT=bin/web-smoke.png node tools/smoke-web.mjs`.

#### Publicação no GitHub Pages

O job `deploy-web` do CI publica o `bin/web` no **GitHub Pages** a cada push em `master`, mas só depois que o build e o smoke test passam — o artefato publicado é exatamente o que foi testado, e um jogo que não desenha quadros não vai para o ar. O endereço é **<https://victorgmbraga.github.io/lokuras-no-espaco/>**.

Na primeira vez, habilite o Pages em **Settings → Pages → Build and deployment → Source: GitHub Actions** (o `GITHUB_TOKEN` do workflow não tem permissão para habilitar o Pages sozinho, então esse passo é manual e único). Depois disso, cada push em `master` atualiza o site; para republicar sem mudar código, use **Run workflow** na aba Actions. Como o HTML, o JS, o WASM e o `.data` usam caminhos relativos, o mesmo build funciona tanto em `http://localhost:8000/` quanto na subpasta `/lokuras-no-espaco/` do Pages.

### Zed

O repositório tem **tasks prontas do Zed** em `.zed/tasks.json` (use `task: spawn` na paleta de comandos — `Ctrl+Shift+P`):

| Task | O que faz |
| --- | --- |
| **Compilar** | Compila o jogo (`build.sh` no Linux/macOS, `build.bat` no Windows) |
| **Executar** | Executa o jogo (compila antes, se `bin/game` ainda não existir) |
| **Compilar e executar** | Compila e executa em sequência |
| **Compilar (web)** | Compila para o navegador (`build-web.sh`/`build-web.bat`, Emscripten) |
| **Executar (web)** | Serve `bin/web` por http e abre o navegador |

A detecção do sistema operacional fica nos wrappers `.zed/*.cmd`: são arquivos "poliglotas" que rodam como script `sh` no Linux/macOS e como `.cmd` no Windows, então a mesma task funciona nos dois sistemas. (O `tasks.json` do Zed ainda não tem uma variável de sistema operacional nem condicionais por plataforma como o `windows`/`linux` do VS Code.)

Para criar atalhos, use `task::Spawn` com `task_name` no seu `keymap.json` global, por exemplo:

```json
{ "context": "Workspace", "bindings": { "alt-b": ["task::Spawn", { "task_name": "Compilar" }] } }
```

---

## Histórico de desenvolvimento

| Versão | Data | Descrição | Resumo |
| --- | --- | --- | --- |
| **TR01** | 16/04/2013 | Game Loop e Sprite | Game loop, SDLBase, Sprite e GameManager exibindo o fundo |
| **EX01** | 20/04/2013 | Arrays | Classe `Planet`, vetor de planetas, clique com dano e HP |
| **EX02** | 13/05/2013 | TileMap | `TileSet`/`TileMap` (mapa montado em código) e rolagem com as setas |
| **TR02** | 28/05/2013 | Movimento, Herança e Input | `InputManager`, `GameObject`/herança, `Earth`/`Moon`, TileMap de arquivo com parallax |
| **EX03** | 07/06/2013 | Filas de Comandos | `FollowerObject` (UFO) com fila de posições e `drawLine`; rotozoom no `Sprite` |
| **TR03** | 09/06/2013 | Animação, Movimento Acelerado, Rotação, Zoom e Colisão | Nave Hyperion, animações, colisões, explosão e câmera vinculada |
| **TR04** | 14/06/2013 | Texto, Áudio, Timer, StateManager e States | Máquina de estados, HUD, texto, áudio e cooldown |

### Changelog por versão

**TR01 — Game Loop e Sprite**

- `SDLBase`: janela 800×600 em software, inicialização da SDL, `loadImage` (otimizando a surface com alpha) e `atualizarTela`.
- `Sprite`: surface, retângulo de clipping e renderização.
- `GameManager`: loop `while(!SDL_QuitRequested())` desenhando `fundo.png`; `main` apenas constrói e roda o gerenciador.

**EX01 — Arrays**

- Nova classe `Planet` (sprite única compartilhada, posição e hit points).
- Vetor `planetArray` no GameManager: planetas criados em posição aleatória, dano por clique (10–19) e remoção quando o HP chega a zero.
- Fundo movido para `img/` junto com o novo sprite do planeta.

**EX02 — TileMap**

- `TileSet` com 30 tiles individuais (`img/tileset/SpaceTile-XX.png`) e `TileMap` de uma camada 16×15 montado em código (`loadTiles`/`buildMap`).
- Rolagem do cenário com as setas (`cameraX`/`cameraY`) e clique compensando o offset da câmera.
- Refatoração do tratamento de eventos (flag `quit`).

**TR02 — Movimento, Herança e Input**

- `InputManager` singleton (teclas pressionadas/soltas, mouse, `quitGame`) e `GameObject` abstrato como pai de todos os objetos.
- Herança: `Planet` (base abstrata) → `PlanetRed`, `Earth` (movida com WASD) e `Moon` (órbita a Terra com seno/cosseno).
- `TileMap`/`TileSet` carregados de arquivo (`map/tileMap.txt`, `img/Tileset.png`), com **parallax por camada** (extra do trabalho).
- Loop com **dt** (`SDL_GetTicks`) e delay fixo por frame.

**EX03 — Filas de Comandos**

- `FollowerObject`: objeto que segue uma **fila de posições** (comandos do mouse), com movimento proporcional em cada eixo e desenho das linhas do caminho (`renderQueueLines` + `SDLBase::drawLine`).
- `Sprite` ganha `rotoZoom`/`restore` (via SDL_gfx) e `SDLBase` ganha `clip` e `rotoZoom`; novos tipos `Point` e `Rect`.
- `Animation` é criada nesta etapa (passa a ser usada no TR03).
- UFO instanciado no GameManager: clique esquerdo continua destruindo planetas, clique direito enfileira comandos para o UFO.

**TR03 — Animação, Movimento Acelerado, Rotação, Zoom e Colisão**

- `AccObject`: a nave Hyperion com **movimento acelerado** (`speed` + `acceleration`), rotação (A/D), animação de boost e `stop()`; HP 20.
- `Animation` completa sobre sprite sheets: `NaveSheet.png` (4 frames) e `BoomSheet.png` (8 frames, explosão) com controle de `frameTime`.
- **Colisão AABB**: `GameObject` ganha `w`/`h` e `collidesWith` (via `Rect`); `Planet` passa a usar as dimensões do sprite.
- Lógica de vitória/derrota em `checkCollision`: nave × planeta (−1 HP + explosão), nave × UFO (derrota) e nave × Terra (vitória).
- `Camera.bind/unbind`: segue a nave e fica livre (setas) após o fim do jogo (extra do trabalho).
- Zoom 3× na Terra e no UFO; criação de planetas passa para o clique; build passa a linkar SDL_gfx.

**TR04 — Texto, Áudio, Timer, StateManager e States**

- **Game engine final**: interface `State` (`load`/`unload`/`update`/`render`) e `StateManager` com pilha de troca de estados e game loop central.
- `StateMenu`, `StateGame` (antigo GameManager refatorado) e `StateWinLose` (vitória/derrota via valor de retorno do `unload`), cada um com fundo, texto e música.
- `Text` (SDL_ttf: Pirata One e Space Age), `Audio` (SDL_mixer: 4 músicas de estado + `boom.wav`), `Timer` (cooldown de 3 s para criar planetas) e `HUD` (extra: classe própria com barra de vida verde/vermelha e comandos na tela).
- Ajustes de gameplay: vida 100, dano de 5 por planeta, som de explosão nas colisões e `AccObject` com **atrito** e `MAX_SPEED` (extra de TR3 consolidado aqui).
- `StateManager` inicializa SDL_ttf/SDL_mixer via `SDLBase::loadSDL`/`unloadSDL`; `main` passa a rodar o `StateManager`.

## Arquitetura

### Núcleo (engine)

| Classe | Papel |
| --- | --- |
| `SDLBase` | Encapsula a SDL: janela 1280×720, alternância para tela cheia (`Alt+Enter`), carregamento/otimização de imagens, blit, flip, `rotoZoom`, `clip`, `drawLine`, `drawCircle`, `drawRect`, overlay de fade e init/shutdown de TTF e Mixer |
| `Sprite` | Surface com clipping; rotação/zoom (`rotoZoom`/`restore`) e escala assada na surface (`scale`) |
| `Animation` | Sprite sheet animada por tempo real; suporta one-shot (`setOneShot`/`restart`/`isFinished`) |
| `Text` | Texto renderizado com TTF (fonte, tamanho, estilo e cor configuráveis) |
| `Audio` | Música (`Mix_Music`) e efeitos (`Mix_Chunk`); controles globais de volume/mudo com legenda no título da janela |
| `Timer` | Cronômetro com start/pause/resume para cooldowns |
| `InputManager` | Singleton de entrada: teclado/mouse (down/up/pressed) e `quitGame`, com suporte a joystick (analog sticks) |
| `Camera` | Singleton; segue um `GameObject` via `bind`/`unBind` (livre com as setas, em tempo real) |
| `State` / `StateManager` | Máquina de estados com pilha de valores entre telas; loop central com dt real, cap de ~60 FPS, fade entre estados e atalhos globais de volume (no navegador o loop vira um callback do `requestAnimationFrame`) |
| `TileSet` / `TileMap` | Mapa em camadas carregado de arquivo, com parallax por camada e culling por tela |
| `PlayStats` | Estatísticas/recorde de partidas persistidas (`lokuras.rec`) |
| `Rect` / AABB | Retângulo de colisão do `GameObject` |

### Hierarquia do jogo

```
Sprite ──► Animation

GameObject (box AABB)
 ├── Planet (abstrata)
 │    ├── PlanetRed   (planetas vermelhos e roxos destrutíveis, quicam nos tiles)
 │    ├── Earth        (objetivo da nave)
 │    ├── Moon         (orbita a Terra)
 │    └── PowerUp      (cristais de vida/escudo/turbo/combustível)
 ├── Asteroid          (rocha pequena atravessável: dano leve e deriva própria)
 ├── FollowerObject    (UFO: chefe perseguidor autônomo, com fases e ondas)
 ├── Turret            (torretas fixas nos painéis, com linha de visada)
 └── AccObject         (nave Hyperion: aceleração, rotação, atrito, animação)

State
 ├── StateMenu
 ├── StateGame
 └── StateWinLose
```

Padrões e técnicas aplicadas: **game loop com dt**, **singleton** (InputManager/Camera), **máquina de estados** com transições por valor de retorno, **herança/polimorfismo**, composição, colisão **AABB**, paralaxe por camadas e animação por sprite sheet.

---

## Estrutura do repositório

```
.
├── src/                     # código-fonte (engine + jogo)
│   ├── main.cpp             # entry point (StateManager)
│   ├── SDLBase.* Sprite.* Animation.* Text.* Audio.* Timer.*
│   ├── InputManager.* Camera.* State.* StateManager.* StateMenu.* StateGame.* StateWinLose.*
│   ├── TileSet.* TileMap.* Rect.h Point.*
│   ├── GameObject.* Planet.* PlanetRed.* Earth.* Moon.* FollowerObject.* Turret.* AccObject.* HUD.*
│   ├── PowerUp.h            # cristais de power-up (header-only)
│   ├── GameSettings.h       # dificuldade e semente da sessão (header-only)
│   └── PlayStats.h          # recordes por dificuldade, contadores e conquistas (header-only)
├── lib/                     # SDL 1.2: headers (include/), libs de importação (lib/) e licenças (LICENSES/)
├── bin/                     # DLLs de runtime do Windows + binários compilados (game.exe/game, ignorados)
├── img/                     # sprites (nave, planetas, asteroides, UFO, torreta, power-ups), telas e Tileset.png
├── map/tileMap.txt          # mapa de referência (o mapa da partida é gerado em código)
├── sfx/                     # músicas (ogg) e efeitos (boom/pickup/engine .wav)
├── ttf/                     # PirataOne e SpaceAge
├── build.bat                # compilação no Windows (MinGW + SDL 1.2 de lib/) -> bin/game.exe
├── run.bat                  # executa o jogo no Windows (compila antes, se necessário)
├── build.sh                 # compilação no Linux (SDL 1.2 do sistema) -> bin/game
├── build-macos.sh           # compilação no macOS (sdl12-compat + Homebrew) -> bin/game/.app
├── run.sh                   # executa o jogo no Linux/macOS (compila antes, se necessário)
├── build-web.bat            # compilação para o navegador (Emscripten) -> bin/web/
├── run-web.bat              # serve bin/web por http e abre o navegador (Windows)
├── build-web.sh             # compilação para o navegador (Emscripten) -> bin/web/
├── run-web.sh               # serve bin/web por http e abre o navegador (Linux/macOS)
├── web/
│   ├── compat/              # API SDL 1.2 sobre a SDL 2 dos ports do Emscripten
│   ├── pre.js               # monta/sincroniza /save (IDBFS) no runtime
│   └── shell.html           # página do jogo (canvas, áudio, autoteste)
├── .zed/                    # tasks do Zed (Compilar/Executar, desktop e web) + wrappers sh/cmd
├── .github/workflows/       # CI: builds Linux/Windows/macOS (+ artefatos) e web (+ GitHub Pages)
└── tools/
    ├── make-assets.py       # regenera TODOS os assets (img/, sfx/) do zero
    ├── setup-mingw.ps1      # auto-bootstrap do MinGW no Windows (download com SHA-256)
    ├── mingw32/             # toolchain baixado pelo bootstrap (ignorado pelo git)
    ├── setup-sdl12.sh       # SDL 1.2 local no Linux, sem root (extrai .deb em sdl12/)
    ├── sdl12/               # SDL 1.2 baixado pelo script acima (ignorado pelo git)
    ├── setup-sdl12-macos.sh # SDL 1.2 local no macOS: compila as cinco libs em sdl12-macos/
    ├── sdl12-macos/         # SDL 1.2 compilado pelo script acima (ignorado pelo git)
    ├── package-macos-app.sh # empacota bin/game em dist/Lokuras no Espaço.app
    ├── setup-emsdk.ps1      # auto-bootstrap do Emscripten no Windows
    ├── setup-emsdk.sh       # auto-bootstrap do Emscripten no Linux/macOS
    ├── smoke-web.mjs        # smoke test do build web (navegador headless via DevTools)
    └── emsdk/               # Emscripten baixado pelos scripts acima (ignorado pelo git)
```

## Licença

- **Código-fonte** (`src/`, `tools/` e scripts de build): licença **MIT** — texto completo em [`LICENSE`](LICENSE).
- **Assets do jogo** (`img/`, `sfx/`, `map/`): **CC0 / domínio público** — gerados pelo próprio projeto ou obtidos sob CC0; atribuições em [CREDITS.md](CREDITS.md).
- **Fontes**: *Pirata One* (SIL OFL 1.1 — texto em `lib/LICENSES/OFL-PirataOne.txt`) e *Space Age* (distribuição livre pelo autor).
- **Bibliotecas de terceiros** (`lib/` e `bin/`): SDL 1.2 e dependências (LGPL/zlib e afins) — avisos e textos em [`lib/LICENSES/NOTICE.md`](lib/LICENSES/NOTICE.md).
- `img/PlanetaTerra.png`: foto do acervo da NASA (uso livre, sem royalties).

## Créditos

- Desenvolvido por **Victor Braga** (matrícula 10/0126481), 2013.
- Disciplina **Introdução ao Desenvolvimento de Jogos** — Profª. Carla Denise Castanho, UnB/CIC (Turma A, 2013).
- Bibliotecas: [SDL 1.2](https://www.libsdl.org/) e SDL_image/SDL_gfx/SDL_ttf/SDL_mixer — versionadas em `lib/`, com as licenças de terceiros em [`lib/LICENSES/`](lib/LICENSES/NOTICE.md).
- Assets (arte, música, efeitos e fontes): próprios, CC0 ou livremente redistribuíveis — detalhes e atribuições em [CREDITS.md](CREDITS.md).

*Projeto de origem acadêmica (UnB/IDJ, 2013), publicado como código aberto.*
