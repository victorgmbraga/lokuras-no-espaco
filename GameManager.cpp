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

	// Carrega o sprite do BG
	bg = new Sprite("img/bg.png");

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

		// Todos os comandos de renderização
		bg->render(0,0);
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
	int planetX = rand() % screenWidth;
	int planetY = rand() % screenHeight;

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
        while(SDL_PollEvent(&event))
        {
                // um evento foi achado
                if( event.type == SDL_QUIT )
                        SDL_Quit();
                if( event.type == SDL_MOUSEBUTTONDOWN )
                {
                        // obtém o estado do mouse
                        int mouseX, mouseY;
                        SDL_GetMouseState( &mouseX, &mouseY );

                        // percorre o array de monstros de forma invertida
                        // até achar o monstro que o usuário clicou.
                        // percorremos de forma invertida porque os últimos
                        // monstros são desenhados no topo, e estes serão
                        // atingidos primeiro
                        for(std::vector<Planet*>::iterator it = planetArray.end() - 1;
                                        it != planetArray.begin() - 1; --it)
                        {
                                Planet *planet = *it;

                                if( mouseX >= planet->getX() &&
                                                mouseX < planet->getX() + planet->getSprite()->getClip().w &&
                                                mouseY >= planet->getY() &&
                                                mouseY < planet->getY() + planet->getSprite()->getClip().h )
                                {
                                        // acerta o planeta, dando dano aleatório entre 10 e 19

                                        planet->setHP(planet->getHP() - (rand() % 10 + 10));

                                        // acertamos apenas um monstro
                                        // portanto saímos do break
                                        break;
                                }
                        }
                }
                if( event.type == SDL_KEYDOWN )
                {
                        // uma tecla foi pressionada
                        if( event.key.keysym.sym == SDLK_ESCAPE )
                        {
                                // se a tecla ESC foi pressionada, sair do programa
                                SDL_Event quit;
                                quit.type = SDL_QUIT;
                                SDL_PushEvent( &quit );
                        }
                        else
                        {
                                // caso qualquer outra tecla seja pressionada,
                                // chamar a rotina de adicionar monstro
                                addPlanet();
                        }
                }
        }
        // fim do loop de eventos
}


