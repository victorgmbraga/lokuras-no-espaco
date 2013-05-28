/*
 * SDLBase.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "SDLBase.h"

SDL_Surface* SDLBase::screen = NULL;

void SDLBase::inicializaSDL()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
	SDL_WM_SetCaption("Trabalho 2 10/0126481", "Trabalho 2 10/0126481");
}

SDL_Surface* SDLBase::getScreen()
{
	return screen;
}

SDL_Surface* SDLBase::loadImage(string arquivo)
{
	SDL_Surface *surface = 0;
	SDL_Surface *surface_aux  = 0;

	// Carrega a imagem
	surface = IMG_Load(arquivo.c_str());

	// Verifica se há transparência na imagem e otimiza ela p/ o tamanho da tela
	if (surface->format->Amask != 0)
		surface_aux = SDL_DisplayFormatAlpha(surface);
	else
		surface_aux = SDL_DisplayFormat(surface);

	// Limpa a surface não otimizada
	SDL_FreeSurface(surface);

	// Retorna a surface otimizada
	return surface_aux;
}



void SDLBase::renderSurface(SDL_Surface* surface, SDL_Rect* clip = NULL, SDL_Rect* dst = NULL)
{
	// surface – a superfície que será colada
	// clip – o retângulo da superfície a ser colado
	// screen – a superfície destino
	// dst – retângulo onde a superfície será colada

	// "Cola" a surface na tela (screen)
	SDL_BlitSurface(surface, clip, screen, dst);
}

void SDLBase::atualizarTela()
{
	// Atualiza a tela com as alterações
	SDL_Flip(screen);
}



