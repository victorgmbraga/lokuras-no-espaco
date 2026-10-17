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

	lifeCounter = new Text("ttf/PirataOne-Regular.ttf", "Vida: 20", 28, 0, {255, 100, 100}, 0, 0);

	commandsText = new Text("ttf/PirataOne-Regular.ttf", "W, A, S, D: Move a nave", 24, 0, {255, 255, 255}, 0, 0);
	int textPosY = SCREEN_H - commandsText->getHeight();
	commandsText->setPos(0, textPosY);

	commandsText2 = new Text("ttf/PirataOne-Regular.ttf", "Mouse1: Adiciona Planeta", 24, 0, {255, 255, 255}, 0, 0);
	textPosY = SCREEN_H - commandsText->getHeight() * 3;
	commandsText2->setPos(0, textPosY);

	commandsText3 = new Text("ttf/PirataOne-Regular.ttf", "Mouse2: Comanda UFO", 24, 0, {255, 255, 255}, 0, 0);
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
	int maxWidth = SCREEN_W * 3 / 10;
	int barHeight = SCREEN_H / 30;
	int barWidth = maxWidth * life / 100;

	if(barWidth < 0)
		barWidth = 0;
	if(barWidth > maxWidth)
		barWidth = maxWidth;

	int color = (life > 20) ? 0x00EA00 : 0xEA0000;
	SDLBase::drawRect(SCREEN_W / 80, SCREEN_H / 60, barWidth, barHeight, color);
}

void HUD::setLife(int life)
{
	this->life = life;
}
