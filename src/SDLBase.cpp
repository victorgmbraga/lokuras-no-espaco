/*
 * SDLBase.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "SDLBase.h"

SDL_Surface* SDLBase::screen = NULL;

void SDLBase::loadSDL()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
	TTF_Init();
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
	SDL_WM_SetCaption(GAME_NAME, GAME_NAME);
}

void SDLBase::unloadSDL()
{
	TTF_Quit();
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

void SDLBase::drawLine(int x1, int y1, int x2, int y2, int rgb, int spacing)
{
	int deltax = abs(x2 - x1); // The difference between the x's
	int deltay = abs(y2 - y1); // The difference between the y's
	int x = x1;                // Start x off at the first pixel
	int y = y1;                // Start y off at the first pixel
	int xinc1 = 0;
	int yinc1 = 0;
	int xinc2 = 0;
	int yinc2 = 0;
	int den = 0;
	int num = 0;
	int numadd = 0;
	int numpixels = 0;

	if (x2 >= x1)              // The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else                       // The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1)              // The y-values are increasing
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else                       // The y-values are decreasing
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay)		// There is at least one x-value for every y-value
	{
		xinc1 = 0;				// Don't change the x when numerator >= denominator
		yinc2 = 0;				// Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;		// There are more x-values than y-values
	}
	else						// There is at least one y-value for every x-value
	{
		xinc2 = 0;				// Don't change the x for every iteration
		yinc1 = 0;				// Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;		// There are more y-values than x-values
	}

	for (int curpixel = 1; curpixel <= numpixels; curpixel++)
	{
		if (spacing == 0)
		{
			SDL_Rect pixel = {x,y,1,1};
			SDL_FillRect(screen,&pixel,rgb);
		}
		else if (curpixel % spacing >= spacing/2)
		{
			SDL_Rect pixel = {x,y,1,1};
			SDL_FillRect(screen,&pixel,rgb);
		}

		num += numadd;         // Increase the numerator by the top of the fraction
		if (num >= den)        // Check if numerator >= denominator
		{
			num -= den;         // Calculate the new numerator value
			x += xinc1;         // Change the x as appropriate
			y += yinc1;         // Change the y as appropriate
		}
		x += xinc2;             // Change the x as appropriate
		y += yinc2;             // Change the y as appropriate
	}
}

SDL_Surface* SDLBase::clip(SDL_Surface* original, SDL_Rect* clip)
{
	static SDL_Surface *new_surface = NULL;

	float width = clip->w;
	float height = clip->h;

	new_surface = SDL_CreateRGBSurface(screen->flags, width, height, screen->format->BitsPerPixel, 0 , 0, 0, 0);

	SDL_BlitSurface(original, clip, new_surface, NULL);

	return new_surface;
}

SDL_Surface* SDLBase::rotoZoom(SDL_Surface* surface, float angle, float scalex, float scaley)
{
	if(surface != NULL && scalex > 0 && scaley > 0)
		return rotozoomSurfaceXY(surface, angle, scalex, scaley, 1);
	return NULL;
}

void SDLBase::drawRect(int x, int y, int w, int h, int rgb)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_FillRect(screen, &rect, rgb);
}





