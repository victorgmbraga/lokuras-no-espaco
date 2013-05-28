/*
 * Planeta.cpp
 *
 *  Created on: 20/04/2013
 *      Author: Victor
 */

#include "Planet.h"

Planet::Planet(Sprite *sprite, float x, float y, float hp): GameObject(x, y)
{
	this->sprite = sprite;
	this->hp = hp;
}

void Planet::render(float cameraX, float cameraY)
{
	if(sprite)
		sprite->render(x - cameraX, y - cameraY);
}


