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

class GameObject
{
	public:
		float x, y, w, h;
		GameObject(float x, float y, float w, float h);
		virtual ~GameObject() {};
		virtual void update(int dt) = 0;
		virtual void render(float cameraX, float cameraY) = 0;
		bool collidesWith(GameObject* other);
		// Velocidade atual em px/ms (0 por padrao). A Camera usa para o
		// lookahead; a nave sobrescreve com a velocidade real.
		virtual float getVelX() const { return 0.0f; }
		virtual float getVelY() const { return 0.0f; }
};

#endif /* GAMEOBJECT_H_ */
