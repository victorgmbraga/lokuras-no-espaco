/*
 * StateWinLose.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "StateWinLose.h"
#include "InputManager.h"
#include <string>

void StateWinLose::load(int stack)
{
	bg = NULL;
	txt = NULL;
	statsText = NULL;
	recordText = NULL;
	pressEnter = NULL;
	music = NULL;
	returnState = 0;

	// Qualquer stack inesperado cai em derrota (antes, sem o else,
	// esses casos deixavam TUDO nulo e explodiam ao renderizar).
	bool win = (stack == WIN);

	if(win)
	{
		bg = new Sprite("img/stateWin.jpg");
		txt = new Text("ttf/PirataOne-Regular.ttf", "PARABÉNS AE MANOLO", 34, 0, {255, 255, 255}, 0, 0);
		music = new Audio("sfx/stateWin.ogg", AUDIOTYPE_MUSIC);
	}
	else
	{
		bg = new Sprite("img/stateLose.jpg");
		txt = new Text("ttf/PirataOne-Regular.ttf", "VC MORREU, RSRSRSRS", 34, 0, {255, 255, 255}, 0, 0);
		music = new Audio("sfx/stateLose.ogg", AUDIOTYPE_MUSIC);
	}
	bg->resizeCover(SCREEN_W, SCREEN_H);

	music->play(-1);

	int textPosX = SCREEN_W/2 - txt->getWidth()/2;
	int textPosY = (SCREEN_H/4) * 1;
	txt->setPos(textPosX, textPosY);

	// Painel de estatisticas da partida.
	statsText = new Text("ttf/PirataOne-Regular.ttf",
		"Tempo: " + PlayStats::formatTime(PlayStats::lastTimeMs()) +
		"   |   Destruidos: " + std::to_string(PlayStats::lastDestroyed()) +
		"   |   Pontos: " + std::to_string(PlayStats::lastScore()),
		30, 0, {255, 230, 130}, 0, 0);
	textPosX = SCREEN_W/2 - statsText->getWidth()/2;
	textPosY = SCREEN_H/2 - statsText->getHeight()/2;
	statsText->setPos(textPosX, textPosY);

	recordText = NULL;
	if(PlayStats::beatRecord())
	{
		recordText = new Text("ttf/PirataOne-Regular.ttf",
			"NOVO RECORDE: " + std::to_string(PlayStats::lastScore()) + " pontos",
			34, 0, {130, 255, 160}, 0, 0);
		textPosX = SCREEN_W/2 - recordText->getWidth()/2;
		textPosY = SCREEN_H/2 + 40;
		recordText->setPos(textPosX, textPosY);
	}

	pressEnter = new Text("ttf/PirataOne-Regular.ttf", "Pressione ENTER para jogar de novo", 30, 0, {255, 255, 255}, 0, 0);
	textPosX = SCREEN_W/2 - pressEnter->getWidth()/2;
	textPosY = (SCREEN_H/4) * 3;
	pressEnter->setPos(textPosX, textPosY);
}

int StateWinLose::unload()
{
	delete(bg);
	delete(txt);
	delete(statsText);
	delete(recordText);
	delete(music);
	delete(pressEnter);

	return 0;
}

int StateWinLose::update(int dt)
{
	bg->update(dt);

	InputManager &im = InputManager::getInstance();

	if(im.isKeyDown(SDLK_RETURN))
		returnState = STATE_MENU;

	// ESC tambem volta ao menu.
	if(im.isKeyDown(SDLK_ESCAPE))
		returnState = STATE_MENU;

	if(im.quitGame())
		returnState = STATE_QUIT;

	return returnState;
}

void StateWinLose::render()
{
	bg->render(0,0);
	txt->render();
	statsText->render();

	if(recordText)
		recordText->render();

	pressEnter->render();
}
