/*
 * SDLBase.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef SDLBASE_H_
#define SDLBASE_H_

#define SCREEN_W 800
#define SCREEN_H 600
#define GAME_NAME "Trabalho 4 10/0126481"

#include <iostream>
#include <stdlib.h>
#include <string>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

using namespace std;

class SDLBase
{
	private:
		static SDL_Surface *screen;
	public:
		static void loadSDL();
		static void unloadSDL();
		static SDL_Surface* getScreen();
		static SDL_Surface* loadImage(string arquivo);
		static void renderSurface(SDL_Surface* surface, SDL_Rect* clip, SDL_Rect* dst);
		static void atualizarTela();
		static void drawLine(int x1, int y1, int x2, int y2, int rgb, int spacing);
		static SDL_Surface* clip(SDL_Surface* original, SDL_Rect* clip);
		static SDL_Surface* rotoZoom(SDL_Surface* surface, float angle, float scalex = 1.0f, float scaley = 1.0f);
		static void drawRect(int x, int y, int w, int h, int rgb);
};

#endif /* SDLBASE_H_ */
