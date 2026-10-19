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

	// Contador numerico ao lado da barra de vida.
	lifeCounter = new Text("ttf/PirataOne-Regular.ttf", "Vida: 100", 28, 0, {255, 255, 255}, 0, 0);
	lifeCounter->setPos(SCREEN_W / 80 + SCREEN_W * 3 / 10 + 14, 6);

	// Painel superior: planetas destruidos e cronometro.
	destroyedText = new Text("ttf/PirataOne-Regular.ttf", "Destruidos: 0", 28, 0, {255, 230, 130}, 0, 0);
	destroyedText->setPos(SCREEN_W / 2 - 300, 6);

	timerText = new Text("ttf/PirataOne-Regular.ttf", "Tempo: 00:00", 28, 0, {255, 230, 130}, 0, 0);
	timerText->setPos(SCREEN_W - 200, 6);

	// Dicas de controle (rodape).
	commandsText = new Text("ttf/PirataOne-Regular.ttf", "W, A, S, D: Move a nave", 24, 0, {255, 255, 255}, 0, 0);
	int textPosY = SCREEN_H - commandsText->getHeight();
	commandsText->setPos(14, textPosY);

	commandsText3 = new Text("ttf/PirataOne-Regular.ttf", "ESC: Pausa   |   +/-: Volume   |   M: Mudo", 24, 0, {255, 255, 255}, 0, 0);
	textPosY = SCREEN_H - commandsText->getHeight() * 2;
	commandsText3->setPos(14, textPosY);

	// Overlay de pausa.
	pauseText = new Text("ttf/PirataOne-Regular.ttf", "PAUSA", 72, 0, {255, 255, 255}, 0, 0);
	int posTextX = SCREEN_W / 2 - pauseText->getWidth() / 2;
	int posTextY = SCREEN_H / 2 - pauseText->getHeight() - 10;
	pauseText->setPos(posTextX, posTextY);

	pauseHint = new Text("ttf/PirataOne-Regular.ttf", "ESC: continuar   |   Q: voltar ao menu", 30, 0, {255, 255, 255}, 0, 0);
	posTextX = SCREEN_W / 2 - pauseHint->getWidth() / 2;
	posTextY = SCREEN_H / 2 + 24;
	pauseHint->setPos(posTextX, posTextY);
}

HUD::~HUD()
{
	delete(lifeCounter);
	delete(destroyedText);
	delete(timerText);
	delete(commandsText);
	delete(commandsText3);
	delete(pauseText);
	delete(pauseHint);
}

void HUD::update(int timeMs, int destroyed)
{
	// Cronometro alinhado a direita (setText so re-renderiza quando muda).
	timerText->setText("Tempo: " + PlayStats::formatTime(timeMs));
	timerText->setPos(SCREEN_W - timerText->getWidth() - 16, 6);

	destroyedText->setText("Destruidos: " + std::to_string(destroyed));
	destroyedText->setPos(SCREEN_W / 2 - destroyedText->getWidth() / 2 - 60, 6);
}

void HUD::render()
{
	displayLife();
	lifeCounter->render();
	destroyedText->render();
	timerText->render();
	commandsText->render();
	commandsText3->render();
}

void HUD::renderPause()
{
	SDLBase::drawOverlay(140);
	pauseText->render();
	pauseHint->render();
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

	if(life < 0)
		life = 0;

	lifeCounter->setText("Vida: " + std::to_string(life));
}
