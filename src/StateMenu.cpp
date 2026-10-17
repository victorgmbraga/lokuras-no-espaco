/*
 * StateMenu.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "StateMenu.h"
#include "InputManager.h"
#include <cmath>

void StateMenu::load(int stack)
{
	// Recorde salvo de partidas anteriores (arquivo lokuras.rec).
	PlayStats::load();

	bg = new Sprite("img/stateMenu.jpg");
	bg->resizeCover(SCREEN_W, SCREEN_H);
	pressEnter = new Text("ttf/PirataOne-Regular.ttf", "Pressione ENTER para jogar", 30, 0, {255, 255, 255}, 0, 0);
	title =  new Text("ttf/SpaceAge.ttf", "LOKURAS NO ESPAÇO", 60, 0, {255, 255, 255}, 0, 0);
	music = new Audio("sfx/stateMenu.ogg", AUDIOTYPE_MUSIC);
	music->play(-1);

	returnState = 0;
	menuTime = 0;

	int textPosX = SCREEN_W/2 - pressEnter->getWidth()/2;
	int textPosY = (SCREEN_H/4) * 3;
	pressEnter->setPos(textPosX, textPosY);

	titleX = SCREEN_W/2 - title->getWidth()/2;
	titleY = (SCREEN_H/4) * 1;
	title->setPos(titleX, titleY);

	// Mostra o recorde, se houver.
	recordText = NULL;
	if(PlayStats::bestScore() >= 0)
	{
		recordText = new Text("ttf/PirataOne-Regular.ttf",
			"Recorde: " + std::to_string(PlayStats::bestScore()) + " pontos",
			30, 0, {255, 230, 130}, 0, 0);
		textPosX = SCREEN_W/2 - recordText->getWidth()/2;
		textPosY = (SCREEN_H/4) * 3 + pressEnter->getHeight() + 12;
		recordText->setPos(textPosX, textPosY);
	}
}

int StateMenu::unload()
{
	delete(bg);
	delete(pressEnter);
	delete(title);
	delete(recordText);
	delete(music);

	return 0;
}

int StateMenu::update(int dt)
{
	bg->update(dt);

	// Swing suave do titulo (menu "vivo").
	menuTime += dt;

	InputManager &im = InputManager::getInstance();

	if(im.isKeyDown(SDLK_RETURN))
		returnState = STATE_GAME;

	// ESC encerra o jogo (acessivel de qualquer estado).
	if(im.isKeyDown(SDLK_ESCAPE))
		returnState = STATE_QUIT;

	if(im.quitGame())
		returnState = STATE_QUIT;

	return returnState;
}

void StateMenu::render()
{
	bg->render(0,0);

	title->setPos(titleX, titleY + sin(menuTime * 0.002f) * 6);
	title->render();

	pressEnter->render();

	if(recordText)
		recordText->render();
}
