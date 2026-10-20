/*
 * PlanetRed.cpp
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 */

#include "PlanetRed.h"

PlanetRed::PlanetRed(Sprite* sprite, float x, float y, int hp,
                     float mundoW, float mundoH): Planet(sprite, x, y, hp)
{
	// Deriva lenta e aleatoria. srand() precisa ter sido semeado.
	vx = ((rand() % 200) - 100) * 0.0002f;    // ate +-0.02 px/ms
	vy = ((rand() % 200) - 100) * 0.0002f;
	this->mundoW = mundoW;
	this->mundoH = mundoH;
}

void PlanetRed::update(int dt)
{
	x += vx * dt;
	y += vy * dt;

	// Quica nas bordas do mundo gerado para esta partida.
	if(x < 0)
	{
		x = 0;
		vx = -vx;
	}
	if(y < 0)
	{
		y = 0;
		vy = -vy;
	}
	if(x + w > mundoW)
	{
		x = mundoW - w;
		vx = -vx;
	}
	if(y + h > mundoH)
	{
		y = mundoH - h;
		vy = -vy;
	}
}
