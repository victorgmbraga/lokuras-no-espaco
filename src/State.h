/*
 * State.h
 *
 *  Created on: 13/06/2013
 *      Author: Victor
 */

#ifndef STATE_H_
#define STATE_H_

// Codigos de troca de estado, lidos pelo StateManager.
#define STATE 0
#define STATE_MENU 1
#define STATE_GAME 2
#define STATE_WINLOSE 3
#define STATE_QUIT -1

class State
{
	public:
		State() {}
		virtual ~State() {}
		virtual void load(int stack = 0) = 0;
		virtual int unload() = 0;
		virtual int update(int dt) = 0;
		virtual void render() = 0;
};

#endif /* STATE_H_ */
