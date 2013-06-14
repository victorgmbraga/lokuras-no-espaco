/*
 * HUD.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "HUD.h"

HUD::HUD()
{
	life = 100;

	lifeCounter = new Text("ttf/PirataOne-Regular.ttf", "Vida: 20", 24, 0, {255, 100, 100}, 0, 0);

	commandsText = new Text("ttf/PirataOne-Regular.ttf", "W, A, S, D: Move a nave", 20, 0, {255, 255, 255}, 0, 0);
	int textPosY = SCREEN_H - commandsText->getHeight();
	commandsText->setPos(0, textPosY);

	commandsText2 = new Text("ttf/PirataOne-Regular.ttf", "Mouse1: Adiciona Planeta", 20, 0, {255, 255, 255}, 0, 0);
	textPosY = SCREEN_H - commandsText->getHeight() * 3;
	commandsText2->setPos(0, textPosY);

	commandsText3 = new Text("ttf/PirataOne-Regular.ttf", "Mouse2: Comanda UFO", 20, 0, {255, 255, 255}, 0, 0);
	textPosY = SCREEN_H - commandsText->getHeight() * 2;
	commandsText3->setPos(0, textPosY);
}

HUD::~HUD()
{
	delete(commandsText);
	delete(commandsText2);
	delete(commandsText3);
	delete(lifeCounter);
}

void HUD::update()
{

}

void HUD::render()
{
	commandsText->render();
	commandsText2->render();
	commandsText3->render();
	displayLife();
}

void HUD::displayLife()
{
	if(life >= 100)
		SDLBase::drawRect(10, 10, 240, 20, 0x00EA00);
	else if(life > 80)
		SDLBase::drawRect(10, 10, 200, 20, 0x00EA00);
	else if(life > 60)
		SDLBase::drawRect(10, 10, 160, 20, 0x00EA00);
	else if(life > 40)
		SDLBase::drawRect(10, 10, 120, 20, 0x00EA00);
	else if(life > 20)
		SDLBase::drawRect(10, 10, 80, 20, 0xEA0000);
	else if(life > 0)
		SDLBase::drawRect(10, 10, 40, 20, 0xEA0000);
}

void HUD::setLife(int life)
{
	this->life = life;
}
