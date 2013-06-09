/*
 * GameObject.h
 *
 *  Created on: 26/05/2013
 *      Author: Victor
 */

#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_

#include "InputManager.h"
#include "Rect.h"

class Camera;

#include "Camera.h"


class GameObject
{
	protected:
		Camera *camera;
		InputManager *inputManager;
		Rect box;
	public:
		float x, y, w, h;
		GameObject(float x, float y, float w, float h);
		virtual ~GameObject() {};
		virtual void update(int dt) = 0;
		virtual void render(float cameraX, float cameraY) = 0;
		bool collidesWith(GameObject* other);
};

#endif /* GAMEOBJECT_H_ */
