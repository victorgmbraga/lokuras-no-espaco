/*
 * Text.cpp
 *
 *  Created on: 11/06/2013
 *      Author: Victor
 */

#include "Text.h"

Text::Text(string fontFile, string text, int ptsize, int style, SDL_Color color, int x, int y)
{
	this->fontFile = fontFile;
	this->text = text;
	this->ptsize = ptsize;
	this->style = style;
	this->color = color;

	font = TTF_OpenFont(fontFile.c_str(), ptsize);
	textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), color);

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = textSurface->w;
	srcRect.h = textSurface->h;

	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = 0;
	dstRect.h = 0;
}

Text::~Text()
{
	TTF_CloseFont(font);
	SDL_FreeSurface(textSurface);
}

void Text::render(int cameraX, int cameraY)
{
	if(textSurface)
		SDLBase::renderSurface(textSurface, &srcRect, &dstRect);
}

void Text::setPos(int x, int y)
{
	dstRect.x = x;
	dstRect.y = y;
}

void Text::setText (string text)
{
	this->text = text;
}

void Text::setText (int number)
{
	string text = to_string(number);
	this->text = text;
}

void Text::setColor(SDL_Color color)
{
	this->color = color;
}

void Text::setStyle(int style)
{
	this->style = style;
}

void Text::setSize(int ptsize)
{
	this->ptsize = ptsize;
}

float Text::getWidth()
{
	return textSurface->w;
}

float Text::getHeight()
{
	return textSurface->h;
}

string Text::to_string(int number)
{
    string number_string = "";
    char ones_char;
    int ones = 0;
    while(true){
        ones = number % 10;
        switch(ones){
            case 0: ones_char = '0'; break;
            case 1: ones_char = '1'; break;
            case 2: ones_char = '2'; break;
            case 3: ones_char = '3'; break;
            case 4: ones_char = '4'; break;
            case 5: ones_char = '5'; break;
            case 6: ones_char = '6'; break;
            case 7: ones_char = '7'; break;
            case 8: ones_char = '8'; break;
            case 9: ones_char = '9'; break;
        }
        number -= ones;
        number_string = ones_char + number_string;
        if(number == 0){
            break;
        }
        number = number/10;
    }
    return number_string;
}
