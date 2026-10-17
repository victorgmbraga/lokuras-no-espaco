/*
 * StateManager.h
 *
 *  Created on: 13/06/2013
 *      Author: Victor
 */

#ifndef STATEMANAGER_H_
#define STATEMANAGER_H_

// Os codigos STATE_* agora moram em State.h (estados precisam devolver
// esses codigos sem conhecer o StateManager).

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
		int dt;
	public:
		StateManager();
		virtual ~StateManager();
		void run();
};

#endif /* STATEMANAGER_H_ */
