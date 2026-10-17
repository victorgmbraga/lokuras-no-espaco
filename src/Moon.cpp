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
	radius = 200;
}

void Moon::update(int dt)
{
	angle += MOON_SPEED * dt;

	float dx = sin(angle) * radius;
	float dy = cos(angle) * radius;

	// A Lua orbita o CENTRO da Terra — e com a escala assada na surface a
	// hitbox tem o tamanho renderizado, entao o centro e x + w/2.
	float centerX = center->x + center->w/2;
	float centerY = center->y + center->h/2;

	float moonW = sprite->getRenderWidth();
	float moonH = sprite->getRenderHeight();

	// Alinha o centro da Lua ao ponto de orbita.
	x = centerX + dx - moonW/2;
	y = centerY + dy - moonH/2;
}
