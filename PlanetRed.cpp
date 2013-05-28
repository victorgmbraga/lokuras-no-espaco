/*
 * PlanetRed.cpp
 *
 *  Created on: 27/05/2013
 *      Author: Victor
 */

#include "PlanetRed.h"

PlanetRed::PlanetRed(Sprite* sprite, float x, float y, int hp): Planet(sprite, x, y, hp){}

void PlanetRed::update(int dt)
{
	float cameraX = camera->getInstance().getX();
	float cameraY = camera->getInstance().getY();
	SDL_Rect rect;

	rect.x = x - cameraX;
	rect.y = y - cameraY;
	rect.w = sprite->getWidth();
	rect.h = sprite->getHeight();


	if(inputManager->getInstance().isMouseDown(1))
		if(inputManager->getInstance().isMouseInside(&rect))
			hp -= (rand() % 10 + 10);
}

