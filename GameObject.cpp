/*
 * GameObject.cpp
 *
 *  Created on: 26/05/2013
 *      Author: Victor
 */

#include "GameObject.h"

GameObject::GameObject(float x, float y)
{
	this->x = x;
	this->y = y;
	camera = NULL;
	inputManager = NULL;
}
