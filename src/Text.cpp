/*
 * Text.cpp
 *
 *  Created on: 11/06/2013
 *      Author: Victor
 */

#include "Text.h"

Text::Text(string fontFile, string text, int ptsize, int style, SDL_Color color, int x, int y)
{
	// O redraw() abaixo libera surfaces: zera os ponteiros ANTES.
	font = NULL;
	textSurface = NULL;

	this->fontFile = fontFile;
	this->ptsize = ptsize;
	this->color = color;
	this->style = style;

	font = TTF_OpenFont(fontFile.c_str(), ptsize);
	if (font == NULL)
	{
		cerr << "Text: fonte '" << fontFile << "' nao encontrada: " << TTF_GetError() << endl;
		SDLBase::unloadSDL();
		exit(1);
	}
	TTF_SetFontStyle(font, style);

	this->text = text;
	redraw();

	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = 0;
	dstRect.h = 0;
}

Text::~Text()
{
	if (textSurface != NULL)
		SDL_FreeSurface(textSurface);
	if (font != NULL)
		TTF_CloseFont(font);
}

// (Re)renderiza a surface do texto com os atributos atuais. Chamada pelo
// construtor e por todos os setters - antes eles mudavam os atributos sem
// refletir nada na tela.
void Text::redraw()
{
	if (textSurface != NULL)
		SDL_FreeSurface(textSurface);
	textSurface = NULL;

	textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), color);

	srcRect.x = 0;
	srcRect.y = 0;
	if (textSurface != NULL)
	{
		srcRect.w = textSurface->w;
		srcRect.h = textSurface->h;
	}
	else
		srcRect.w = srcRect.h = 0;
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
	// Texto identico: evita re-renderizar em todo frame.
	if (this->text == text)
		return;

	this->text = text;
	redraw();
}

void Text::setText (int number)
{
	setText(std::to_string(number));
}

void Text::setColor(SDL_Color color)
{
	this->color = color;
	redraw();
}

void Text::setStyle(int style)
{
	this->style = style;
	if (font != NULL)
		TTF_SetFontStyle(font, style);
	redraw();
}

void Text::setSize(int ptsize)
{
	if (ptsize <= 0 || ptsize == this->ptsize)
		return;

	this->ptsize = ptsize;

	// A fonte fixa o tamanho na abertura: reabre com o novo tamanho.
	if (font != NULL)
	{
		TTF_CloseFont(font);
		font = TTF_OpenFont(fontFile.c_str(), ptsize);
		if (font == NULL)
		{
			cerr << "Text: falha ao reabrir a fonte '" << fontFile << "': " << TTF_GetError() << endl;
			SDLBase::unloadSDL();
			exit(1);
		}
		TTF_SetFontStyle(font, style);
	}
	redraw();
}

float Text::getWidth()
{
	return textSurface ? textSurface->w : 0;
}

float Text::getHeight()
{
	return textSurface ? textSurface->h : 0;
}
