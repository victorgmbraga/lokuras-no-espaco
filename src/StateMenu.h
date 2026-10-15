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
#include "InputManager.h"

class StateMenu: public State
{
	private:
		Sprite *bg;
		Text *pressEnter;
		Text *title;
		Audio *music;
		int returnState;
		InputManager *inputManager;
	public:
		StateMenu() {}
		virtual ~StateMenu() {}
		void load(int stack = 0);
		int unload();
		int update(int dt);
		void render();
};

#endif /* STATEMENU_H_ */
