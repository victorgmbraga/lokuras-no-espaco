/*
 * GameManager.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef GAMEMANAGER_H_
#define GAMEMANAGER_H_

#define SCROLL 20.0f
#define TILEMAP_SIZE 16
#define TILE_SIZE 75
#define N_TEXTURES 30

#include "Sprite.h"
#include "SDLBase.h"
#include "Planet.h"
#include "TileSet.h"
#include "TileMap.h"
#include <stdlib.h>
#include <vector>

class GameManager
{
	private:
		Sprite *bg;
		Sprite *planetSprite;
		std::vector<Planet*> planetArray;
		TileSet *tileSet;
		TileMap *tileMap;
		float cameraX, cameraY;
		float cameraSpeedX, cameraSpeedY;
		bool quit;

	public:
		GameManager (void);
		~GameManager (void);
		void run (void);
		void addPlanet (void);
		void checkPlanets (void);
		void processEvents (void);
		void loadTiles (void);
		void buildMap (void);
};

#endif /* GAMEMANAGER_H_ */
