/*
 * main.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateManager.h"
#include "GameSettings.h"
#include <cstdlib> // atoi
#include <cstring> // strcmp

int main(int argc, char *argv[])
{
	// Argumentos opcionais (desktop e web via Module.arguments):
	//   --seed N   gera o mapa/posicoes com a semente N (mesma seed = mesmo mundo)
	//   --daily    usa o numero do dia como semente (desafio diario igual para todos)
	for(int i = 1; i < argc; ++i)
	{
		if(!strcmp(argv[i], "--seed") && i + 1 < argc)
			GameSettings::seed() = (unsigned int)atoi(argv[++i]);
		else if(!strcmp(argv[i], "--daily"))
			GameSettings::daily() = true;
	}

	StateManager *state = new StateManager();
	state->run();
	delete(state);
	return 0;
}
