/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateGame.h"

void StateGame::load(int stack)
{
	// Câmera e Input Manager
	camera = NULL;
	inputManager = NULL;

	// Terra
	earthSprite = new Sprite("img/PlanetaTerra.png");
	earthSprite->rotoZoom(0, 3, 3, 0);
	earth = new Earth(earthSprite, 1000, 1600, 100);

	// Lua
	moonSprite = new Sprite("img/Lua.png");
	moon = new Moon(moonSprite, 1, earth);

	// UFO
	ufoSprite = new Sprite("img/ufo.png");
	ufoSprite->rotoZoom(0, 3, 3, 0);
	ufo = new FollowerObject(ufoSprite, 960, 435);

	// Nave
	shipAnimation = new Animation("img/NaveSheet.png", 200, 4);
	ship = new AccObject(shipAnimation, 0, 0, 94, 100, 100);

	// Explosão
	boom = new Animation("img/BoomSheet.png", 200, 8);
	boomX = 0;
	boomY = 0;

	// Background
	bg = new Sprite("img/fundo.png");
	bg->resizeCover(SCREEN_W, SCREEN_H);

	// TileSet e TileMap
	tileSet = new TileSet(75, 75, "img/Tileset.png");
	tileMap = new TileMap("map/tileMap.txt", tileSet);

	// Planetas Vermelhos
	redPlanetSprite = new Sprite("img/PlanetaVermelho.png");
	redPlanetCoolDown = new Timer();
	redPlanetCoolDown->start();

	// Atribui a câmera à nave.
	camera->getInstance().bind(ship);

	// Música e sons
	sfx = new Audio("sfx/boom.wav", AUDIOTYPE_SOUND);
	music = new Audio("sfx/stateGame.ogg", AUDIOTYPE_MUSIC);
	music->play(-1);

	// HUD
	hud = new HUD();

	// Variáveis de retorno
	returnState = 0;
	returnStack = 0;
}

int StateGame::unload()
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
	delete(hud);
	return returnStack;
}

int StateGame::update(int dt)
{
	// Planetas vermelhos
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->update(dt);

	// Terra
	earth->update(dt);

	// Lua
	moon->update(dt);

	// UFO
	if(ufo)
		ufo->update(dt);

	// Nave
	if(ship)
		ship->update(dt);

	// Explosão
	boom->update(dt);

	// Verifica se deve sair do jogo
	if(inputManager->getInstance().quitGame())
		returnState = -1;

	// Adiciona os planetas vermelhos ao pressionar o mouse
	if(inputManager->getInstance().isMouseDown(1) && ufo != NULL)
		addPlanet();

	// Câmera
	camera->getInstance().update();

	// Colisões
	checkCollision(dt);

	return returnState;
}

void StateGame::render()
{
	float cameraX = camera->getInstance().getX();
	float cameraY = camera->getInstance().getY();

	// Background
	bg->render(0,0);

	// Tilemap
	tileMap->render(cameraX, cameraY);

	// Planetas vermelhos
	for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
		(*it)->render(cameraX, cameraY);

	// Terra
	earth->render(cameraX, cameraY);

	// Lua
	moon->render(cameraX, cameraY);

	// UFO e suas linhas
	if(ufo)
	{
		ufo->renderQueueLines(cameraX, cameraY);
		ufo->render(cameraX, cameraY);
	}

	// Nave
	if(ship)
		ship->render(cameraX, cameraY);

	// Explosão
	if(boomX != 0 || boomY != 0 || !ship)
		boom->render(boomX - cameraX, boomY - cameraY);

	// Texto
	hud->render();
}

void StateGame::addPlanet()
{
	if(redPlanetCoolDown->getTime() > 3000)
	{
		redPlanetCoolDown->start();

		// Pega o tamanho da tela
		int screenWidth = SCREEN_W;
		int screenHeight = SCREEN_H;

		// Cria uma posição aleatória dentro da tela
		int planetX = camera->getInstance().getX() + (rand() % screenWidth);
		int planetY = camera->getInstance().getY() + (rand() % screenHeight);

		// Adiciona o planeta na posição
		PlanetRed *planetRed = new PlanetRed(redPlanetSprite, planetX, planetY, 20);
		planetArray.push_back(planetRed);
	}
}

void StateGame::checkCollision(int dt)
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

				// Diminui o hp da Ship em 5.
				ship->hp -= 5;

				// Muda a vida no HUD
				hud->setLife(ship->hp);

				// Toca o som da explosão
				sfx->play(0);
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

			// Toca o som de explosão
			sfx->play(0);

			// Chama a tela de derrota
			returnState = 3;
			returnStack = 0;

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

			// Chama a tela de vitória
			returnState = 3;
			returnStack = 1;

			return;
		}
	}
}
