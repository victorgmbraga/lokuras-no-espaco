/*
 * SDLBase.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef SDLBASE_H_
#define SDLBASE_H_

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

class SDLBase
{
	private:
		static SDL_Surface *screen;

	public:
		static void inicializaSDL();
		static SDL_Surface* getScreen();
		static SDL_Surface* loadImage(string arquivo);
		static void renderSurface(SDL_Surface* surface, SDL_Rect* clip, SDL_Rect* dst);
		static void atualizarTela();
};

#endif /* SDLBASE_H_ */
