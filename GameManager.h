/*
 * GameManager.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef GAMEMANAGER_H_
#define GAMEMANAGER_H_

#define TILEMAP_SIZE 16
#define TILE_SIZE 75
#define N_TEXTURES 30

#include "Sprite.h"
#include "SDLBase.h"
#include "Camera.h"
#include "PlanetRed.h"
#include "Earth.h"
#include "Moon.h"
#include "TileSet.h"
#include "TileMap.h"
#include "InputManager.h"
#include <stdlib.h>
#include <vector>

class GameManager
{
	private:
		Camera *camera;
		InputManager *inputManager;
		Sprite *bg;
		Sprite *planetSprite;
		Sprite *earthSprite;
		Sprite *moonSprite;
		std::vector<Planet*> planetArray;
		TileSet *tileSet;
		TileMap *tileMap;
		Earth *earth;
		Moon *moon;
		bool quit;
		int dt, frameStart, frameEnd;

	public:
		GameManager();
		~GameManager();
		void run();
		void update(int dt);
		void render();
		void addPlanet();
		void checkPlanets();
};

#endif /* GAMEMANAGER_H_ */
