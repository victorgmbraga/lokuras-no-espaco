/*
 * Camera.cpp
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#include "Camera.h"

Camera* Camera::instance = NULL;

Camera::Camera(float x, float y)
{
	this->x = x;
	this->y = y;
	inputManager = NULL;
}

Camera& Camera::getInstance()
{
	if (!instance)
		instance = new Camera();
	return *instance;
}

void Camera::update(int dt)
{
	if(inputManager->getInstance().isKeyPressed(SDLK_UP))
		y -= CAMERA_SPEED;
	if(inputManager->getInstance().isKeyPressed(SDLK_DOWN))
		y += CAMERA_SPEED;
	if(inputManager->getInstance().isKeyPressed(SDLK_LEFT))
		x -= CAMERA_SPEED;
	if(inputManager->getInstance().isKeyPressed(SDLK_RIGHT))
		x += CAMERA_SPEED;
}

