/*
 * StateMenu.h
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#ifndef STATEMENU_H_
#define STATEMENU_H_

#include "SDLBase.h"
#include "State.h"
#include "Sprite.h"
#include "Text.h"
#include "Audio.h"
#include "PlayStats.h"

#define MENU_ACH_LINHAS 6

class StateMenu: public State
{
	private:
		Sprite *bg;
		Text *pressEnter;
		Text *title;
		Text *recordText;  // recorde da dificuldade escolhida (NULL se ainda nao existe)
		Text *difficultyText;
		Text *diffDesc;      // o que muda em cada dificuldade
		Text *hintText;    // semente/desafio do dia
		Text *achTexts[MENU_ACH_LINHAS]; // lista de conquistas (lado esquerdo)
		Audio *music;
		int titleX, titleY;
		float menuTime;    // para o "swing" do titulo
		int returnState;
		int shownDifficulty; // última dificuldade refletida nos textos
		void refreshTexts();
	public:
		StateMenu() {}
		virtual ~StateMenu() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* STATEMENU_H_ */
