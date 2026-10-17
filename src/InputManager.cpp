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
	fullscreenToggle = false;
	altEnterDown = false;
	joystick = NULL;

	for (int i = 0; i < 2; ++i)
		joyAxes[i] = 0;

	// Abre o primeiro joystick/gamepad, se houver (opcional).
	if (SDL_NumJoysticks() > 0)
	{
		joystick = SDL_JoystickOpen(0);
		if (joystick == NULL)
			cerr << "InputManager: nao foi possivel abrir o joystick 0: " << SDL_GetError() << endl;
		else
			SDL_JoystickEventState(SDL_ENABLE);
	}

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

	for(int i = 0; i < N_JOY_BUTTONS; ++i)
	{
		joyButtonState[i] = false;
		joyButtonUp[i] = false;
	}

	for (int i = 0; i < 2; ++i)
		joyAxes[i] = 0;

	mouse = SDL_GetMouseState(NULL, NULL);
	keyboard = SDL_GetKeyState(NULL);

	while(SDL_PollEvent(&event))
	{
		/* verifica tipo do evento e toma a ação */
		switch (event.type)
		{
			case SDL_KEYDOWN:
				// Alt+Enter alterna a tela cheia. O evento nao vira um ENTER
				// normal para nao iniciar o jogo sem querer no menu.
				if ((event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
					&& (event.key.keysym.mod & KMOD_ALT))
				{
					if (!altEnterDown)
						fullscreenToggle = true;
					altEnterDown = true;
				}
				else if (event.key.keysym.sym >= 0 && event.key.keysym.sym < N_KEYS)
					keyDown[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
					altEnterDown = false;
				if (event.key.keysym.sym >= 0 && event.key.keysym.sym < N_KEYS)
					keyUp[event.key.keysym.sym] = true;
				break;
			case SDL_MOUSEMOTION:
				mouseX = event.motion.x;
				mouseY = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				// Botões fora da faixa (incluindo roda) nunca mais estouram o array.
				if (event.button.button >= 0 && event.button.button < N_MOUSE_BUTTONS)
					mouseDown[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button >= 0 && event.button.button < N_MOUSE_BUTTONS)
					mouseUp[event.button.button] = true;
				break;
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis < 2)
				{
					Sint16 value = event.jaxis.value;
					// Zona morta conservadora (~24% do curso).
					int dead = 8000;
					joyAxes[event.jaxis.axis] = value > dead ? 1 : (value < -dead ? -1 : 0);
				}
				break;
			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.button >= 0 && event.jbutton.button < N_JOY_BUTTONS)
					joyButtonState[event.jbutton.button] = true;
				break;
			case SDL_JOYBUTTONUP:
				if (event.jbutton.button >= 0 && event.jbutton.button < N_JOY_BUTTONS)
					joyButtonUp[event.jbutton.button] = true;
				break;
			case SDL_QUIT:
				quit = true;
				break;
		}
	}
}

bool InputManager::isKeyDown(int key)
{
	return key >= 0 && key < N_KEYS && keyDown[key];
}

bool InputManager::isKeyUp(int key)
{
	return key >= 0 && key < N_KEYS && keyUp[key];
}

bool InputManager::isKeyPressed(int key)
{
	return key >= 0 && key < N_KEYS && keyboard[key];
}

bool InputManager::isMouseDown(int button)
{
	return button >= 0 && button < N_MOUSE_BUTTONS && mouseDown[button];
}

bool InputManager::isMouseUp(int button)
{
	return button >= 0 && button < N_MOUSE_BUTTONS && mouseUp[button];
}

// "Pressionado agora": o press event chegou neste frame.
bool InputManager::isMousePressed(Uint8 button)
{
	return button < N_MOUSE_BUTTONS && mouseDown[button];
}

// "Segurado": estado atual do botão (polling da SDL, não eventos).
bool InputManager::isMouseHeld(Uint8 button)
{
	return button >= 1 && button <= 8 && (mouse & SDL_BUTTON(button));
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

int InputManager::getJoyAxis(int axis)
{
	return axis >= 0 && axis < 2 ? joyAxes[axis] : 0;
}

bool InputManager::isJoyDown(int button)
{
	return button >= 0 && button < N_JOY_BUTTONS && joyButtonState[button];
}

bool InputManager::isJoyUp(int button)
{
	return button >= 0 && button < N_JOY_BUTTONS && joyButtonUp[button];
}

bool InputManager::quitGame()
{
	return quit;
}

bool InputManager::consumeFullscreenToggle()
{
	bool requested = fullscreenToggle;
	fullscreenToggle = false;
	return requested;
}
