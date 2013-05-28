/*
 * Sprite.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef SPRITE_H_
#define SPRITE_H_

#include "SDLBase.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <stdlib.h>
#include <string>

class Sprite
{
	private:
		SDL_Surface* surface;
		SDL_Rect clipRect;

	public:
		Sprite(string arquivo);
		~Sprite();
		void load(string arquivo);
		void clip(int x, int y, int w, int h);
		SDL_Rect* getClip();
		void render(int x, int y);
		int getWidth();
		int getHeight();
};

#endif /* SPRITE_H_ */
