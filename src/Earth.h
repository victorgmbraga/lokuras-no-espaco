/*
 * Earth.h
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#ifndef EARTH_H_
#define EARTH_H_

#define EARTH_SPEED 0.3

#include "Planet.h"
#include "InputManager.h"

class Earth: public Planet
{
	private:
		float vx, vy;
	public:
		Earth(Sprite* sprite, float x, float y, int hp = 1);
		virtual void update(int dt);
};

#endif /* EARTH_H_ */
