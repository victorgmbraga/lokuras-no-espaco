/*
 * Camera.cpp
 *
 *  Created on: 28/05/2013
 *      Author: Victor
 */

#include "Camera.h"

Camera* Camera::instance = NULL;

Camera::Camera(float x, float y, GameObject *bindObject)
{
	this->x = x;
	this->y = y;
	inputManager = NULL;
	this->bindObject = bindObject;
}

Camera& Camera::getInstance()
{
	if (!instance)
		instance = new Camera();
	return *instance;
}

void Camera::update()
{
	// Se a câmera está associada com um GameObject, atualiza a
	// posição de acordo com o objeto.
	if(bindObject)
	{
		x = bindObject->x - SCREEN_W/2 + bindObject->w/2;
		y = bindObject->y - SCREEN_H/2 + bindObject->h/2;
	}
	// Senão, atualiza a câmera de acordo com as setas do teclado.
	else
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
}

void Camera::bind(GameObject* bindObject)
{
	this->bindObject = bindObject;
}

void Camera::unBind()
{
	if(bindObject)
		bindObject = NULL;
}

