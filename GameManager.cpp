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

	// Inicializa a terra.
	earthSprite = new Sprite("img/PlanetaTerra.png");
	earthSprite->rotoZoom(0, 3, 3, 1);
	earth = new Earth(earthSprite, 1678, 2588, 100);

	// Inicializa a lua ao redor da terra.
	moonSprite = new Sprite("img/Lua.png");
	moon = new Moon(moonSprite, 1, earth);

	// Inicializa o UFO.
	ufoSprite = new Sprite("img/ufo.png");
	ufoSprite->rotoZoom(0, 3, 3, 0);
	ufo = new FollowerObject(ufoSprite, 960, 435);

	// Inicializa a nave.
	shipAnimation = new Animation("img/NaveSheet.png", 200, 4);
	//shipAnimation->setFrame(1);
	ship = new AccObject(shipAnimation, 0, 0, 94, 100, 20);

	// Inicializa a animação de explosão;
	boom = new Animation("img/BoomSheet.png", 200, 8);
	boomX = 0;
	boomY = 0;

	// Carrega o BG
	bg = new Sprite("img/fundo.png");

	// Cria o TileSet
	tileSet = new TileSet(75, 75, "img/Tileset.png");

	// Cria o TileMap
	tileMap = new TileMap("map/tileMap.txt", tileSet);

	// Carrega o sprite do planeta vermelho.
	redPlanetSprite = new Sprite("img/PlanetaVermelho.png");

	// Atribui a câmera à nave.
	camera->getInstance().bind(ship);
}

GameManager::~GameManager()
{
	delete(bg);
	delete(redPlanetSprite);
	delete(earth);
	delete(earthSprite);
	delete(moon);
	delete(moonSprite);
	planetArray.erase(planetArray.begin(),planetArray.end());
	delete(camera);
	delete(inputManager);
	delete(ufo);
	delete(ufoSprite);
	delete(ship);
	delete(shipAnimation);
	delete(boom);
}

void GameManager::run()
{
	while(!SDL_QuitRequested())
	{
		frameStart = SDL_GetTicks();
		dt = frameStart - frameEnd;

		// Atualização dos objetos
		update(dt);

		// Renderização
		render();

		// Atualizar a tela
		SDLBase::atualizarTela();

		// Delay (p/ 60 fps)
		SDL_Delay(13);

		frameEnd = frameStart;
	}
}

void GameManager::update(int dt)
{
	inputManager->getInstance().update();

	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->update(dt);
	earth->update(dt);
	moon->update(dt);
	if(ufo)
		ufo->update(dt);
	if(ship)
		ship->update(dt);
	boom->update(dt);

	if(inputManager->getInstance().quitGame())
		quit = true;
	if(inputManager->getInstance().isMouseDown(1) && ufo != NULL)
		addPlanet();

	camera->getInstance().update();

	checkCollision(dt);
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
	if(ufo)
	{
		ufo->renderQueueLines(cameraX, cameraY);
		ufo->render(cameraX, cameraY);
	}
	if(ship)
		ship->render(cameraX, cameraY);
	if(boomX != 0 || boomY != 0)
		boom->render(boomX - cameraX, boomY - cameraY);
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
	PlanetRed *planetRed = new PlanetRed(redPlanetSprite, planetX, planetY, 20);
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

void GameManager::checkCollision(int dt)
{
	// Se UFO e Ship existem.
	if(ship && ufo)
	{
		// Testa a colisão da Ship contra cada um dos planetas.
		std::vector<Planet*>::iterator it = planetArray.begin();

		while(it != planetArray.end())
			// Se colidir
			if((*it)->collidesWith(ship))
			{
				// Altera a posição de boomX e boomY para para a posição do planeta.
				boomX = (*it)->x;
				boomY = (*it)->y;

				// Remove o planeta do vector.
				it = planetArray.erase(it);

				// Diminui o hp da Ship em 1.
				ship->hp -= 1;
			}
			else
				++it;


		// Teste a colisão da Ship com o UFO. Caso seja true, defina os hitPoints
		// da Ship como 0.
		if(ship->collidesWith(ufo))
			ship->hp = 0;

		// Teste os hitpoints da Ship. Se for <= 0, o UFO ganhou.
		if(ship->hp <= 0)
		{
			// Defina a posição de boomX e boomY como a posição da Ship.
			boomX = ship->x;
			boomY = ship->y;

			// "Libera" a câmera
			camera->getInstance().unBind();

			// Delete a Ship, e torne seu ponteiro nulo.
			delete(ship);
			ship = NULL;

			// De um update na boom.
			boom->update(dt);

			return;
		}

		// Teste a colisão da Ship com a Earth. Se for true, a Ship ganhou.
		if(ship->collidesWith(earth))
		{
			// Para a nave.
			ship->stop();

			// Defina a posição de boomX e boomY como a posição do UFO.
			boomX = ufo->x;
			boomY = ufo->y;

			// "Libera" a câmera
			camera->getInstance().unBind();

			// Delete o UFO, e torne seu ponteiro nulo.
			delete(ufo);
			ufo = NULL;

			// De um update na boom.
			boom->update(dt);

			return;
		}
	}
}
