/*
 * StateWinLose.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "StateWinLose.h"

void StateWinLose::load(int stack)
{
	if(stack == LOSE)
	{
		bg = new Sprite("img/stateLose.jpg");
		txt = new Text("ttf/PirataOne-Regular.ttf", "VC MORREU, RSRSRSRS", 28, 0, {255, 255, 255}, 0, 0);
		music = new Audio("sfx/stateLose.ogg", AUDIOTYPE_MUSIC);
	}
	else if(stack == WIN)
	{
		bg = new Sprite("img/stateWin.jpg");
		txt = new Text("ttf/PirataOne-Regular.ttf", "PARABÉNS AE MANOLO", 28, 0, {255, 255, 255}, 0, 0);
		music = new Audio("sfx/stateWin.mp3", AUDIOTYPE_MUSIC);
	}

	music->play(-1);
	returnState = 0;

	int textPosX = SCREEN_W/2 - txt->getWidth()/2;
	int textPosY = (SCREEN_H/4) * 1;
	txt->setPos(textPosX, textPosY);

	pressEnter = new Text("ttf/PirataOne-Regular.ttf", "Pressione ENTER para jogar de novo", 24, 0, {255, 255, 255}, 0, 0);
	textPosX = SCREEN_W/2 - pressEnter->getWidth()/2;
	textPosY = (SCREEN_H/4) * 3;
	pressEnter->setPos(textPosX, textPosY);

}

int StateWinLose::unload()
{
	delete(bg);
	delete(txt);
	delete(music);
	delete(pressEnter);

	return 0;
}

int StateWinLose::update(int dt)
{
	bg->update(dt);

	if(inputManager->getInstance().isKeyDown(SDLK_RETURN))
		returnState = 1;

	if(inputManager->getInstance().quitGame())
		returnState = -1;

	return returnState;
}

void StateWinLose::render()
{
	bg->render(0,0);
	txt->render();
	pressEnter->render();
}
