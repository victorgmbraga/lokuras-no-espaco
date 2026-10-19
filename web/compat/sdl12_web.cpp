/*
 * sdl12_web.cpp
 *
 *  Created on: 19/10/2026
 *      Author: Victor
 *
 *  Implementacao da camada de compatibilidade SDL 1.2 do build WebAssembly
 *  (declarada em web/compat/SDL/SDL.h) sobre a SDL 2 + ports do Emscripten.
 *
 *  Como a SDL 1.2, a tela e uma surface de software: o jogo desenha nela com
 *  SDL_BlitSurface/SDL_FillRect e apresenta com SDL_Flip, que aqui envia a
 *  surface para uma textura (SDL_UpdateTexture) e a desenha no canvas
 *  (SDL_RenderPresent). A entrada e traduzida no outro sentido: os eventos da
 *  SDL 2 viram eventos com os codigos SDLK_* da 1.2 e o estado do teclado e
 *  reconstruido no indice esperado pelo InputManager.
 */

#include "SDL/SDL.h"
#include "SDL/SDL_rotozoom.h"

#include <emscripten.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// A implementacao usa as funcoes reais da SDL 2; as macros acima existem para
// o codigo do jogo (assinaturas da SDL 1.2) e nao valem neste arquivo.
#undef SDL_CreateRGBSurface
#undef SDL_PollEvent
#undef rotozoomSurfaceXY

// ---------------------------------------------------------------------------
// Estado interno
// ---------------------------------------------------------------------------

// Grava o /save no IndexedDB agora (o shell tambem sincroniza periodicamente,
// mas o recorde nao precisa esperar o proximo intervalo).
extern "C" void lokurasWebSincronizarSave(void)
{
	EM_ASM({
		if (Module.lokurasSincronizarSave) Module.lokurasSincronizarSave();
	});
}

