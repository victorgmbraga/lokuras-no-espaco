/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "GameManager.h"

GameManager::GameManager (void)
{
	SDLBase::inicializaSDL();
	// Define o BG
	bg = new Sprite("fundo.png");
}

GameManager::~GameManager (void)
{
	delete(bg);
}

void GameManager::run (void)
{
	while(!SDL_QuitRequested())
	{
		// Captura de Input

		// Atualização dos objetos

		// Todos os comandos de renderização
		bg->render(0,0);

		// Atualizar a tela
		SDLBase::atualizarTela();
	}
}



