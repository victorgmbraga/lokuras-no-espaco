/*
 * InputManager.cpp
 *
 *  Created on: 17/05/2013
 *      Author: Victor
 */

#include "InputManager.h"

InputManager* InputManager::instance = NULL;

InputManager::InputManager()
{
	mouse = 0;
	keyboard = NULL;
	mouseX = 0;
	mouseY = 0;
	quit = false;
	//update();
}

InputManager& InputManager::getInstance()
{
	if (!instance)
		instance = new InputManager();
	return *instance;
}

void InputManager::update()
{
	SDL_Event event;

	for(int i = 0; i < N_KEYS; ++i)
	{
		keyDown[i] = false;
		keyUp[i] = false;
	}

	for(int i = 0; i < N_MOUSE_BUTTONS; ++i)
	{
		mouseDown[i] = false;
		mouseUp[i] = false;
	}

	mouse = SDL_GetMouseState(NULL, NULL);
	keyboard = SDL_GetKeyState(NULL);

	while(SDL_PollEvent(&event))
	{
		/* verifica tipo do evento e toma a ação */
		switch (event.type)
		{
			case SDL_KEYDOWN:
				keyDown[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				keyUp[event.key.keysym.sym] = true;
				break;
			case SDL_MOUSEMOTION:
				mouseX = event.motion.x;
				mouseY = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseDown[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				mouseUp[event.button.button] = true;
				break;
			case SDL_QUIT:
				quit = true;
				break;
		}
	}
}

bool InputManager::isKeyDown(int key)
{
	return keyDown[key];
}

bool InputManager::isKeyUp(int key)
{
	return keyUp[key];
}

bool InputManager::isKeyPressed(int key)
{
	if(keyboard[key])
		return true;
	else
		return false;
}

bool InputManager::isMouseDown(int button)
{
	return mouseDown[button];
}

bool InputManager::isMouseUp(int button)
{
	return mouseUp[button];
}

bool InputManager::isMousePressed(Uint8 button)
{
	if(SDL_BUTTON(button))
		return true;
	else
		return false;
}

int InputManager::mousePosX()
{
	return mouseX;
}

int InputManager::mousePosY()
{
	return mouseY;
}

bool InputManager::isMouseInside(SDL_Rect *rect)
{
	int x = rect->x;
	int y = rect->y;
	int w = rect->x + rect->w;
	int h = rect->y + rect->h;

	if(mouseX >= x)
		if(mouseX <= w)
			if(mouseY >= y)
				if(mouseY <= h)
					return true;
	return false;
}

bool InputManager::quitGame()
{
	return quit;
}