namespace
{

SDL_Window *janela = NULL;
SDL_Renderer *renderizador = NULL;
SDL_Texture *textura = NULL;
SDL_Surface *tela = NULL;
int larguraJanela = 0;
int alturaJanela = 0;
bool telaCheia = false;
std::string tituloJanela = "Lokuras no Espaco";

// Estado do teclado no formato da SDL 1.2: um array indexado pelos SDLK_* da
// 1.2 (o Emscripten so expoe o estado por scancode, da SDL 2).
const int N_KEYS_SDL12 = 512;
Uint8 teclas12[N_KEYS_SDL12];

// Eventos sintetizados pela traducao (a roda do mouse vira dois eventos de
// botao, 4/5, como na SDL 1.2). Fila LIFO: so o proximo e consumido.
SDL_Event filaEventos[4];
int filaEventosCount = 0;

// Codigo SDL 1.2 correspondente a cada scancode da SDL 2 (0 = sem traducao).
Uint32 mapaTeclas[SDL_NUM_SCANCODES];
bool mapaTeclasPronto = false;

// Gamepads abertos automaticamente (no navegador eles aparecem depois que o
// jogo ja comecou, entao nao da para abrir "de uma vez" no inicio).
std::vector<SDL_Joystick *> joysticks;

// Atualiza o titulo da aba do navegador junto com o titulo da "janela".
EM_JS(void, lokurasWebSetDocumentTitle, (const char *title), {
	document.title = UTF8ToString(title);
});

// Conta os frames apresentados (usado pelo auto-teste da pagina web).
EM_JS(void, lokurasWebContaFrame, (void), {
	window.__lokuras_frames = (window.__lokuras_frames | 0) + 1;
});

void montarMapaTeclas()
{
	if (mapaTeclasPronto)
		return;

	mapaTeclasPronto = true;
	memset(mapaTeclas, 0, sizeof(mapaTeclas));

	// Letras e digitos: na SDL 1.2 o keysym e o proprio caractere minusculo.
	for (int i = SDL_SCANCODE_A; i <= SDL_SCANCODE_Z; ++i)
		mapaTeclas[i] = (Uint32)('a' + i - SDL_SCANCODE_A);
	for (int i = SDL_SCANCODE_1; i <= SDL_SCANCODE_9; ++i)
		mapaTeclas[i] = (Uint32)('1' + i - SDL_SCANCODE_1);
	mapaTeclas[SDL_SCANCODE_0] = '0';

	// Pontuacao e edicao (os codigos ASCII da 1.2 sao iguais aos da SDL 2).
	mapaTeclas[SDL_SCANCODE_RETURN]       = SDLK_RETURN;
	mapaTeclas[SDL_SCANCODE_ESCAPE]       = SDLK_ESCAPE;
	mapaTeclas[SDL_SCANCODE_BACKSPACE]    = SDLK_BACKSPACE;
	mapaTeclas[SDL_SCANCODE_TAB]          = SDLK_TAB;
	mapaTeclas[SDL_SCANCODE_SPACE]        = SDLK_SPACE;
	mapaTeclas[SDL_SCANCODE_MINUS]        = SDLK_MINUS;
	mapaTeclas[SDL_SCANCODE_EQUALS]       = SDLK_EQUALS;
	mapaTeclas[SDL_SCANCODE_LEFTBRACKET]  = SDLK_LEFTBRACKET;
	mapaTeclas[SDL_SCANCODE_RIGHTBRACKET] = SDLK_RIGHTBRACKET;
	mapaTeclas[SDL_SCANCODE_BACKSLASH]    = SDLK_BACKSLASH;
	mapaTeclas[SDL_SCANCODE_SEMICOLON]    = SDLK_SEMICOLON;
	mapaTeclas[SDL_SCANCODE_APOSTROPHE]   = SDLK_QUOTE;
	mapaTeclas[SDL_SCANCODE_GRAVE]        = SDLK_BACKQUOTE;
	mapaTeclas[SDL_SCANCODE_COMMA]        = SDLK_COMMA;
	mapaTeclas[SDL_SCANCODE_PERIOD]       = SDLK_PERIOD;
	mapaTeclas[SDL_SCANCODE_SLASH]        = SDLK_SLASH;
	mapaTeclas[SDL_SCANCODE_CAPSLOCK]     = SDLK_CAPSLOCK;
	mapaTeclas[SDL_SCANCODE_DELETE]       = SDLK_DELETE;

	// Navegacao.
	mapaTeclas[SDL_SCANCODE_UP]        = SDLK_UP;
	mapaTeclas[SDL_SCANCODE_DOWN]      = SDLK_DOWN;
	mapaTeclas[SDL_SCANCODE_LEFT]      = SDLK_LEFT;
	mapaTeclas[SDL_SCANCODE_RIGHT]     = SDLK_RIGHT;
	mapaTeclas[SDL_SCANCODE_INSERT]    = SDLK_INSERT;
	mapaTeclas[SDL_SCANCODE_HOME]      = SDLK_HOME;
	mapaTeclas[SDL_SCANCODE_END]       = SDLK_END;
	mapaTeclas[SDL_SCANCODE_PAGEUP]    = SDLK_PAGEUP;
	mapaTeclas[SDL_SCANCODE_PAGEDOWN]  = SDLK_PAGEDOWN;
	mapaTeclas[SDL_SCANCODE_PRINTSCREEN] = SDLK_PRINT;
	mapaTeclas[SDL_SCANCODE_SCROLLLOCK]  = SDLK_SCROLLOCK;
	mapaTeclas[SDL_SCANCODE_PAUSE]       = SDLK_PAUSE;
	mapaTeclas[SDL_SCANCODE_NUMLOCKCLEAR] = SDLK_NUMLOCK;

	// Teclado numerico.
	mapaTeclas[SDL_SCANCODE_KP_0]        = SDLK_KP0;
	mapaTeclas[SDL_SCANCODE_KP_1]        = SDLK_KP1;
	mapaTeclas[SDL_SCANCODE_KP_2]        = SDLK_KP2;
	mapaTeclas[SDL_SCANCODE_KP_3]        = SDLK_KP3;
	mapaTeclas[SDL_SCANCODE_KP_4]        = SDLK_KP4;
	mapaTeclas[SDL_SCANCODE_KP_5]        = SDLK_KP5;
	mapaTeclas[SDL_SCANCODE_KP_6]        = SDLK_KP6;
	mapaTeclas[SDL_SCANCODE_KP_7]        = SDLK_KP7;
	mapaTeclas[SDL_SCANCODE_KP_8]        = SDLK_KP8;
	mapaTeclas[SDL_SCANCODE_KP_9]        = SDLK_KP9;
	mapaTeclas[SDL_SCANCODE_KP_PERIOD]   = SDLK_KP_PERIOD;
	mapaTeclas[SDL_SCANCODE_KP_DIVIDE]   = SDLK_KP_DIVIDE;
	mapaTeclas[SDL_SCANCODE_KP_MULTIPLY] = SDLK_KP_MULTIPLY;
	mapaTeclas[SDL_SCANCODE_KP_MINUS]    = SDLK_KP_MINUS;
	mapaTeclas[SDL_SCANCODE_KP_PLUS]     = SDLK_KP_PLUS;
	mapaTeclas[SDL_SCANCODE_KP_ENTER]    = SDLK_KP_ENTER;
	mapaTeclas[SDL_SCANCODE_KP_EQUALS]   = SDLK_KP_EQUALS;

	// F1..F12 (SDLK_F1 == 282 na SDL 1.2).
	for (int i = SDL_SCANCODE_F1; i <= SDL_SCANCODE_F12; ++i)
		mapaTeclas[i] = (Uint32)(SDLK_F1 + i - SDL_SCANCODE_F1);

	// Modificadores (SDLK_LSHIFT == 304 etc.).
	mapaTeclas[SDL_SCANCODE_LSHIFT] = SDLK_LSHIFT;
	mapaTeclas[SDL_SCANCODE_RSHIFT] = SDLK_RSHIFT;
	mapaTeclas[SDL_SCANCODE_LCTRL]  = SDLK_LCTRL;
	mapaTeclas[SDL_SCANCODE_RCTRL]  = SDLK_RCTRL;
	mapaTeclas[SDL_SCANCODE_LALT]   = SDLK_LALT;
	mapaTeclas[SDL_SCANCODE_RALT]   = SDLK_RALT;
	mapaTeclas[SDL_SCANCODE_LGUI]   = SDLK_LMETA;
	mapaTeclas[SDL_SCANCODE_RGUI]   = SDLK_RMETA;
}

void criarJanela(int largura, int altura)
{
	janela = SDL_CreateWindow(tituloJanela.c_str(), SDL_WINDOWPOS_CENTERED,
	                          SDL_WINDOWPOS_CENTERED, largura, altura, 0);
	if (!janela)
	{
		SDL_SetError("SDL_CreateWindow falhou: %s", SDL_GetError());
		return;
	}

	// Prefere um renderizador acelerado (WebGL no navegador); se nao houver,
	// aceita o de software - nos dois casos a textura recebe a surface.
	renderizador = SDL_CreateRenderer(janela, -1, 0);
	if (!renderizador)
		renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_SOFTWARE);
	if (!renderizador)
	{
		SDL_SetError("SDL_CreateRenderer falhou: %s", SDL_GetError());
		return;
	}

