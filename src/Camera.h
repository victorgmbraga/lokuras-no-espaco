/*
 * Camera.h
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#define CAMERA_SPEED 10.0f

class GameObject;

#include <stdlib.h>
#include "SDLBase.h"
#include "InputManager.h"
#include "GameObject.h"

class Camera
{
	private:
		float x, y;
		InputManager *inputManager;
		Camera(float x = 0, float y = 0, GameObject* bindObject = NULL);
		static Camera *instance;
		GameObject *bindObject;
	public:
		static Camera& getInstance();
		void update();
		inline float getX() { return x; }
		inline float getY() { return y; }
		void bind(GameObject* bindObject);
		void unBind();
};

#endif /* CAMERA_H_ */
