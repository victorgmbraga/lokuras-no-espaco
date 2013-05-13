/*
 * GameManager.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "GameManager.h"

GameManager::GameManager (void)
{
	SDLBase::inicializaSDL();

	quit = false;
	cameraX = 0;
	cameraY = 0;
	cameraSpeedX = 0;
	cameraSpeedY = 0;

	// Carrega o BG
	bg = new Sprite("img/bg.png");

	// Cria o TileSet
	tileSet = new TileSet(75, 75);
	loadTiles();

	// Cria o TileMap
	tileMap = new TileMap(16, 16, 1, tileSet);
	buildMap();

	// Carrega o sprite do planeta
	planetSprite = new Sprite("img/planet.png");
}

GameManager::~GameManager (void)
{
	delete(bg);
	delete(planetSprite);
	planetArray.erase(planetArray.begin(),planetArray.end());
}

void GameManager::run (void)
{
	while(!SDL_QuitRequested())
	{
		// Captura de Input
		processEvents();

		// Atualização dos objetos
		checkPlanets();
		cameraX += cameraSpeedX;
		cameraY += cameraSpeedY;

		// Todos os comandos de renderização
		bg->render(0,0);
		tileMap->render(cameraX, cameraY);
		for(std::vector<Planet*>::iterator it = planetArray.begin(); it != planetArray.end(); ++it)
			(*it)->render();


		// Atualizar a tela
		SDLBase::atualizarTela();
	}
}

void GameManager::addPlanet()
{
	// Pega o tamanho da tela
	int screenWidth = bg->getWidth();
	int screenHeight = bg->getHeight();

	// Cria uma posição aleatória dentro da tela
	int planetX = cameraX + (rand() % screenWidth);
	int planetY = cameraY + (rand() % screenHeight);

	// Adiciona o planeta na posição
	Planet *planet = new Planet(planetSprite, 20, planetX, planetY);
	planetArray.push_back(planet);
}

void GameManager::checkPlanets()
{
	std::vector<Planet*>::iterator it = planetArray.begin();

	// Deleta os planetas com HP <= 0
	while(it != planetArray.end())
	{
		if((*it)->getHP() <= 0)
			it = planetArray.erase(it);
		else
			++it;
	}
}

void GameManager::processEvents()
{
	// procura um evento
	SDL_Event event;
	while( SDL_PollEvent( &event ) && !quit ) // inicializar um atributo quit do tipo bool com o valor false
	{
		// um evento foi achado
		if( event.type == SDL_QUIT )
			quit = true;
		else if( event.type == SDL_MOUSEBUTTONDOWN )
		{
			// obtém o estado do mouse
			int mouseX, mouseY;
			SDL_GetMouseState( &mouseX, &mouseY );

			// percorre o array de planetas de forma invertida
			// até achar o planeta que o usuário clicou.
			// percorremos de forma invertida porque os últimos
			// planetas são desenhados no topo, e estes serão
			// atingidos primeiro
			for(std::vector<Planet*>::iterator it = planetArray.end() - 1;
					it != planetArray.begin() - 1; --it)
			{
				Planet *planet = *it;

				if( mouseX + cameraX >= planet->getX() &&
					mouseX + cameraX < planet->getX() + planet->getSprite()->getClip().w &&
					mouseY + cameraY >= planet->getY() &&
					mouseY + cameraY < planet->getY() + planet->getSprite()->getClip().h )
				{
					// acerta o planeta, dando dano aleatório entre 10 e 19

					planet->setHP(planet->getHP() - (rand() % 10 + 10));

					// acertamos apenas um planeta
					// portanto saímos do break
					break;
				}
			}
		}
		else if( event.type == SDL_KEYDOWN )
		{
			// uma tecla foi pressionada
			if( event.key.keysym.sym == SDLK_ESCAPE )
			{
				// se a tecla ESC foi pressionada, sair do programa
				quit = true;
			}
			// Determina as variÃ¡veis de velocidade da cÃ¢mera
			// de acordo com as teclas pressionadas
			else if( event.key.keysym.sym == SDLK_UP )
				cameraSpeedY -= SCROLL;
			else if( event.key.keysym.sym == SDLK_DOWN )
				cameraSpeedY += SCROLL;
			else if( event.key.keysym.sym == SDLK_LEFT )
				cameraSpeedX -= SCROLL;
			else if( event.key.keysym.sym == SDLK_RIGHT )
				cameraSpeedX += SCROLL;
			else if( event.key.keysym.sym == SDLK_SPACE )
			{
				addPlanet();
			}
		}
		else if( event.type == SDL_KEYUP )
		{
			// uma tecla foi solta

			// diminui as variÃ¡veis de velocidade da cÃ¢mera
			// para que elas voltam a ser zero caso uma tecla
			// tenha sido solta
			if( event.key.keysym.sym == SDLK_UP )
				cameraSpeedY += SCROLL;
			else if( event.key.keysym.sym == SDLK_DOWN )
				cameraSpeedY -= SCROLL;
			else if( event.key.keysym.sym == SDLK_LEFT )
				cameraSpeedX += SCROLL;
			else if( event.key.keysym.sym == SDLK_RIGHT )
				cameraSpeedX -= SCROLL;
		}
	}
	// fim do loop de eventos
}

void GameManager::loadTiles (void)
{
	tileSet->addTile("img/tileset/SpaceTile-01.png");
	tileSet->addTile("img/tileset/SpaceTile-02.png");
	tileSet->addTile("img/tileset/SpaceTile-03.png");
	tileSet->addTile("img/tileset/SpaceTile-04.png");
	tileSet->addTile("img/tileset/SpaceTile-05.png");
	tileSet->addTile("img/tileset/SpaceTile-06.png");
	tileSet->addTile("img/tileset/SpaceTile-07.png");
	tileSet->addTile("img/tileset/SpaceTile-08.png");
	tileSet->addTile("img/tileset/SpaceTile-09.png");
	tileSet->addTile("img/tileset/SpaceTile-10.png");
	tileSet->addTile("img/tileset/SpaceTile-11.png");
	tileSet->addTile("img/tileset/SpaceTile-12.png");
	tileSet->addTile("img/tileset/SpaceTile-13.png");
	tileSet->addTile("img/tileset/SpaceTile-14.png");
	tileSet->addTile("img/tileset/SpaceTile-15.png");
	tileSet->addTile("img/tileset/SpaceTile-16.png");
	tileSet->addTile("img/tileset/SpaceTile-17.png");
	tileSet->addTile("img/tileset/SpaceTile-18.png");
	tileSet->addTile("img/tileset/SpaceTile-19.png");
	tileSet->addTile("img/tileset/SpaceTile-20.png");
	tileSet->addTile("img/tileset/SpaceTile-21.png");
	tileSet->addTile("img/tileset/SpaceTile-22.png");
	tileSet->addTile("img/tileset/SpaceTile-23.png");
	tileSet->addTile("img/tileset/SpaceTile-24.png");
	tileSet->addTile("img/tileset/SpaceTile-25.png");
	tileSet->addTile("img/tileset/SpaceTile-26.png");
	tileSet->addTile("img/tileset/SpaceTile-27.png");
	tileSet->addTile("img/tileset/SpaceTile-28.png");
	tileSet->addTile("img/tileset/SpaceTile-29.png");
	tileSet->addTile("img/tileset/SpaceTile-30.png");
}

void GameManager::buildMap (void)
{
	tileMap->at( 0, 0) = 0;
	tileMap->at( 1, 0) = 1;
	tileMap->at( 2, 0) = 1;
	tileMap->at( 3, 0) = 1;
	tileMap->at( 4, 0) = 1;
	tileMap->at( 5, 0) = 1;
	tileMap->at( 6, 0) = 1;
	tileMap->at( 7, 0) = 1;
	tileMap->at( 8, 0) = 1;
	tileMap->at( 9, 0) = 1;
	tileMap->at(10, 0) = 1;
	tileMap->at(11, 0) = 1;
	tileMap->at(12, 0) = 5;
	tileMap->at(13, 0) = -1;
	tileMap->at(14, 0) = -1;
	tileMap->at(15, 0) = -1;
	tileMap->at( 0, 1) = 6;
	tileMap->at( 1, 1) = 10;
	tileMap->at( 2, 1) = 10;
	tileMap->at( 3, 1) = 10;
	tileMap->at( 4, 1) = 10;
	tileMap->at( 5, 1) = 10;
	tileMap->at( 6, 1) = 10;
	tileMap->at( 7, 1) = 10;
	tileMap->at( 8, 1) = 10;
	tileMap->at( 9, 1) = 10;
	tileMap->at(10, 1) = 10;
	tileMap->at(11, 1) = 10;
	tileMap->at(12, 1) = 11;
	tileMap->at(13, 1) = -1;
	tileMap->at(14, 1) = -1;
	tileMap->at(15, 1) = -1;
	tileMap->at( 0, 2) = 12;
	tileMap->at( 1, 2) = 7;
	tileMap->at( 2, 2) = 8;
	tileMap->at( 3, 2) = 8;
	tileMap->at( 4, 2) = 8;
	tileMap->at( 5, 2) = 8;
	tileMap->at( 6, 2) = 8;
	tileMap->at( 7, 2) = 8;
	tileMap->at( 8, 2) = 9;
	tileMap->at( 9, 2) = 10;
	tileMap->at(10, 2) = 10;
	tileMap->at(11, 2) = 10;
	tileMap->at(12, 2) = 17;
	tileMap->at(13, 2) = -1;
	tileMap->at(14, 2) = -1;
	tileMap->at(15, 2) = -1;
	tileMap->at( 0, 3) = 6;
	tileMap->at( 1, 3) = 13;
	tileMap->at( 2, 3) = 14;
	tileMap->at( 3, 3) = 14;
	tileMap->at( 4, 3) = 14;
	tileMap->at( 5, 3) = 14;
	tileMap->at( 6, 3) = 14;
	tileMap->at( 7, 3) = 14;
	tileMap->at( 8, 3) = 15;
	tileMap->at( 9, 3) = 10;
	tileMap->at(10, 3) = 10;
	tileMap->at(11, 3) = 10;
	tileMap->at(12, 3) = 11;
	tileMap->at(13, 3) = -1;
	tileMap->at(14, 3) = -1;
	tileMap->at(15, 3) = -1;
	tileMap->at( 0, 4) = 12;
	tileMap->at( 1, 4) = 13;
	tileMap->at( 2, 4) = 14;
	tileMap->at( 3, 4) = 14;
	tileMap->at( 4, 4) = 14;
	tileMap->at( 5, 4) = 14;
	tileMap->at( 6, 4) = 14;
	tileMap->at( 7, 4) = 14;
	tileMap->at( 8, 4) = 15;
	tileMap->at( 9, 4) = 7;
	tileMap->at(10, 4) = 25;
	tileMap->at(11, 4) = 28;
	tileMap->at(12, 4) = 29;
	tileMap->at(13, 4) = -1;
	tileMap->at(14, 4) = -1;
	tileMap->at(15, 4) = -1;
	tileMap->at( 0, 5) = 6;
	tileMap->at( 1, 5) = 13;
	tileMap->at( 2, 5) = 14;
	tileMap->at( 3, 5) = 14;
	tileMap->at( 4, 5) = 14;
	tileMap->at( 5, 5) = 14;
	tileMap->at( 6, 5) = 14;
	tileMap->at( 7, 5) = 14;
	tileMap->at( 8, 5) = 15;
	tileMap->at( 9, 5) = 19;
	tileMap->at(10, 5) = 3;
	tileMap->at(11, 5) = 4;
	tileMap->at(12, 5) = 5;
	tileMap->at(13, 5) = -1;
	tileMap->at(14, 5) = -1;
	tileMap->at(15, 5) = -1;
	tileMap->at( 0, 6) = 6;
	tileMap->at( 1, 6) = 13;
	tileMap->at( 2, 6) = 14;
	tileMap->at( 3, 6) = 14;
	tileMap->at( 4, 6) = 14;
	tileMap->at( 5, 6) = 14;
	tileMap->at( 6, 6) = 14;
	tileMap->at( 7, 6) = 14;
	tileMap->at( 8, 6) = 15;
	tileMap->at( 9, 6) = 10;
	tileMap->at(10, 6) = 10;
	tileMap->at(11, 6) = 10;
	tileMap->at(12, 6) = 11;
	tileMap->at(13, 6) = -1;
	tileMap->at(14, 6) = -1;
	tileMap->at(15, 6) = -1;
	tileMap->at( 0, 7) = 12;
	tileMap->at( 1, 7) = 13;
	tileMap->at( 2, 7) = 14;
	tileMap->at( 3, 7) = 14;
	tileMap->at( 4, 7) = 14;
	tileMap->at( 5, 7) = 14;
	tileMap->at( 6, 7) = 14;
	tileMap->at( 7, 7) = 14;
	tileMap->at( 8, 7) = 15;
	tileMap->at( 9, 7) = 10;
	tileMap->at(10, 7) = 10;
	tileMap->at(11, 7) = 10;
	tileMap->at(12, 7) = 17;
	tileMap->at(13, 7) = -1;
	tileMap->at(14, 7) = -1;
	tileMap->at(15, 7) = -1;
	tileMap->at( 0, 8) = 6;
	tileMap->at( 1, 8) = 19;
	tileMap->at( 2, 8) = 20;
	tileMap->at( 3, 8) = 20;
	tileMap->at( 4, 8) = 20;
	tileMap->at( 5, 8) = 20;
	tileMap->at( 6, 8) = 20;
	tileMap->at( 7, 8) = 20;
	tileMap->at( 8, 8) = 21;
	tileMap->at( 9, 8) = 10;
	tileMap->at(10, 8) = 10;
	tileMap->at(11, 8) = 10;
	tileMap->at(12, 8) = 11;
	tileMap->at(13, 8) = -1;
	tileMap->at(14, 8) = -1;
	tileMap->at(15, 8) = -1;
	tileMap->at( 0, 9) = 24;
	tileMap->at( 1, 9) = 25;
	tileMap->at( 2, 9) = 25;
	tileMap->at( 3, 9) = 25;
	tileMap->at( 4, 9) = 25;
	tileMap->at( 5, 9) = 25;
	tileMap->at( 6, 9) = 25;
	tileMap->at( 7, 9) = 25;
	tileMap->at( 8, 9) = 25;
	tileMap->at( 9, 9) = 26;
	tileMap->at(10, 9) = 26;
	tileMap->at(11, 9) = 26;
	tileMap->at(12, 9) = 29;
	tileMap->at(13, 9) = -1;
	tileMap->at(14, 9) = -1;
	tileMap->at(15, 9) = -1;
	tileMap->at( 0,10) = -1;
	tileMap->at( 1,10) = -1;
	tileMap->at( 2,10) = -1;
	tileMap->at( 3,10) = -1;
	tileMap->at( 4,10) = -1;
	tileMap->at( 5,10) = -1;
	tileMap->at( 6,10) = -1;
	tileMap->at( 7,10) = -1;
	tileMap->at( 8,10) = -1;
	tileMap->at( 9,10) = -1;
	tileMap->at(10,10) = -1;
	tileMap->at(11,10) = -1;
	tileMap->at(12,10) = -1;
	tileMap->at(13,10) = -1;
	tileMap->at(14,10) = -1;
	tileMap->at(15,10) = -1;
	tileMap->at( 0,11) = 0;
	tileMap->at( 1,11) = 1;
	tileMap->at( 2,11) = 1;
	tileMap->at( 3,11) = 5;
	tileMap->at( 4,11) = -1;
	tileMap->at( 5,11) = 0;
	tileMap->at( 6,11) = 2;
	tileMap->at( 7,11) = 3;
	tileMap->at( 8,11) = 4;
	tileMap->at( 9,11) = 5;
	tileMap->at(10,11) = -1;
	tileMap->at(11,11) = -1;
	tileMap->at(12,11) = -1;
	tileMap->at(13,11) = -1;
	tileMap->at(14,11) = -1;
	tileMap->at(15,11) = -1;
	tileMap->at( 0,12) = 6;
	tileMap->at( 1,12) = 7;
	tileMap->at( 2,12) = 28;
	tileMap->at( 3,12) = 29;
	tileMap->at( 4,12) = -1;
	tileMap->at( 5,12) = 12;
	tileMap->at( 6,12) = 10;
	tileMap->at( 7,12) = 10;
	tileMap->at( 8,12) = 10;
	tileMap->at( 9,12) = 17;
	tileMap->at(10,12) = -1;
	tileMap->at(11,12) = -1;
	tileMap->at(12,12) = -1;
	tileMap->at(13,12) = 0;
	tileMap->at(14,12) = 5;
	tileMap->at(15,12) = -1;
	tileMap->at( 0,13) = 18;
	tileMap->at( 1,13) = 17;
	tileMap->at( 2,13) = -1;
	tileMap->at( 3,13) = -1;
	tileMap->at( 4,13) = -1;
	tileMap->at( 5,13) = 6;
	tileMap->at( 6,13) = 10;
	tileMap->at( 7,13) = 10;
	tileMap->at( 8,13) = 10;
	tileMap->at( 9,13) = 11;
	tileMap->at(10,13) = -1;
	tileMap->at(11,13) = -1;
	tileMap->at(12,13) = -1;
	tileMap->at(13,13) = 24;
	tileMap->at(14,13) = 29;
	tileMap->at(15,13) = -1;
	tileMap->at( 0,14) = 24;
	tileMap->at( 1,14) = 29;
	tileMap->at( 2,14) = -1;
	tileMap->at( 3,14) = -1;
	tileMap->at( 4,14) = -1;
	tileMap->at( 5,14) = 24;
	tileMap->at( 6,14) = 25;
	tileMap->at( 7,14) = 27;
	tileMap->at( 8,14) = 28;
	tileMap->at( 9,14) = 29;
	tileMap->at(10,14) = -1;
	tileMap->at(11,14) = -1;
	tileMap->at(12,14) = -1;
	tileMap->at(13,14) = -1;
	tileMap->at(14,14) = -1;
	tileMap->at(15,14) = -1;
	// A linha 15 tem apenas -1, nao é necessário preenche-la, pois
	// o mapa é inicializado com -1
}
