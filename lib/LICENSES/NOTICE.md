# Avisos de terceiros (third-party notices)

Este repositório redistribui bibliotecas de terceiros para que o build e a execução
funcionem sem instalações externas (além do compilador MinGW). Os avisos de copyright
originais foram preservados nos arquivos, sem alteração.

## Build — `lib/`

| Componente | Versão | Arquivos | Licença | Texto |
| --- | --- | --- | --- | --- |
| SDL | 1.2.15 | `include/SDL/*` (núcleo), `lib/libSDL.dll.a`, `lib/libSDLmain.a` | LGPL 2.1+ | [LGPL-2.1.txt](LGPL-2.1.txt) |
| SDL_image | 1.2.11 | `include/SDL/SDL_image.h`, `lib/libSDL_image.a` | zlib | [SDL_image.txt](SDL_image.txt) |
| SDL_ttf | 2.0.11 | `include/SDL/SDL_ttf.h`, `lib/libSDL_ttf.a` | zlib | [SDL_ttf.txt](SDL_ttf.txt) |
| SDL_mixer | 1.2.12 | `include/SDL/SDL_mixer.h`, `lib/libSDL_mixer.a` | zlib | [SDL_mixer.txt](SDL_mixer.txt) |
| SDL_gfx | — | `include/SDL/SDL_rotozoom.h`, `lib/libSDL_gfx.a` | zlib | [SDL_gfx.txt](SDL_gfx.txt) |

## Runtime — `bin/`

| Componente | Licença |
| --- | --- |
| SDL.dll (SDL 1.2) | LGPL 2.1+ |
| SDL_image.dll | zlib |
| SDL_ttf.dll | zlib |
| SDL_mixer.dll | zlib |
| smpeg.dll (SMPEG) | LGPL 2 |
| libpng15-15.dll | zlib |
| zlib1.dll | zlib |
| libjpeg-8.dll | IJG (BSD-like) |
| libtiff-5.dll | libtiff (BSD-like) |
| libwebp-2.dll | BSD |
| libfreetype-6.dll | FreeType License (FTL) |
| libogg-0.dll, libvorbis-0.dll, libvorbisfile-3.dll | BSD |
| libFLAC-8.dll | BSD |
| libmikmod-2.dll | LGPL 2.1+ |
| libwinpthread-1.dll (mingw-w64) | permissiva (mingw-w64 runtime) |
| libstdc++-6.dll, libgcc_s_dw2-1.dll | GPL 3 com GCC Runtime Library Exception |

Observações de conformidade:

- O jogo usa o SDL de forma **dinâmica** (DLLs em `bin/`); o material estático do SDL
  que entra no executável se resume ao shim `libSDLmain.a`. Como todo o código-fonte
  do jogo e o script de build são públicos neste repositório, qualquer pessoa consegue
  religar o binário com versões modificadas das bibliotecas (requisito do LGPL).
- Os textos integrais aplicáveis aos arquivos de build estão nesta pasta; para as DLLs
  de runtime, os textos podem ser obtidos nos sites oficiais de cada projeto.
