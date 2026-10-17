/*
 * StateManager.cpp
 *
 *  Created on: 13/06/2013
 *      Author: Victor
 */

#include "StateManager.h"
#include <time.h>

// Alvo de ~60 FPS: cada ciclo do loop dura ~16ms (o loop original era um
// busy-loop rodando a 100% da CPU).
static const int FRAME_MS = 16;

// Limite de dt: se a janela ficar congelada (alt-tab, pausa do sistema),
// o jogo "acorda" com o passo limitado em vez de teleportar tudo.
static const int MAX_DT = 50;

StateManager::StateManager()
{
	stack = 0;

	// Inicializa a SDL
	SDLBase::loadSDL();

	// Semeia o gerador de numeros aleatorios uma unica vez (spawns de
	// planetas/power-ups dependem dele).
	srand(time(NULL));

	timer = new Timer();
	timer->start();
	dt = 0;

	currentState = new StateMenu();
	currentState->load();

	// Dá o primeiro update na InputManager
	InputManager::getInstance().update();
}

StateManager::~StateManager()
{
	// Fecha o estado atual (antes o objeto e os recursos dele vazavam).
	if(currentState)
	{
		currentState->unload();
		delete(currentState);
		currentState = NULL;
	}

	delete(timer);

	SDLBase::unloadSDL();
}

void StateManager::run()
{
	bool quit = false;

	// Transicao suave entre estados: 0 = tela normal,
	// 1 = escurecendo, 2 = clareando apos a troca.
	int fadeAlpha = 0;
	int fadePhase = 0;
	int targetCode = 0;

	while(!quit)
	{
		// dt real entre frames (em ms).
		dt = timer->getTime();
		timer->start();

		if(dt > MAX_DT)
			dt = MAX_DT;
		if(dt <= 0)
			dt = 1;

		// Atualização da InputManager — deve ser chamado somente aqui
		InputManager::getInstance().update();

		// Alt+Enter alterna entre janela e tela cheia.
		if(InputManager::getInstance().consumeFullscreenToggle())
		{
			SDLBase::toggleFullscreen();

			// Reaplica o indicador de volume no titulo da janela.
			if(Audio::isCaptionVolumeActive())
				Audio::refreshCaption();
		}

		// Teclas globais de volume (tocam em qualquer estado).
		InputManager &im = InputManager::getInstance();
		if(im.isKeyDown(SDLK_PLUS) || im.isKeyDown(SDLK_EQUALS) || im.isKeyDown(SDLK_KP_PLUS))
			Audio::setMusicVolume(Audio::getMusicVolume() + 16);
		if(im.isKeyDown(SDLK_MINUS) || im.isKeyDown(SDLK_KP_MINUS))
			Audio::setMusicVolume(Audio::getMusicVolume() - 16);
		if(im.isKeyDown(SDLK_m))
			Audio::toggleMute();

		// STATE MANAGER — durante o fade o jogo congela (evita troca dupla).
		if(fadePhase == 0)
		{
			int request = currentState->update(dt);
			if(request != 0)
			{
				targetCode = request;
				fadePhase = 1;
			}
		}

		// Atualização dos objetos na tela
		currentState->render();

		// Transição entre estados.
		if(fadePhase == 1)
		{
			fadeAlpha += dt * 550 / 1000;
			if(fadeAlpha >= 255)
			{
				fadeAlpha = 255;

				switch(targetCode)
				{
					case STATE_MENU:
						stack = currentState->unload();
						delete(currentState);
						currentState = new StateMenu();
						currentState->load(stack);
					break;
					case STATE_GAME:
						stack = currentState->unload();
						delete(currentState);
						currentState = new StateGame();
						currentState->load(stack);
					break;
					case STATE_WINLOSE:
						stack = currentState->unload();
						delete(currentState);
						currentState = new StateWinLose();
						currentState->load(stack);
					break;
					case STATE_QUIT:
						quit = true;
					break;
				}

				fadePhase = 2;
			}
		}
		else if(fadePhase == 2)
		{
			fadeAlpha -= dt * 900 / 1000;
			if(fadeAlpha <= 0)
			{
				fadeAlpha = 0;
				fadePhase = 0;
			}
		}

		// Sombra do fade por cima do frame atual.
		if(fadeAlpha > 0)
			SDLBase::drawOverlay(fadeAlpha);

		// Atualizar a tela
		SDLBase::atualizarTela();

		// Limita a frequencia do loop (~60 FPS).
		int elapsedMs = timer->getTime();
		if(elapsedMs < FRAME_MS)
			SDL_Delay(FRAME_MS - elapsedMs);
	}
}
