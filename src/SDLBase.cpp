/*
 * SDLBase.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "SDLBase.h"

SDL_Surface* SDLBase::screen = NULL;
bool SDLBase::fullscreen = false;

void SDLBase::loadSDL()
{
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);

	// Joystick/gamepad: opcional, nao impede o jogo de rodar se faltar.
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
		cerr << "SDL: joystick indisponivel: " << SDL_GetError() << endl;

	fullscreen = false;
	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
	if (!screen)
	{
		cerr << "Nao foi possivel criar a janela " << SCREEN_W << "x" << SCREEN_H
		     << ": " << SDL_GetError() << endl;
		exit(1);
	}
	TTF_Init();
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
	SDL_WM_SetCaption(GAME_NAME, GAME_NAME);
}

void SDLBase::unloadSDL()
{
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit();
}

SDL_Surface* SDLBase::getScreen()
{
	return screen;
}

bool SDLBase::toggleFullscreen()
{
	if (!screen)
		return false;

	// O caminho normal (SDL 1.2 e sdl12-compat) troca o modo da janela sem
	// recriar a surface da tela, preservando as imagens ja carregadas.
	if (SDL_WM_ToggleFullScreen(screen))
	{
		fullscreen = !fullscreen;
		return true;
	}

	// Fallback para drivers que nao implementam WM_ToggleFullScreen.
	bool target = !fullscreen;
	Uint32 flags = SDL_SWSURFACE | (target ? SDL_FULLSCREEN : 0);
	SDL_Surface *newScreen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, flags);
	if (!newScreen)
	{
		cerr << "Nao foi possivel alternar para "
		     << (target ? "tela cheia" : "janela") << ": " << SDL_GetError() << endl;
		return false;
	}

	screen = newScreen;
	fullscreen = target;
	SDL_WM_SetCaption(GAME_NAME, GAME_NAME);
	return true;
}

bool SDLBase::isFullscreen()
{
	return fullscreen;
}

SDL_Surface* SDLBase::loadImage(string arquivo)
{
	SDL_Surface *surface = 0;
	SDL_Surface *surface_aux  = 0;

	// Carrega a imagem
	surface = IMG_Load(arquivo.c_str());

	// Asset ausente/corrompido e fatal: reporta com clareza em vez de
	// estourar segfault mais adiante.
	if (surface == NULL)
	{
		cerr << "SDLBase::loadImage: nao foi possivel carregar '" << arquivo
		     << "': " << IMG_GetError() << endl;
		return NULL;
	}

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
	// surface � a superf�cie que ser� colada
	// clip � o ret�ngulo da superf�cie a ser colado
	// screen � a superf�cie destino
	// dst � ret�ngulo onde a superf�cie ser� colada

	// "Cola" a surface na tela (screen)
	SDL_BlitSurface(surface, clip, screen, dst);
}

void SDLBase::atualizarTela()
{
	// Atualiza a tela com as altera��es
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
	// Surface nova por chamada (antes era um static, que era sobrescrito a
	// cada clip - fragil se duas superficies usassem o clip na mesma frame).
	SDL_Surface *new_surface = NULL;

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

// Circulo por Bresenham (8 octantes), pixel a pixel sobre a tela.
void SDLBase::drawCircle(int cx, int cy, int r, int rgb)
{
	if (r <= 0)
		return;

	int x = r;
	int y = 0;
	int err = 1 - r;

	while (x >= y)
	{
		SDL_Rect pixel = {cx + x, cy + y, 1, 1};
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx + y; pixel.y = cy + x;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx - y; pixel.y = cy + x;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx - x; pixel.y = cy + y;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx - x; pixel.y = cy - y;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx - y; pixel.y = cy - x;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx + y; pixel.y = cy - x;
		SDL_FillRect(screen, &pixel, rgb);
		pixel.x = cx + x; pixel.y = cy - y;
		SDL_FillRect(screen, &pixel, rgb);

		y++;
		if (err < 0)
			err += 2 * y + 1;
		else
		{
			x--;
			err += 2 * (y - x) + 1;
		}
	}
}

// Retangulo preto semitransparente cobrindo a tela toda (fade de transicao e
// pausa). O overlay e criado uma unica vez e a opacidade e ajustada a cada uso.
void SDLBase::drawOverlay(int alpha)
{
	if (alpha <= 0)
		return;

	static SDL_Surface *overlay = NULL;

	if (overlay == NULL)
	{
		overlay = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
			SCREEN_W, SCREEN_H, 32, 0, 0, 0, 0);
		if (overlay == NULL)
			return;
		SDL_FillRect(overlay, NULL, SDL_MapRGBA(overlay->format, 0, 0, 0, SDL_ALPHA_OPAQUE));
	}

	if (alpha > 255)
		alpha = 255;

	SDL_SetAlpha(overlay, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
	SDL_BlitSurface(overlay, NULL, screen, NULL);
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