	textura = SDL_CreateTexture(renderizador, SDL_PIXELFORMAT_ARGB8888,
	                            SDL_TEXTUREACCESS_STREAMING, largura, altura);
	if (!textura)
	{
		SDL_SetError("SDL_CreateTexture falhou: %s", SDL_GetError());
		return;
	}
	SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_NONE);

	larguraJanela = largura;
	alturaJanela = altura;
}

void abrirJoystick(int deviceIndex)
{
	SDL_Joystick *joy = SDL_JoystickOpen(deviceIndex);
	if (!joy)
		return;

	// Na SDL 1.2 os eixos/botoes chegam como eventos; habilita e guarda a
	// referencia para fechar quando o gamepad for desconectado.
	SDL_JoystickEventState(SDL_ENABLE);
	joysticks.push_back(joy);
}

void fecharJoystick(int instanceId)
{
	for (size_t i = 0; i < joysticks.size(); ++i)
	{
		if (SDL_JoystickInstanceID(joysticks[i]) == instanceId)
		{
			SDL_JoystickClose(joysticks[i]);
			joysticks.erase(joysticks.begin() + i);
			return;
		}
	}
}

}  // namespace

// ---------------------------------------------------------------------------
// Video
// ---------------------------------------------------------------------------

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	(void)bpp;  // o jogo sempre pede 32 bits; a textura usa ARGB8888

	montarMapaTeclas();

	if (!janela)
	{
		criarJanela(width, height);
		if (!janela || !renderizador || !textura)
			return NULL;
	}
	else if (width != larguraJanela || height != alturaJanela)
	{
		SDL_SetWindowSize(janela, width, height);
		SDL_DestroyTexture(textura);
		textura = SDL_CreateTexture(renderizador, SDL_PIXELFORMAT_ARGB8888,
		                            SDL_TEXTUREACCESS_STREAMING, width, height);
		if (!textura)
			return NULL;
		SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_NONE);
		larguraJanela = width;
		alturaJanela = height;
	}

	telaCheia = (flags & SDL_FULLSCREEN) != 0;
	SDL_SetWindowFullscreen(janela, telaCheia ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

	// A "tela" da SDL 1.2: surface de software 32 bits onde o jogo desenha.
	if (tela)
		SDL_FreeSurface(tela);
	tela = SDL_CreateRGBSurface(0, width, height, 32,
	                            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (!tela)
	{
		SDL_SetError("SDL_CreateRGBSurface falhou: %s", SDL_GetError());
		return NULL;
	}
	memset(tela->pixels, 0, (size_t)tela->pitch * height);

	return tela;
}

int SDL_Flip(SDL_Surface *screen)
{
	if (!screen || !textura || !renderizador)
		return -1;

	// O jogo so apresenta a tela principal.
	if (screen != tela)
		return 0;

	if (SDL_UpdateTexture(textura, NULL, screen->pixels, screen->pitch) != 0)
		return -1;

	SDL_RenderClear(renderizador);
	SDL_RenderCopy(renderizador, textura, NULL, NULL);
	SDL_RenderPresent(renderizador);

	lokurasWebContaFrame();
	return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{
	(void)icon;

	if (!title)
		return;

	tituloJanela = title;
	if (janela)
		SDL_SetWindowTitle(janela, title);
	lokurasWebSetDocumentTitle(title);
}

int SDL_WM_ToggleFullScreen(SDL_Surface *screen)
{
	if (!janela || !screen)
		return 0;

	bool alvo = !telaCheia;
	if (SDL_SetWindowFullscreen(janela, alvo ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
		return 0;

	telaCheia = alvo;
	return 1;
}

SDL_Surface *SDL_DisplayFormat(SDL_Surface *surface)
{
	if (!surface)
		return NULL;

	SDL_Surface *convertida = tela
		? SDL_ConvertSurface(surface, tela->format, 0)
		: SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
	if (!convertida)
		return NULL;

	// Sem alpha: copia direta (a SDL 1.2 nao tinha blend mode por surface).
	SDL_SetSurfaceBlendMode(convertida, SDL_BLENDMODE_NONE);
	return convertida;
}

SDL_Surface *SDL_DisplayFormatAlpha(SDL_Surface *surface)
{
	if (!surface)
		return NULL;

	SDL_Surface *convertida = SDL_DisplayFormat(surface);
	if (!convertida)
		return NULL;

	// Com alpha: a SDL 1.2 marcava SDL_SRCALPHA; na SDL 2 isso e o
	// SDL_BLENDMODE_BLEND na surface.
	SDL_SetSurfaceBlendMode(convertida, SDL_BLENDMODE_BLEND);
	return convertida;
}

int SDL_SetAlpha(SDL_Surface *surface, Uint32 flags, Uint8 alpha)
{
	if (!surface)
		return -1;

	if (flags & SDL_SRCALPHA)
	{
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(surface, alpha);
	}
	else
	{
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		SDL_SetSurfaceAlphaMod(surface, 255);
	}

	return 0;
}

SDL_Surface *lokurasSDL12CreateRGBSurface(Uint32 flags, int width, int height,
                                          int depth, Uint32 Rmask, Uint32 Gmask,
                                          Uint32 Bmask, Uint32 Amask)
{
	// Os bits SDL_SWSURFACE/SDL_SRCALPHA da SDL 1.2 nao existem na SDL 2 (e
	// nao ha o que configurar: o alpha por surface fica em SDL_SetAlpha).
	(void)flags;

	if (depth == 32 && Rmask == 0 && Gmask == 0 && Bmask == 0 && Amask == 0)
	{
		Rmask = 0x00FF0000;
		Gmask = 0x0000FF00;
		Bmask = 0x000000FF;
		Amask = 0xFF000000;
	}

	return SDL_CreateRGBSurface(0, width, height, depth, Rmask, Gmask, Bmask, Amask);
}

// ---------------------------------------------------------------------------
// Entrada
// ---------------------------------------------------------------------------

Uint8 *SDL_GetKeyState(int *numkeys)
{
	montarMapaTeclas();

	if (numkeys)
		*numkeys = N_KEYS_SDL12;

	memset(teclas12, 0, sizeof(teclas12));

	// Reconstroi o estado a partir da SDL 2 (por scancode), traduzindo para os
	// codigos da 1.2 - isso tambem evita teclas "presas" quando o canvas perde
	// o foco.
	const Uint8 *estado = SDL_GetKeyboardState(NULL);
	if (estado)
	{
		for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc)
		{
			Uint32 k = mapaTeclas[sc];
			if (k && estado[sc])
				teclas12[k] = 1;
		}
	}

	return teclas12;
}

int lokurasSDL12PollEvent(SDL_Event *event)
{
	if (!event)
		return 0;

	if (filaEventosCount > 0)
	{
		*event = filaEventos[--filaEventosCount];
		return 1;
	}

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				// keysym da SDL 2 -> SDLK_* da SDL 1.2 (modificadores e
				// scancode continuam os da SDL 2; KMOD_* tem os mesmos bits).
				e.key.keysym.sym = (SDL_Keycode)mapaTeclas[e.key.keysym.scancode];
				*event = e;
				return 1;

			case SDL_MOUSEWHEEL:
			{
				if (e.wheel.y == 0)
					continue;

				// SDL 1.2: a roda chega como MOUSEBUTTONDOWN/UP dos botoes
				// 4 (cima) e 5 (baixo). O "up" fica na fila.
				Uint8 botao = e.wheel.y > 0 ? 4 : 5;
				int mx = 0, my = 0;
				SDL_GetMouseState(&mx, &my);

				SDL_Event up;
				memset(&up, 0, sizeof(up));
				up.type = SDL_MOUSEBUTTONUP;
				up.button.button = botao;
				up.button.state = SDL_RELEASED;
				up.button.x = mx;
				up.button.y = my;
				filaEventos[filaEventosCount++] = up;

				memset(event, 0, sizeof(*event));
				event->type = SDL_MOUSEBUTTONDOWN;
				event->button.button = botao;
				event->button.state = SDL_PRESSED;
				event->button.x = mx;
				event->button.y = my;
				return 1;
			}

			// Eventos que nao existem na SDL 1.2: o gamepad conectado e
			// aberto aqui (o jogo guarda o ponteiro, mas escuta eixos/botoes
			// pelos eventos) e o evento em si nao chega ao jogo.
			case SDL_JOYDEVICEADDED:
				abrirJoystick(e.jdevice.which);
				continue;

			case SDL_JOYDEVICEREMOVED:
				fecharJoystick(e.jdevice.which);
				continue;

			default:
				*event = e;
				return 1;
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Rotozoom (SDL_gfx 1.2 -> SDL2_gfx)
// ---------------------------------------------------------------------------

SDL_Surface *lokurasSDL12RotozoomSurfaceXY(SDL_Surface *src, double angle,
                                           double zoomx, double zoomy, int smooth)
{
	if (!src)
		return NULL;

	SDL_Surface *dst = rotozoomSurfaceXY(src, angle, zoomx, zoomy, smooth);
	if (!dst)
		return NULL;

	// A SDL2_gfx cria a surface de destino sem estado de blend; na SDL 1.2 o
	// SRCALPHA da surface de origem valia tambem para o resultado (e o sprite
	// girado e justamente o caso com transparencia).
	SDL_BlendMode modo = SDL_BLENDMODE_NONE;
	SDL_GetSurfaceBlendMode(src, &modo);
	SDL_SetSurfaceBlendMode(dst, modo);

	Uint8 alphaMod = 255;
	SDL_GetSurfaceAlphaMod(src, &alphaMod);
	SDL_SetSurfaceAlphaMod(dst, alphaMod);

	return dst;
}
