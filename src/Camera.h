/*
 * Camera.h
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#ifndef CAMERA_H_
#define CAMERA_H_

// Velocidade da camara livre em px/ms (tempo real, nao por frame).
#define CAMERA_SPEED 0.6f

class GameObject;

#include <stdlib.h>
#include "SDLBase.h"
#include "InputManager.h"
#include "GameObject.h"

class Camera
{
	private:
		float x, y;
		static Camera *instance;
		GameObject *bindObject;
		Camera(float x = 0, float y = 0, GameObject* bindObject = NULL);

		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
	public:
		static Camera& getInstance();
		void update(int dt);
		inline float getX() { return x; }
		inline float getY() { return y; }
		void bind(GameObject* bindObject);
		void unBind();
};

#endif /* CAMERA_H_ */
