/*
 * SDL_rotozoom.h
 *
 *  Created on: 19/10/2026
 *      Author: Victor
 *
 *  No build WebAssembly a rotacao/escala das surfaces vem do SDL2_gfx (port
 *  do Emscripten), que expoe o mesmo rotozoomSurfaceXY da SDL_gfx 1.2 usada
 *  no Windows/Linux.
 */

#ifndef SDL12_WEB_SDL_ROTOZOOM_H_
#define SDL12_WEB_SDL_ROTOZOOM_H_

#include "SDL.h"
#include <SDL2/SDL2_rotozoom.h>

/* A SDL2_gfx devolve a surface girada sem o blend mode da origem (na SDL 1.2
 * o SRCALPHA valia para o resultado tambem), entao o rotozoom passa pela
 * implementacao de web/compat/sdl12_web.cpp. La a macro e desfeita e a funcao
 * real da SDL2_gfx e chamada diretamente. */
extern SDL_Surface *lokurasSDL12RotozoomSurfaceXY(SDL_Surface *src, double angle,
                                                  double zoomx, double zoomy, int smooth);
#define rotozoomSurfaceXY lokurasSDL12RotozoomSurfaceXY

#endif /* SDL12_WEB_SDL_ROTOZOOM_H_ */
