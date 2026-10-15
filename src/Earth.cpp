/*
 * Earth.cpp
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#include "Earth.h"

Earth::Earth(Sprite* sprite, float x, float y, int hp): Planet(sprite, x, y, hp)
{
	vx = 0;
	vy = 0;
}

void Earth::update(int dt)
{
/*
	if(inputManager->getInstance().isKeyDown(SDLK_d))
		vx = EARTH_SPEED * dt;
	else if(inputManager->getInstance().isKeyUp(SDLK_d))
		vx = 0;

	if(inputManager->getInstance().isKeyDown(SDLK_a))
		vx = - EARTH_SPEED * dt;
	else if(inputManager->getInstance().isKeyUp(SDLK_a))
		vx = 0;

	if(inputManager->getInstance().isKeyDown(SDLK_w))
		vy = - EARTH_SPEED * dt;
	else if(inputManager->getInstance().isKeyUp(SDLK_w))
		vy = 0;

	if(inputManager->getInstance().isKeyDown(SDLK_s))
		vy = EARTH_SPEED * dt;
	else if(inputManager->getInstance().isKeyUp(SDLK_s))
		vy = 0;

	x += vx;
	y += vy;
*/
}


