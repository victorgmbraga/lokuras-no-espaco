/*
 * Planet.h
 *
 *  Created on: 20/04/2013
 *      Author: Victor
 */

#ifndef PLANET_H_
#define PLANET_H_

#include "GameObject.h"
#include "Sprite.h"

class Planet: public GameObject
{
	public:
		Sprite *sprite;
		float hp;
		Planet(Sprite *sprite, float x, float y, float hp);
		virtual ~Planet() {};
		void render(float cameraX = 0.0, float cameraY = 0.0);
		virtual void update(int dt) = 0;
};

#endif /* PLANET_H_ */
