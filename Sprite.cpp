/*
 * Sprite.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "Sprite.h"

Sprite::Sprite(string arquivo)
{
	surface = NULL;
	load(arquivo);
}

Sprite::~Sprite()
{
	if (surface != NULL)
		SDL_FreeSurface(surface);
}

void Sprite::load(string arquivo)
{
	// Limpa a surface se não vazia
	if (surface != NULL)
		SDL_FreeSurface(surface);

	// Carrega a imagem na surface
	surface = SDLBase::loadImage(arquivo);

	// Define o retângulo de clipping (mostrando a imagem toda)
	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = surface->w;
	clipRect.h = surface->h;
}

void Sprite::clip(int x, int y, int w, int h)
{
	// Define o retângulo de clipping
	clipRect.x = x;
	clipRect.y = y;
	clipRect.w = w;
	clipRect.h = h;
}

SDL_Rect* Sprite::getClip()
{
	return &clipRect;
}

void Sprite::render(int x, int y)
{
	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = 0;
	dst.h = 0;

	SDLBase::renderSurface(surface, &clipRect, &dst);
}

int Sprite::getWidth()
{
	return surface->w;
}

int Sprite::getHeight()
{
	return surface->h;
}



