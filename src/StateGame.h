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

#include <iostream>
#include <string>
#include "Animation.h"
#include "AccObject.h"
#include "Sprite.h"
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
#include "State.h"
#include "Audio.h"
#include "Text.h"
#include "Timer.h"
#include "HUD.h"

using namespace std;

class StateGame: public State
{
	private:
		// Câmera
		Camera *camera;

		// Input Manager
		InputManager *inputManager;

		// Background
		Sprite *bg;

		// TileSet e TileMap
		TileSet *tileSet;
		TileMap *tileMap;

		// Planetas Vermelhos
		Sprite *redPlanetSprite;
		std::vector<Planet*> planetArray;
		Timer *redPlanetCoolDown;

		// Terra
		Sprite *earthSprite;
		Earth *earth;

		// Lua
		Sprite *moonSprite;
		Moon *moon;

		// UFO
		Sprite *ufoSprite;
		FollowerObject *ufo;

		// Nave
		Animation *shipAnimation;
		AccObject *ship;

		// Música e Sons
		Audio *music;
		Audio *sfx;

		// HUD
		HUD *hud;

		// Explosão
		Animation *boom;
		float boomX, boomY;

		// Variáveis de retorno
		int returnState;
		int returnStack;

		void checkCollision(int dt);
		void addPlanet();
	public:
		StateGame() {}
		virtual ~StateGame() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* GAMEMANAGER_H_ */
