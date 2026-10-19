/*
 * SDL.h
 *
 *  Created on: 19/10/2026
 *      Author: Victor
 *
 *  Camada de compatibilidade SDL 1.2 para o build WebAssembly (Emscripten).
 *
 *  O jogo foi escrito contra a API da SDL 1.2 (SetVideoMode/Flip/surface de
 *  software). O Emscripten nao oferece mais o port da SDL 1.2, apenas o da
 *  SDL 2, entao este header (usado com -Iweb/compat) reexpõe a parte da API
 *  1.2 que o jogo usa sobre a SDL 2 + ports (SDL_image, SDL_ttf, SDL_mixer e
 *  SDL2_gfx). O codigo do jogo nao muda: no Windows/Linux ele continua
 *  compilando contra a SDL 1.2 de verdade (lib/ e do sistema).
 *
 *  Implementacao: web/compat/sdl12_web.cpp.
 */

#ifndef SDL12_WEB_H_
#define SDL12_WEB_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/* --- Constantes da SDL 1.2 que a SDL 2 nao tem (ou tem com outro valor) --- */

/* Flags de surface/video. Na SDL 2 elas nao existem mais; os valores da 1.2
 * sao restaurados porque o codigo do jogo testa alguns desses bits. */
#undef SDL_SWSURFACE
#define SDL_SWSURFACE  0x00000000
#undef SDL_HWSURFACE
#define SDL_HWSURFACE  0x00000001
#undef SDL_ASYNCBLIT
#define SDL_ASYNCBLIT  0x00000004
#undef SDL_ANYFORMAT
#define SDL_ANYFORMAT  0x10000000
#undef SDL_HWPALETTE
#define SDL_HWPALETTE  0x20000000
#undef SDL_DOUBLEBUF
#define SDL_DOUBLEBUF  0x40000000
#undef SDL_FULLSCREEN
#define SDL_FULLSCREEN 0x80000000
#undef SDL_RESIZABLE
#define SDL_RESIZABLE  0x00000010
#undef SDL_NOFRAME
#define SDL_NOFRAME    0x00000020
#undef SDL_SRCCOLORKEY
#define SDL_SRCCOLORKEY 0x00001000
#undef SDL_RLEACCEL
#define SDL_RLEACCEL   0x00004000
#undef SDL_SRCALPHA
#define SDL_SRCALPHA   0x00010000

/* Codigos de tecla da SDL 1.2 (iguais em Windows/Linux). A SDL 2 usa valores
 * diferentes para tudo que nao e ASCII; aqui os valores da 1.2 voltam para que
 * InputManager (isKeyDown(SDLK_*) / SDL_GetKeyState) continue funcionando com
 * os mesmos indices. */
