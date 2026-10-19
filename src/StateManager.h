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

		// Estado do loop central. Sao membros (e nao locais do run()) porque
		// no build web cada frame roda em um callback do navegador - veja
		// StateManager.cpp.
		bool quit;
		int fadeAlpha;
		int fadePhase;
		int targetCode;

		// Um frame do jogo: atualiza a entrada, o estado atual e apresenta.
		void frame();

		#ifdef __EMSCRIPTEN__
		static void frameCallback(void *arg);
		#endif
	public:
		StateManager();
		virtual ~StateManager();
		void run();
};

#endif /* STATEMANAGER_H_ */
