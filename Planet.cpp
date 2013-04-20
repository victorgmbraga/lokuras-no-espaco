/*
 * Planeta.cpp
 *
 *  Created on: 20/04/2013
 *      Author: Victor
 */

#include "Planet.h"

Planet::Planet (Sprite *sprite, float hp = 20, float x = 0, float y = 0)
{
	this->sprite = sprite;
	this->hp = hp;
	this->x = x;
	this->y = y;
}

Sprite* Planet::getSprite (void)
{
	return sprite;
}

void Planet::setSprite (Sprite *sprite)
{
	this->sprite = sprite;
}

float Planet::getX (void)
{
	return x;
}

void Planet::setX (float x)
{
	this->x = x;
}

float Planet::getY (void)
{
	return y;
}

void Planet::setY (float y)
{
	this->y = y;
}

float Planet::getHP (void)
{
	return hp;
}

void Planet::setHP (float hp)
{
	this->hp = hp;
}

void Planet::render (void)
{
	sprite->render(x,y);
}


