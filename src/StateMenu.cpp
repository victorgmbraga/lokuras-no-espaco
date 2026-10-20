/*
 * StateMenu.cpp
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#include "StateMenu.h"
#include "GameSettings.h"
#include "InputManager.h"
#include <cmath>

void StateMenu::load(int stack)
{
	// Recordes, contadores e conquistas salvos de partidas anteriores.
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

	// Selecao de dificuldade (1/2/3) e recorde correspondente.
	difficultyText = new Text("ttf/PirataOne-Regular.ttf", "Dificuldade", 30, 0, {255, 255, 255}, 0, 0);
	difficultyText->setPos(0, (SCREEN_H/4) * 3 - 95);

	// O que muda em cada dificuldade (atualizado no refreshTexts).
	diffDesc = new Text("ttf/PirataOne-Regular.ttf", "", 22, 0, {180, 210, 180}, 0, 0);

	recordText = new Text("ttf/PirataOne-Regular.ttf", "Recorde", 26, 0, {255, 230, 130}, 0, 0);

	hintText = new Text("ttf/PirataOne-Regular.ttf",
		"?seed=N ou ?daily=1 no navegador: mapa e posicoes iguais para todos",
		22, 0, {150, 160, 190}, 0, 0);
	hintText->setPos(SCREEN_W/2 - hintText->getWidth()/2, SCREEN_H - 30);

	// Lista de conquistas (coluna esquerda).
	for(int i = 0; i < MENU_ACH_LINHAS; ++i)
	{
		achTexts[i] = new Text("ttf/PirataOne-Regular.ttf", "", 22, 0, {180, 200, 230}, 0, 0);
		achTexts[i]->setPos(24, 90 + i * 30);
	}

	shownDifficulty = -1; // força o primeiro refresh
	refreshTexts();
}

void StateMenu::refreshTexts()
{
	int diff = GameSettings::difficulty();
	shownDifficulty = diff;

	difficultyText->setText(std::string("Dificuldade: ") + GameSettings::difficultyName(diff) + "   (1 = facil, 2 = normal, 3 = dificil)");
	difficultyText->setPos(SCREEN_W/2 - difficultyText->getWidth()/2, (SCREEN_H/4) * 3 - 95);

	// Resumo honesto do que cada dificuldade muda (spawn, chefe, torres, tiro, combustivel).
	if(diff == DIF_FACIL)
		diffDesc->setText("Facil: UFO lento e fraco, 2 torres, tiro inimigo fraco, mais combustivel");
	else if(diff == DIF_DIFICIL)
		diffDesc->setText("Dificil: UFO rapido e forte, 6 torres, tiro inimigo forte, pouco combustivel");
	else
		diffDesc->setText("Normal: desafio equilibrado, 4 torres, tiro e combustivel padrao");
	diffDesc->setPos(SCREEN_W/2 - diffDesc->getWidth()/2, (SCREEN_H/4) * 3 - 55);

	int best = PlayStats::bestScore(diff);
	recordText->setText(best >= 0 ? "Recorde nessa dificuldade: " + std::to_string(best) + " pontos"
	                              : "Sem recorde nessa dificuldade ainda");
	recordText->setPos(SCREEN_W/2 - recordText->getWidth()/2, (SCREEN_H/4) * 3 + pressEnter->getHeight() + 14);

	for(int i = 0; i < MENU_ACH_LINHAS; ++i)
	{
		achTexts[i]->setText(std::string(PlayStats::ach(i) ? "[X] " : "[ ] ") + PlayStats::achName(i));

		// Conquistadas ficam verdes, pendentes em cinza.
		SDL_Color cor;
		if(PlayStats::ach(i))
		{
			cor.r = 130; cor.g = 255; cor.b = 160;
		}
		else
		{
			cor.r = 150; cor.g = 155; cor.b = 175;
		}
		achTexts[i]->setColor(cor);
	}
}

int StateMenu::unload()
{
	delete(bg);
	delete(pressEnter);
	delete(title);
	delete(difficultyText);
	delete(diffDesc);
	delete(recordText);
	delete(hintText);

	for(int i = 0; i < MENU_ACH_LINHAS; ++i)
		delete(achTexts[i]);

	delete(music);

	return 0;
}

int StateMenu::update(int dt)
{
	bg->update(dt);

	// Swing suave do titulo (menu "vivo").
	menuTime += dt;

	InputManager &im = InputManager::getInstance();

	// 1/2/3 escolhem a dificuldade; os textos refletem a mudanca na hora.
	if(im.isKeyDown(SDLK_1) && GameSettings::difficulty() != DIF_FACIL)
		GameSettings::difficulty() = DIF_FACIL;
	if(im.isKeyDown(SDLK_2) && GameSettings::difficulty() != DIF_NORMAL)
		GameSettings::difficulty() = DIF_NORMAL;
	if(im.isKeyDown(SDLK_3) && GameSettings::difficulty() != DIF_DIFICIL)
		GameSettings::difficulty() = DIF_DIFICIL;

	if(GameSettings::difficulty() != shownDifficulty)
		refreshTexts();

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
	difficultyText->render();
	diffDesc->render();
	recordText->render();
	hintText->render();

	for(int i = 0; i < MENU_ACH_LINHAS; ++i)
		achTexts[i]->render();
}
