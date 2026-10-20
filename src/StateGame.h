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
#include "GameSettings.h"
#include "Turret.h"
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

		// TileSet e TileMap (mapa aleatorio gerado a cada partida)
		TileSet *tileSet;
		TileMap *tileMap;
		int worldWidth, worldHeight; // limites do mapa atual (px)

		// Planetas perigosos (vermelho e roxo; surgem sozinhos em ritmo crescente)
		Sprite *redPlanetSprite;
		Sprite *purplePlanetSprite;
		std::vector<Planet*> planetArray;
		int redPlanetCooldown;

		// Terra
		Sprite *earthSprite;
		Earth *earth;

		// Lua (perigo em orbita)
		Sprite *moonSprite;
		Moon *moon;

		// UFO (perseguidor autonomo, chefe em ondas)
		Sprite *ufoSprite;
		FollowerObject *ufo;
		int wave;                 // onda atual do chefe (1, 2, 3...)
		int ufoRespawnTimer;      // ms ate a proxima onda (0 = sem respawn)
		int ufoShootTimer;        // cadencia de tiros do chefe (fase 2+)
		int ufoSummonTimer;       // invocacao de planetas (fase 3)
		int ufosKilledThisRun;
		// Explosao em cadeia quando o chefe morre (multiplos "booms")
		int chainTimer, chainTick;
		float chainX, chainY;

		// Torres fixas nos paineis solidos do mapa (cada uma com sua sprite,
		// porque o rotoZoom da mira e por sprite).
		std::vector<Turret*> turretArray;

		// Nave
		Animation *shipAnimation;
		AccObject *ship;

		// Projeteis: tiros da nave e do chefe/torres (flag "enemy")
		struct Bullet
		{
			float x, y;
			float vx, vy;
			int life;    // ms restantes
			int damage;  // dano ao jogador (so para tiros inimigos)
			int grace;   // ms em que o tile solido da origem nao "engole" o tiro
			bool enemy;
		};
		std::vector<Bullet> bulletArray;
		int shootCooldown;

		// Pontuacao e combo
		int score;
		int comboStreak;     // abates emendados sem deixar o timer vencer
		int comboTimer;      // ms restantes da janela de combo
		float comboMult;     // multiplicador atual (exibido no HUD)
		int bestStreak;      // maior sequencia da partida (estatistica final)
		int survivalTimer;   // acumula 10 s de sobrevivencia -> +10 pontos

		// Combustivel
		float fuel;          // 0..100 (%); drena com o tempo de partida
		int fuelDamageTimer; // 1 s sem combustivel = 4 de vida
		bool fuelAnnounced;  // toast de "sem combustivel" so uma vez

		// Textos flutuantes (pool de Text pre-alocado)
		struct Floater
		{
			Text *text;
			bool active;
			bool big;
			float x, y;   // posicao no mundo
			int life;     // ms restantes
			int total;    // vida inicial (para a subida ser linear)
		};
		std::vector<Floater> floaters;

		// Flags de partida (conquistas)
		bool tookDamage;

		// Contador de disparos inimigos emitidos (torres + chefe) e timer do
		// JSON de diagnostico (publicado em window.__lokuras_debug no web).
		int enemyShotsFired;
		int debugMs;

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
		Sprite *powerUpSprites[4];
		int powerUpCooldown;

		// Estado da partida
		Timer *gameTimer;
		int gameTimeMs;
		int destroyedCount;
		int invulnTime;     // ms de i-frames restantes
		float gravX, gravY; // "corrente" gravitacional da Terra (px/ms)
		int shakeTime;      // ms de tremor de tela restantes
		float shipPrevX, shipPrevY; // posicao antes do update (quique em tiles)
		bool paused;

		// Variáveis de retorno
		int returnState;
		int returnStack;

		void buildRandomMap();
		void placeEarthAndUfo();
		void spawnInitialPlanets();
		void buildTurrets();
		void updateTurrets(int dt);
		bool lineIsClear(float x0, float y0, float x1, float y1);
		void checkCollision(int dt);
		void shoot();
		void addEnemyShot(float fromX, float fromY, float targetX, float targetY,
		                  float speed, int damage, int spreadDeg);
		void updateBullets(int dt);
		bool bulletHit(const Bullet &bullet, GameObject *object);
		void resolveShipTiles();
		void addPlanet();
		int nextPlanetInterval();
		void spawnPowerUp();
		void spawnDrop(float x, float y, int chancePercent);
		void addKillScore(int base, float x, float y);
		void killUfo();
		void respawnUfo();
		void updateUfoBehavior(int dt);
		void updateFuel(int dt);
		void updateSurvivalScore(int dt);
		void addFloater(string text, float x, float y, SDL_Color color, bool big, int lifeMs);
		void toast(string text);
		void updateFloaters(int dt);
		void renderFloaters(float cameraX, float cameraY);
		void tryAch(int id);
		void checkAchievements();
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
