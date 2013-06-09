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

#include "Animation.h"
#include "AccObject.h"
#include "Sprite.h"
#include "SDLBase.h"
#include "Camera.h"
#include "PlanetRed.h"
#include "Earth.h"
#include "Moon.h"
#include "TileSet.h"
#include "TileMap.h"
#include "InputManager.h"
#include "FollowerObject.h"
#include <stdlib.h>
#include <vector>

class GameManager
{
	private:
		bool quit;
		int dt, frameStart, frameEnd;

		Camera *camera;
		InputManager *inputManager;

		Sprite *bg;

		TileSet *tileSet;
		TileMap *tileMap;

		Sprite *redPlanetSprite;
		std::vector<Planet*> planetArray;

		Sprite *earthSprite;
		Earth *earth;

		Sprite *moonSprite;
		Moon *moon;

		Sprite *ufoSprite;
		FollowerObject *ufo;

		Animation *shipAnimation;
		AccObject *ship;

		Animation *boom;
		float boomX, boomY;

		void checkCollision(int dt);
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
