/*
 * StateWinLose.h
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#ifndef STATEWINLOSE_H_
#define STATEWINLOSE_H_

#define LOSE 0
#define WIN 1

#include "SDLBase.h"
#include "State.h"
#include "Sprite.h"
#include "Text.h"
#include "Audio.h"
#include "PlayStats.h"

class StateWinLose: public State
{
	private:
		Sprite *bg;
		Text *txt;
		Text *statsText;   // tempo, planetas e pontos da partida
		Text *recordText;  // "NOVO RECORDE!" quando for o caso (NULL senao)
		Text *pressEnter;
		Audio *music;
		int returnState;
	public:
		StateWinLose() {}
		virtual ~StateWinLose() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* STATEWINLOSE_H_ */