#undef SDLK_CAPSLOCK
#define SDLK_CAPSLOCK   301
#undef SDLK_PAUSE
#define SDLK_PAUSE      19
#undef SDLK_KP0
#define SDLK_KP0        256
#undef SDLK_KP1
#define SDLK_KP1        257
#undef SDLK_KP2
#define SDLK_KP2        258
#undef SDLK_KP3
#define SDLK_KP3        259
#undef SDLK_KP4
#define SDLK_KP4        260
#undef SDLK_KP5
#define SDLK_KP5        261
#undef SDLK_KP6
#define SDLK_KP6        262
#undef SDLK_KP7
#define SDLK_KP7        263
#undef SDLK_KP8
#define SDLK_KP8        264
#undef SDLK_KP9
#define SDLK_KP9        265
#undef SDLK_KP_PERIOD
#define SDLK_KP_PERIOD  266
#undef SDLK_KP_DIVIDE
#define SDLK_KP_DIVIDE  267
#undef SDLK_KP_MULTIPLY
#define SDLK_KP_MULTIPLY 268
#undef SDLK_KP_MINUS
#define SDLK_KP_MINUS   269
#undef SDLK_KP_PLUS
#define SDLK_KP_PLUS    270
#undef SDLK_KP_ENTER
#define SDLK_KP_ENTER   271
#undef SDLK_KP_EQUALS
#define SDLK_KP_EQUALS  272
#undef SDLK_UP
#define SDLK_UP         273
#undef SDLK_DOWN
#define SDLK_DOWN       274
#undef SDLK_RIGHT
#define SDLK_RIGHT      275
#undef SDLK_LEFT
#define SDLK_LEFT       276
#undef SDLK_INSERT
#define SDLK_INSERT     277
#undef SDLK_HOME
#define SDLK_HOME       278
#undef SDLK_END
#define SDLK_END        279
#undef SDLK_PAGEUP
#define SDLK_PAGEUP     280
#undef SDLK_PAGEDOWN
#define SDLK_PAGEDOWN   281
#undef SDLK_F1
#define SDLK_F1         282
#undef SDLK_F2
#define SDLK_F2         283
#undef SDLK_F3
#define SDLK_F3         284
#undef SDLK_F4
#define SDLK_F4         285
#undef SDLK_F5
#define SDLK_F5         286
#undef SDLK_F6
#define SDLK_F6         287
#undef SDLK_F7
#define SDLK_F7         288
#undef SDLK_F8
#define SDLK_F8         289
#undef SDLK_F9
#define SDLK_F9         290
#undef SDLK_F10
#define SDLK_F10        291
#undef SDLK_F11
#define SDLK_F11        292
#undef SDLK_F12
#define SDLK_F12        293
#undef SDLK_NUMLOCK
#define SDLK_NUMLOCK    300
#undef SDLK_SCROLLOCK
#define SDLK_SCROLLOCK  302
#undef SDLK_RSHIFT
#define SDLK_RSHIFT     303
#undef SDLK_LSHIFT
#define SDLK_LSHIFT     304
#undef SDLK_RCTRL
#define SDLK_RCTRL      305
#undef SDLK_LCTRL
#define SDLK_LCTRL      306
#undef SDLK_RALT
#define SDLK_RALT       307
#undef SDLK_LALT
#define SDLK_LALT       308
#undef SDLK_RMETA
#define SDLK_RMETA      309
#undef SDLK_LMETA
#define SDLK_LMETA      310
#undef SDLK_PRINT
#define SDLK_PRINT      316

/* --- API 1.2 implementada em web/compat/sdl12_web.cpp -------------------- */

/* SDL_SetVideoMode: cria a janela/canvas na primeira chamada (1280x720, 32
 * bits) e devolve a surface de software onde o jogo desenha - o equivalente
 * moderno e uma textura de streaming atualizada em SDL_Flip. */
extern SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
extern int SDL_Flip(SDL_Surface *screen);

extern void SDL_WM_SetCaption(const char *title, const char *icon);
extern int SDL_WM_ToggleFullScreen(SDL_Surface *screen);

/* Conversao para o formato da tela (a SDL 2 tem SDL_ConvertSurface). */
extern SDL_Surface *SDL_DisplayFormat(SDL_Surface *surface);
extern SDL_Surface *SDL_DisplayFormatAlpha(SDL_Surface *surface);

/* Alpha por surface da SDL 1.2 (SDL 2: blend mode + alpha mod). */
extern int SDL_SetAlpha(SDL_Surface *surface, Uint32 flags, Uint8 alpha);

/* Estado do teclado indexado pelos SDLK_* da SDL 1.2 (ate 512 teclas). */
extern Uint8 *SDL_GetKeyState(int *numkeys);

/* SDL_CreateRGBSurface da 1.2 recebe as flags (SDL_SWSURFACE/SDL_SRCALPHA)
 * como primeiro argumento; a implementacao descarta esses bits ao chamar a
 * SDL 2 (que nao os conhece). */
extern SDL_Surface *lokurasSDL12CreateRGBSurface(Uint32 flags, int width, int height,
                                                 int depth, Uint32 Rmask, Uint32 Gmask,
                                                 Uint32 Bmask, Uint32 Amask);
#define SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask) \
	lokurasSDL12CreateRGBSurface((flags), (width), (height), (depth), (rmask), (gmask), (bmask), (amask))

/* SDL_PollEvent traduz os eventos da SDL 2 para o que o jogo espera da 1.2
 * (keysym -> SDLK_* da 1.2, roda do mouse -> botoes 4/5) e cuida dos
 * gamepads que aparecem depois do inicio (JOYDEVICEADDED). */
extern int lokurasSDL12PollEvent(SDL_Event *event);
#define SDL_PollEvent(event) lokurasSDL12PollEvent(event)

#endif /* SDL12_WEB_H_ */
