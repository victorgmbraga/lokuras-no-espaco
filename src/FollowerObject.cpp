/*
 * FollowerObject.cpp
 *
 *  Created on: 31/05/2013
 *      Author: Victor
 */

#include "FollowerObject.h"

FollowerObject::FollowerObject(Sprite* sprite, float x, float y): GameObject(x, y, sprite->getWidth(), sprite->getHeight())
{
	this->sprite = sprite;
	speedX = 0;
	speedY = 0;
	deltaX = 0;
	deltaY = 0;
	xCoordinatesQueue = new std::queue<float>;
	yCoordinatesQueue = new std::queue<float>;
}

void FollowerObject::update(int dt)
{
	// Capura o input do botão direito do mouse. Se sim, coloca
	// a posição dele na fila de ações.
	if(inputManager->getInstance().isMouseDown(3))
	{
		int mousePosX = inputManager->getInstance().mousePosX();
		int mousePosY = inputManager->getInstance().mousePosY();
		float cameraX = camera->getInstance().getX();
		float cameraY = camera->getInstance().getY();

		enqueueCommand(mousePosX + cameraX, mousePosY + cameraY);
	}

	// Se há um destino na pilha, muda a velocidade do objeto.
	if(!(bool)xCoordinatesQueue->empty() || !(bool)yCoordinatesQueue->empty())
	{
		if(!deltaX)
			deltaX = xCoordinatesQueue->front() - x;
		if(!deltaY)
			deltaY = yCoordinatesQueue->front() - y;

		speedX = 0.001 * deltaX * dt;
		speedY = 0.001 * deltaY * dt;

		if((deltaX >= 0 && x >= xCoordinatesQueue->front()) || (deltaX <= 0 && x <= xCoordinatesQueue->front()) ||
			(deltaY >= 0 && y >= yCoordinatesQueue->front()) || (deltaY <= 0 && y <= yCoordinatesQueue->front()))
		{
			x = xCoordinatesQueue->front();
			y = yCoordinatesQueue->front();

			xCoordinatesQueue->pop();
			yCoordinatesQueue->pop();

			speedX = 0;
			speedY = 0;

			deltaX = 0;
			deltaY = 0;
		}
		else
		{
			x += speedX;
			y += speedY;
		}
	}
}

void FollowerObject::render(float cameraX, float cameraY)
{
	// Calcula o centro do objeto (para não renderizar o
	// canto superior esquerdo).
	float fixedX = x - sprite->getWidth()/2;
	float fixedY = y - sprite->getHeight()/2;

	if(sprite)
		sprite->render(fixedX - cameraX, fixedY - cameraY);
}

void FollowerObject::renderQueueLines(float cameraX, float cameraY)
{
	std::queue<float> *auxXCoordinatesQueue = new std::queue<float>;
	std::queue<float> *auxYCoordinatesQueue = new std::queue<float>;
	float x = this->x;
	float y = this->y;
	float destX;
	float destY;

	while(!(bool)xCoordinatesQueue->empty())
	{
		 destX = xCoordinatesQueue->front();
		 destY = yCoordinatesQueue->front();

		 SDLBase::drawLine(x - cameraX, y - cameraY, destX - cameraX, destY - cameraY, 0xFFFFFF, 1);

		 x = destX;
		 y = destY;

		 xCoordinatesQueue->pop();
		 yCoordinatesQueue->pop();

		 auxXCoordinatesQueue->push(destX);
		 auxYCoordinatesQueue->push(destY);
	}

	xCoordinatesQueue = auxXCoordinatesQueue;
	yCoordinatesQueue = auxYCoordinatesQueue;
}

void FollowerObject::enqueueCommand(float x, float y)
{
	xCoordinatesQueue->push(x);
	yCoordinatesQueue->push(y);
}
