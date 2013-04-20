/*
 * Planet.h
 *
 *  Created on: 20/04/2013
 *      Author: Victor
 */

#ifndef PLANET_H_
#define PLANET_H_

#include "Sprite.h"

class Planet
{
	private:
		Sprite *sprite;
		float x, y;
		float hp;
	public:
		Planet (Sprite *sprite, float hitpoints, float x, float y);
		Sprite* getSprite (void);
		void setSprite (Sprite *sprite);
		float getX (void);
		void setX (float x);
		float getY (void);
		void setY (float y);
		float getHP (void);
		void setHP (float hp);
		void render(void);
};

#endif /* PLANET_H_ */
