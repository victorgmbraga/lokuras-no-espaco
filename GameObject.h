/*
 * GameObject.h
 *
 *  Created on: 26/05/2013
 *      Author: Victor
 */

#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_

#include "Camera.h"
#include "InputManager.h"

class GameObject
{
	protected:
		Camera* camera;
		InputManager *inputManager;
	public:
		float x, y;
		GameObject(float x, float y);
		virtual ~GameObject() {};
		virtual void update(int dt) = 0;
		virtual void render(float cameraX, float cameraY) = 0;
};

#endif /* GAMEOBJECT_H_ */
