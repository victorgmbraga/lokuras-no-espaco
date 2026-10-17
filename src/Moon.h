/*
 * Moon.h
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#ifndef MOON_H_
#define MOON_H_

#define MOON_SPEED 0.001

#include "Planet.h"
#include "Camera.h"
#include <math.h> // sin/cos (o Windows aceitava "Math.h", mas o Linux diferencia maiusculas)

class Moon: public Planet
{
	private:
		float radius;
		float angle;
		Planet* center;
	public:
		Moon(Sprite* sprite, int hp = 1, Planet* center = NULL);
		virtual void update(int dt);
};



#endif /* MOON_H_ */
