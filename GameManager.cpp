/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "GameManager.h"

GameManager::GameManager()
{
	SDLBase::inicializaSDL();

	camera = NULL;
	inputManager = NULL;
	quit = false;
	dt = 0;
	frameStart = 0;
	frameEnd = 0;

	earthSprite = new Sprite("img/PlanetaTerra.png");
	earth = new Earth(earthSprite, 0, 0, 100);

	moonSprite = new Sprite("img/Lua.png");
	moon = new Moon(moonSprite, 1, earth);

	ufoSprite = new Sprite("img/ufo.png");
	ufo = new FollowerObject(ufoSprite, 10, 10);

	// Carrega o BG
	bg = new Sprite("img/fundo.png");

	// Cria o TileSet
	tileSet = new TileSet(75, 75, "img/Tileset.png");

	// Cria o TileMap
	tileMap = new TileMap("map/tileMap.txt", tileSet);

	// Carrega o sprite do planeta
	planetSprite = new Sprite("img/PlanetaVermelho.png");
}

GameManager::~GameManager()
{
	delete(bg);
	delete(planetSprite);
	delete(earth);
	delete(earthSprite);
	delete(moon);
	delete(moonSprite);
	planetArray.erase(planetArray.begin(),planetArray.end());
	delete(camera);
	delete(inputManager);
	delete(ufo);
	delete(ufoSprite);
}

void GameManager::run()
{
	while(!SDL_QuitRequested())
	{
		frameStart = SDL_GetTicks();
		dt = frameStart - frameEnd;

		// Atualização dos objetos
		update(dt);

		// Todos os comandos de renderização
		render();

		// Atualizar a tela
		SDLBase::atualizarTela();

		SDL_Delay(13);

		frameEnd = frameStart;
	}
}

void GameManager::update(int dt)
{
	inputManager->getInstance().update();
	camera->getInstance().update(dt);

	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->update(dt);
	checkPlanets();
	earth->update(dt);
	moon->update(dt);
	ufo->update(dt);

	if(inputManager->getInstance().quitGame())
		quit = true;
	if(inputManager->getInstance().isKeyDown(SDLK_SPACE))
		addPlanet();
}

void GameManager::render()
{
	float cameraX = camera->getInstance().getX();
	float cameraY = camera->getInstance().getY();

	bg->render(0,0);
	tileMap->render(cameraX, cameraY);
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->render(cameraX, cameraY);
	earth->render(cameraX, cameraY);
	moon->render(cameraX, cameraY);
	ufo->renderQueueLines(cameraX, cameraY);
	ufo->render(cameraX, cameraY);
}

void GameManager::addPlanet()
{
	// Pega o tamanho da tela
	int screenWidth = bg->getWidth();
	int screenHeight = bg->getHeight();

	// Cria uma posição aleatória dentro da tela
	int planetX = camera->getInstance().getX() + (rand() % screenWidth);
	int planetY = camera->getInstance().getY() + (rand() % screenHeight);

	// Adiciona o planeta na posição
	PlanetRed *planetRed = new PlanetRed(planetSprite, planetX, planetY, 20);
	planetArray.push_back(planetRed);
}

void GameManager::checkPlanets()
{
	std::vector<Planet*>::iterator it = planetArray.begin();

	// Deleta os planetas com HP <= 0
	while(it != planetArray.end())
	{
		if((*it)->hp <= 0)
			it = planetArray.erase(it);
		else
			++it;
	}
}
