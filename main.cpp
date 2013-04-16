/*
 * main.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "GameManager.h"

int main (int argc, char *argv[])
{
	GameManager *game = new GameManager();
	game->run();
	delete(game);
	return 0;
}



