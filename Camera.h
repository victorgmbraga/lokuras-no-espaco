/*
 * Camera.h
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdlib.h>
#include "InputManager.h"

#define CAMERA_SPEED 10.0f

class Camera
{
	private:
		float x, y;
		InputManager *inputManager;
		Camera(float x = 0, float y = 0);
		static Camera *instance;
	public:
		static Camera& getInstance();
		void update(int dt);
		inline float getX() { return x; }
		inline float getY() { return y; }
};

#endif /* CAMERA_H_ */
