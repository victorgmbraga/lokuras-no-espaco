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
#include "InputManager.h"

class StateWinLose: public State
{
	private:
		Sprite *bg;
		Text *txt;
		Text *pressEnter;
		Audio *music;
		int returnState;
		InputManager *inputManager;
	public:
		StateWinLose() {}
		virtual ~StateWinLose() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* STATEWINLOSE_H_ */
