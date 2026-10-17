/*
 * GameManager.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef GAMEMANAGER_H_
#define GAMEMANAGER_H_

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
#include "PowerUp.h"
#include "PlayStats.h"
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

		// Lua (perigo em orbita)
		Sprite *moonSprite;
		Moon *moon;

		// UFO (perseguidor autonomo)
		Sprite *ufoSprite;
		FollowerObject *ufo;

		// Nave
		Animation *shipAnimation;
		AccObject *ship;

		// Música e Sons
		Audio *music;
		Audio *sfx;
		Audio *sfxPickup;
		Audio *sfxEngine;

		// HUD
		HUD *hud;

		// Explosão (one-shot)
		Animation *boom;
		float boomX, boomY;
		bool boomActive;

		// Power-ups
		std::vector<PowerUp*> powerUpArray;
		Sprite *powerUpSprites[3];
		int powerUpCooldown;

		// Estado da partida
		Timer *gameTimer;
		int gameTimeMs;
		int destroyedCount;
		int invulnTime;     // ms de i-frames restantes
		float gravX, gravY; // "corrente" gravitacional da Terra (px/ms)
		int shakeTime;      // ms de tremor de tela restantes
		bool paused;

		// Variáveis de retorno
		int returnState;
		int returnStack;

		void checkCollision(int dt);
		void addPlanet();
		void spawnPowerUp();
		void endGame(bool win);
		void renderObjective(float cameraX, float cameraY);
	public:
		StateGame() {}
		virtual ~StateGame() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* GAMEMANAGER_H_ */
