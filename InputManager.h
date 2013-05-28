/*
 * InputManager.h
 *
 *  Created on: 17/05/2013
 *      Author: Victor
 */

#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#define N_KEYS 255
#define N_MOUSE_BUTTONS 3

#include "SDLBase.h"
#include <stdlib.h>


class InputManager
{
	private:
		Uint8 mouse, *keyboard;
		Uint8 keyDown[N_KEYS], keyUp[N_KEYS];
		bool quit;
		Uint8 mouseDown[N_MOUSE_BUTTONS], mouseUp[N_MOUSE_BUTTONS];
		int mouseX, mouseY;
		static InputManager *instance;
		InputManager();

	public:
		static InputManager& getInstance();
		void update();
		bool isKeyDown(int key);
		bool isKeyUp(int key);
		bool isKeyPressed(int key);
		bool isMouseDown(int button);
		bool isMouseUp(int button);
		bool isMousePressed(Uint8 button);
		int mousePosX();
		int mousePosY();
		bool isMouseInside(SDL_Rect *rect);
		bool quitGame();
};


#endif /* INPUTMANAGER_H_ */
