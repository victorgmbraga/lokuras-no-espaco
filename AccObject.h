/*
 * AccObject.h
 *
 *  Created on: 09/06/2013
 *      Author: Victor
 */

#ifndef ACCOBJECT_H_
#define ACCOBJECT_H_

#define PI 3.1415

#include "GameObject.h"
#include "Animation.h"

class AccObject: public GameObject
{
	protected:
		Animation *animation;
		float speed;
		float acceleration;
		float rotation;
	public:
		int hp;
		AccObject(Animation* animation, float x, float y, float w, float h, int hp);
		virtual ~AccObject();
		void render(float cameraX, float cameraY);
		void update(int dt);
		void stop();
};

#endif /* ACCOBJECT_H_ */
