/*
 * PlanetRed.h
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 */

#ifndef PLANETRED_H_
#define PLANETRED_H_

#include "Planet.h"
#include "InputManager.h"

class PlanetRed: public Planet
{
	public:
		PlanetRed(Sprite* sprite, float x, float y, int hp = 1);
		virtual void update(int dt);
};


#endif /* PLANETRED_H_ */
