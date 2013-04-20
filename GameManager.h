/*
 * GameManager.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef GAMEMANAGER_H_
#define GAMEMANAGER_H_

#include "Sprite.h"
#include "SDLBase.h"
#include "Planet.h"
#include <stdlib.h>
#include <vector>

class GameManager
{
	private:
		Sprite *bg;
		Sprite *planetSprite;
		std::vector<Planet*> planetArray;

	public:
		GameManager (void);
		~GameManager (void);
		void run (void);
		void addPlanet (void);
		void checkPlanets (void);
		void processEvents (void);
};

#endif /* GAMEMANAGER_H_ */
