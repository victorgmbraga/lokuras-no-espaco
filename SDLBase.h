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
#include <SDL/SDL_rotozoom.h>
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
		static void drawLine(int x1, int y1, int x2, int y2, int rgb, int spacing);
		static SDL_Surface* clip(SDL_Surface* original, SDL_Rect* clip);
		static SDL_Surface* rotoZoom(SDL_Surface* surface, float angle, float scalex = 1.0f, float scaley = 1.0f);
};

#endif /* SDLBASE_H_ */
