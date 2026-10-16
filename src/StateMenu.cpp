/*
 * StateMenu.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "StateMenu.h"

void StateMenu::load(int stack)
{
	bg = new Sprite("img/stateMenu.jpg");
	pressEnter = new Text("ttf/PirataOne-Regular.ttf", "Pressione ENTER para jogar", 24, 0, {255, 255, 255}, 0, 0);
	title =  new Text("ttf/SpaceAge.ttf", "LOKURAS NO ESPAÇO", 48, 0, {255, 255, 255}, 0, 0);
	music = new Audio("sfx/stateMenu.ogg", AUDIOTYPE_MUSIC);
	music->play(-1);
	returnState = 0;

	int textPosX = SCREEN_W/2 - pressEnter->getWidth()/2;
	int textPosY = (SCREEN_H/4) * 3;
	pressEnter->setPos(textPosX, textPosY);

	textPosX = SCREEN_W/2 - title->getWidth()/2;
	textPosY = (SCREEN_H/4) * 1;
	title->setPos(textPosX, textPosY);
}

int StateMenu::unload()
{
	delete(bg);
	delete(pressEnter);
	delete(title);
	delete(music);

	return 0;
}

int StateMenu::update(int dt)
{
	bg->update(dt);

	if(inputManager->getInstance().isKeyDown(SDLK_RETURN))
		returnState = 2;

	if(inputManager->getInstance().quitGame())
		returnState = -1;

	return returnState;
}

void StateMenu::render()
{
	bg->render(0,0);
	pressEnter->render();
	title->render();
}
