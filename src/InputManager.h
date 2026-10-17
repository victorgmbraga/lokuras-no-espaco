/*
 * InputManager.h
 *
 *  Created on: 17/05/2013
 *      Author: Victor
 */

#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

// SDL_GetKeyState cobre ate SDLK_LAST (~323): 512 cobre todas as SDLKey.
#define N_KEYS 512
// SDL 1.2: botões 1..3 + roda (4/5); 8 cobre os mouses "extras".
#define N_MOUSE_BUTTONS 8
#define N_JOY_BUTTONS 16

#include "SDLBase.h"
#include <stdlib.h>


class InputManager
{
	private:
		Uint8 mouse, *keyboard;
		Uint8 keyDown[N_KEYS], keyUp[N_KEYS];
		bool quit;
		bool fullscreenToggle;
		bool altEnterDown;
		Uint8 mouseDown[N_MOUSE_BUTTONS], mouseUp[N_MOUSE_BUTTONS];
		int mouseX, mouseY;
		SDL_Joystick *joystick;
		int joyAxes[2];                  // -1, 0 ou 1 (com zona morta)
		bool joyButtonState[N_JOY_BUTTONS];
		bool joyButtonUp[N_JOY_BUTTONS];
		static InputManager *instance;
		InputManager();

		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

	public:
		static InputManager& getInstance();
		void update();
		bool isKeyDown(int key);
		bool isKeyUp(int key);
		bool isKeyPressed(int key);
		bool isMouseDown(int button);
		bool isMouseUp(int button);
		bool isMousePressed(Uint8 button);
		bool isMouseHeld(Uint8 button);
		int mousePosX();
		int mousePosY();
		bool isMouseInside(SDL_Rect *rect);
		int getJoyAxis(int axis);
		bool isJoyDown(int button);
		bool isJoyUp(int button);
		bool quitGame();
		bool consumeFullscreenToggle();
};


#endif /* INPUTMANAGER_H_ */
