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
	protected:
		SDL_Surface* surface;
		SDL_Rect clipRect;
		SDL_Surface* surfaceRotoZoom;
		float angle;
		float scaleX, scaleY;
	public:
		Sprite(string arquivo);
		virtual ~Sprite();
		SDL_Rect* getClip();
		int getWidth();
		int getHeight();
		void load(string arquivo);
		void clip(int x, int y, int w, int h);
		virtual void update(int dt) { };
		void render(int x, int y);
		void rotoZoom(float angle, float scalex = 1.0f, float scaley = 1.0f, bool force = false);
		void restore();
};

#endif /* SPRITE_H_ */
