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

#define MENU_ACH_LINHAS 7
#define MENU_LEG_LINHAS 4

class StateMenu: public State
{
	private:
		Sprite *bg;
		Text *pressEnter;
		Text *title;
		Text *recordText;  // recorde do modo e dificuldade (NULL se ainda nao existe)
		Text *difficultyText;
		Text *diffDesc;      // o que muda em cada dificuldade
		Text *modeText;      // travessia (4) ou pontos (5)
		Text *modeDescText;  // objetivo de cada modo
		Text *dailyText;     // desafio diario: melhor de hoje
		Text *hintText;    // semente/desafio do dia
		Text *achTexts[MENU_ACH_LINHAS]; // lista de conquistas (lado esquerdo)
		Text *legendTexts[MENU_LEG_LINHAS]; // legenda dos power-ups (lado direito)
		Audio *music;
		int titleX, titleY;
		float menuTime;    // para o "swing" do titulo
		int returnState;
		int shownDifficulty; // última dificuldade refletida nos textos
		int shownMode;       // ultimo modo refletido nos textos
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
