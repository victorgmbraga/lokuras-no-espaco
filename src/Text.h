/*
 * Text.h
 *
 *  Created on: 11/06/2013
 *      Author: Victor
 */

#ifndef TEXT_H_
#define TEXT_H_

#include "SDLBase.h"

class Text
{
	private:
		TTF_Font *font;
		SDL_Surface *textSurface;
		string fontFile;
		string text;
		int style;
		int ptsize;
		SDL_Color color;
		SDL_Rect srcRect;
		SDL_Rect dstRect;
		string to_string(int number);
	public:
		Text(string fontFile, string text, int ptsize = 10, int style = 0, SDL_Color color = {0, 0, 0}, int x = 0, int y = 0);
		virtual ~Text();
		void render(int cameraX = 0, int cameraY = 0);
		void setPos(int x, int y);
		void setText (string text);
		void setText (int number);
		void setColor(SDL_Color color);
		void setStyle (int style);
		void setSize(int ptsize);
		float getWidth();
		float getHeight();
};

#endif /* TEXT_H_ */
