/*
 * Moon.cpp
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#include "Moon.h"

Moon::Moon(Sprite* sprite, int hp, Planet* center): Planet(sprite, center->x, center->y, hp)
{
	this->center = center;
	angle = 0;
	radius = 50;
}

void Moon::update(int dt)
{
	angle += MOON_SPEED * dt;

	float dx = sin(angle) * radius;
	float dy = cos(angle) * radius;

	float centerX = center->x + center->sprite->getWidth()/2;
	float centerY = center->y + center->sprite->getHeight()/2;

	float moonW = sprite->getWidth();
	float moonH = sprite->getHeight();

	x = centerX + dx - moonW;
	y = centerY + dy - moonH;
}



