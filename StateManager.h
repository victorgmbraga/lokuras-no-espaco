/*
 * StateManager.h
 *
 *  Created on: 13/06/2013
 *      Author: Victor
 */

#ifndef STATEMANAGER_H_
#define STATEMANAGER_H_

#define STATE 0
#define STATE_MENU 1
#define STATE_GAME 2
#define STATE_WINLOSE 3
#define STATE_QUIT -1

#include "SDLBase.h"
#include "Timer.h"
#include "InputManager.h"
#include "State.h"
#include "StateGame.h"
#include "StateMenu.h"
#include "StateWinLose.h"

class StateManager
{
	private:
		State *currentState;
		int stack;
		Timer *timer;
		InputManager *inputManager;
		int dt;
	public:
		StateManager();
		virtual ~StateManager();
		void run();
};

#endif /* STATEMANAGER_H_ */
