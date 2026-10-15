# Lokuras no Espaço — Projeto IDJ

Jogo 2D em **C++ com SDL 1.2**, desenvolvido para a disciplina **Introdução ao Desenvolvimento de Jogos (IDJ)** da Universidade de Brasília (UnB/CIC, Turma A, 2013 — Profª. Carla Denise Castanho).

Este repositório reconstrói todo o **fluxo de desenvolvimento da disciplina** em um histórico Git linear: cada entrega (trabalho **TR** ou exercício **EX**) virou um commit, partindo do game loop básico (TR01) até a game engine completa com máquina de estados (TR04), que dá origem ao jogo **Lokuras no Espaço**.

> Projeto para Windows (SDL 1.2 / MinGW 32 bits). Cada commit do histórico contém o PDF oficial da entrega correspondente.

---

## O jogo

Você pilota a nave **Hyperion**, perdida no espaço e caçada por alienígenas. Sua única esperança é voltar para a **Terra** — mas o caminho é cheio de planetas vermelhos e um UFO teimosamente em seu encalço.

- **Objetivo:** alcançar a Terra sem ser capturado pelo UFO.
- **Planetas vermelhos** aparecem pela tela (spawn controlado por cooldown); colidir custa **5 de vida** (a nave começa com **100**, mostrada em uma barra no HUD).
- **UFO:** segue uma **fila de comandos de posição** (com as "queue lines" desenhadas no caminho). Se ele alcançar a nave, é derrota na hora.
- **Explosão:** ao colidir com planetas ou UFO, uma animação de explosão (BoomSheet) e som de efeito são disparados.
- **Câmera** centralizada na nave.

### Controles

| Entrada | Ação |
| --- | --- |
| `ENTER` | Iniciar o jogo (menu) / jogar de novo (vitória ou derrota) |
| `W` / `S` | Acelerar / frear a nave (movimento acelerado com atrito) |
| `A` / `D` | Rotacionar a nave |
| `Mouse 1` (esquerdo) | Criar um planeta vermelho em posição aleatória |
| `Mouse 3` (direito) | Enfileirar uma posição para o UFO se mover até lá |
| Fechar a janela | Sair do jogo |

### Estados

`StateMenu` (título, créditos e música) → `StateGame` (jogabilidade) → `StateWinLose` (tela de vitória "PARABÉNS AE MANOLO" ou derrota "VC MORREU, RSRSRSRS", com música própria). Cada estado tem música tema; o estado de jogo tem `stateGame.mp3` e o SFX `boom.wav`.

---

## Como compilar e executar

**Requisitos**

- Windows;
- MinGW-w64 **i686** (msvcrt, posix) com `g++`;
- SDL 1.2 + SDL_image + SDL_gfx + SDL_ttf + SDL_mixer (headers e libs).

O script `build.bat` compila as fontes de `src/` e gera `bin\game.exe`. Edite as variáveis `MINGW` e `SDL` no topo do script apontando para a sua instalação:

```bat
set MINGW=<caminho>\mingw32\bin
set SDL=<caminho>\sdl12
```

> O PDF `SDL-Tutorial_Windows.pdf` (no repositório) cobre a instalação do SDL 1.2 no Windows.

A linha de compilação usada é, resumidamente:

```bat
g++ -std=gnu++11 -O2 -w -Dmain=SDL_main -I"%SDL%\include" -I"%SDL%\include\SDL" -I"src" ^
  src\AccObject.cpp src\Animation.cpp src\Audio.cpp src\Camera.cpp src\Earth.cpp ^
  src\FollowerObject.cpp src\GameObject.cpp src\HUD.cpp src\InputManager.cpp src\main.cpp ^
  src\Moon.cpp src\Planet.cpp src\PlanetRed.cpp src\Point.cpp src\SDLBase.cpp ^
  src\Sprite.cpp src\StateGame.cpp src\StateManager.cpp src\StateMenu.cpp ^
  src\StateWinLose.cpp src\Text.cpp src\TileMap.cpp src\TileSet.cpp src\Timer.cpp ^
  -o bin\game.exe ^
  -L"%SDL%\lib" -lmingw32 -lSDLmain -lSDL_gfx -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL ^
  -static-libgcc -static-libstdc++
```

As DLLs de runtime necessárias estão versionadas em `bin/` (SDL.dll, SDL_image.dll, SDL_ttf.dll, SDL_mixer.dll e dependências como `libpng15-15`, `zlib1`, `libjpeg-8`, `libfreetype-6`, entre outras), junto do executável. Depois de compilar, execute `run.bat` — ou `bin\game.exe` a partir da raiz do projeto, pois o jogo carrega os assets por caminhos relativos.

---

## Histórico de desenvolvimento (versões)

Cada commit do `master` é uma entrega da disciplina, em ordem cronológica:

```
TR01 (16/04/2013) → EX01 (20/04) → EX02 (13/05) → TR02 (28/05)
    → EX03 (07/06) → TR03 (09/06) → TR04 (14/06)
```

| Versão | Data | Entrega (PDF) | Resumo |
| --- | --- | --- | --- |
| **TR01** | 16/04/2013 | _Trabalho 1.pdf — Game Loop e Sprite | Game loop, SDLBase, Sprite e GameManager exibindo o fundo |
| **EX01** | 20/04/2013 | _Exercicio1.pdf — Arrays | Classe `Planet`, vetor de planetas, clique com dano e HP |
| **EX02** | 13/05/2013 | _Exercicio2.pdf — TileMap | `TileSet`/`TileMap` (mapa montado em código) e rolagem com as setas |
| **TR02** | 28/05/2013 | _Trabalho2.pdf — Movimento, Herança e Input | `InputManager`, `GameObject`/herança, `Earth`/`Moon`, TileMap de arquivo com parallax |
| **EX03** | 07/06/2013 | _Exercicio3.pdf — Filas de Comandos | `FollowerObject` (UFO) com fila de posições e `drawLine`; rotozoom no `Sprite` |
| **TR03** | 09/06/2013 | _Trabalho3.pdf — Animação, Movimento Acelerado, Rotação, Zoom e Colisão | Nave Hyperion, animações, colisões, explosão e câmera vinculada |
| **TR04** | 14/06/2013 | _Trabalho4.pdf — Texto, Áudio, Timer, StateManager e States | Máquina de estados, HUD, texto, áudio e cooldown |

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

---

## Arquitetura

### Núcleo (engine)

| Classe | Papel |
| --- | --- |
| `SDLBase` | Encapsula a SDL: tela 800×600, carregamento/otimização de imagens, blit, flip, `rotoZoom`, `clip`, `drawLine`, `drawRect` e init/shutdown de TTF e Mixer |
| `Sprite` | Surface com clipping; suporta rotação/zoom (`rotoZoom`/`restore`) |
| `Animation` | Sprite sheet animada por `frameTime` (herda de `Sprite`) |
| `Text` | Texto renderizado com TTF (fonte, tamanho, estilo e cor configuráveis) |
| `Audio` | Música (`Mix_Music`) e efeitos (`Mix_Chunk`) |
| `Timer` | Cronômetro com start/pause/resume para cooldowns |
| `InputManager` | Singleton de entrada: teclado/mouse (down/up/pressed) e `quitGame` |
| `Camera` | Singleton; segue um `GameObject` via `bind`/`unBind` |
| `State` / `StateManager` | Máquina de estados com pilha de valores entre telas |
| `TileSet` / `TileMap` | Mapa em camadas carregado de arquivo, com parallax por camada |
| `Rect` / AABB | Retângulo de colisão do `GameObject` |

### Hierarquia do jogo

```
Sprite ──► Animation

GameObject (box AABB)
 ├── Planet (abstrata)
 │    ├── PlanetRed   (planetas vermelhos destrutíveis)
 │    ├── Earth        (objetivo da nave)
 │    └── Moon         (orbita a Terra)
 ├── FollowerObject    (UFO: fila de comandos + queue lines)
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
│   └── GameObject.* Planet.* PlanetRed.* Earth.* Moon.* FollowerObject.* AccObject.* HUD.*
├── bin/                     # DLLs de runtime versionadas + game.exe (compilado, ignorado)
├── img/                     # sprites, telas de menu/vitória/derrota e tileset
├── map/tileMap.txt          # mapa em camadas
├── sfx/                     # músicas (mp3) e boom.wav
├── ttf/                     # Pirata One e Space Age
├── build.bat                # compilação (MinGW + SDL 1.2) -> bin/game.exe
├── run.bat                  # executa o jogo a partir da raiz
├── README.txt               # notas da entrega original do Trabalho 4
├── _Trabalho4.pdf           # especificação oficial desta versão
└── SDL-Tutorial_Windows.pdf # tutorial de setup do SDL 1.2 no Windows
```

As versões anteriores (e os PDFs `_Exercicio1.pdf`, `_Trabalho1.pdf` etc.) estão preservadas nos commits do histórico — use `git log --oneline` e `git show <commit>` para navegar entre elas.

---

## Créditos

- Desenvolvido por **Victor Braga** (matrícula 10/0126481), 2013.
- Disciplina **Introdução ao Desenvolvimento de Jogos** — Profª. Carla Denise Castanho, UnB/CIC (Turma A, 2013); especificações (PDFs) de autoria da equipe da disciplina.
- Bibliotecas: [SDL 1.2](https://www.libsdl.org/) e SDL_image/SDL_gfx/SDL_ttf/SDL_mixer.
- Sprites da nave: *Spaceship Sprite Package by Kryptid*; tiles espaciais *SpaceTile*; demais imagens e músicas utilizadas nas aulas da disciplina.

Projeto acadêmico, sem fins comerciais.
