/*
 * FollowerObject.cpp
 *
 *  Created on: 31/05/2013
 *      Author: Victor
 */

#include "FollowerObject.h"
#include "Camera.h"
#include <cmath>

FollowerObject::FollowerObject(Sprite* sprite, float x, float y):
	GameObject(x, y, sprite->getRenderWidth(), sprite->getRenderHeight())
{
	this->sprite = sprite;
	speedX = 0;
	speedY = 0;
	deltaX = 0;
	deltaY = 0;
	autonomous = false;
	target = NULL;
	chaseSpeed = 0;
	hp = UFO_MAX_HP;
	maxHp = UFO_MAX_HP;
	renderOffsetX = 0;
	renderOffsetY = 0;
	xCoordinatesQueue = new std::queue<float>;
	yCoordinatesQueue = new std::queue<float>;
}

FollowerObject::~FollowerObject()
{
	delete(xCoordinatesQueue);
	delete(yCoordinatesQueue);
}

void FollowerObject::setAutonomous(bool value)
{
	autonomous = value;
}

void FollowerObject::setChaseTarget(GameObject* target)
{
	this->target = target;
}

void FollowerObject::setChaseSpeed(float speed)
{
	chaseSpeed = speed;
}

void FollowerObject::setHitbox(float width, float height)
{
	float spriteW = sprite->getRenderWidth();
	float spriteH = sprite->getRenderHeight();

	if(width <= 0 || height <= 0 || width > spriteW || height > spriteH)
		return;

	// Mantem o centro do objeto (e o sprite no mesmo lugar) e encolhe a
	// caixa de colisao em volta dele.
	float centerX = x + w/2;
	float centerY = y + h/2;

	w = width;
	h = height;
	x = centerX - w/2;
	y = centerY - h/2;

	renderOffsetX = (spriteW - w) / 2;
	renderOffsetY = (spriteH - h) / 2;
}

bool FollowerObject::isAutonomous()
{
	return autonomous;
}

void FollowerObject::update(int dt)
{
	// Modo perseguidor (UFO da partida): avanca sempre na direcao do
	// centro do alvo, com velocidade controlada pelo StateGame.
	if(autonomous)
	{
		if(target)
		{
			float tx = target->x + target->w/2 - (x + w/2);
			float ty = target->y + target->h/2 - (y + h/2);
			float dist = sqrtf(tx*tx + ty*ty);

			if(dist > 2.0f)
			{
				float passo = chaseSpeed * dt;
				if(passo > dist)
					passo = dist;
				x += tx / dist * passo;
				y += ty / dist * passo;
			}
		}
		return;
	}

	// Captura o input do botão direito do mouse. Se sim, coloca
	// a posição dele na fila de ações. O destino guarda o ponto onde o
	// CENTRO do objeto deve parar (por isso o meio dos tamanhos).
	if(InputManager::getInstance().isMouseDown(3))
	{
		int mousePosX = InputManager::getInstance().mousePosX();
		int mousePosY = InputManager::getInstance().mousePosY();
		float cameraX = Camera::getInstance().getX();
		float cameraY = Camera::getInstance().getY();

		enqueueCommand(mousePosX + cameraX - w/2, mousePosY + cameraY - h/2);
	}

	// Se há um destino na pilha, muda a velocidade do objeto.
	if(!(bool)xCoordinatesQueue->empty() || !(bool)yCoordinatesQueue->empty())
	{
		if(!deltaX)
			deltaX = xCoordinatesQueue->front() - x;
		if(!deltaY)
			deltaY = yCoordinatesQueue->front() - y;

		speedX = FOLLOW_SPEED * deltaX * dt;
		speedY = FOLLOW_SPEED * deltaY * dt;

		// Limita a velocidade: com dt real grande, delta*0.0008*dt pode
		// dar um passo desproporcional.
		float cap = FOLLOW_MAX * dt;
		if(speedX > cap)
			speedX = cap;
		if(speedX < -cap)
			speedX = -cap;
		if(speedY > cap)
			speedY = cap;
		if(speedY < -cap)
			speedY = -cap;

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
	// A hitbox pode ser menor que o sprite (UFO com margens transparentes);
	// o offset mantem a imagem exatamente onde estava em relacao ao centro.
	if(sprite)
		sprite->render(x - cameraX - renderOffsetX, y - cameraY - renderOffsetY);
}

void FollowerObject::renderQueueLines(float cameraX, float cameraY)
{
	// Copias locais das filas: a original NAO e tocada (antes ela era
	// esvaziada e recriada a cada frame - vazamento em todo frame ativo).
	std::queue<float> auxX(*xCoordinatesQueue);
	std::queue<float> auxY(*yCoordinatesQueue);

	// A cadeia de linhas sai do CENTRO do objeto e passa pelos centros.
	float x = this->x + w/2;
	float y = this->y + h/2;
	float destX;
	float destY;

	while(!(bool)auxX.empty())
	{
		destX = auxX.front() + w/2;
		destY = auxY.front() + h/2;

		SDLBase::drawLine(x - cameraX, y - cameraY, destX - cameraX, destY - cameraY, 0xFFFFFF, 1);

		x = destX;
		y = destY;

		auxX.pop();
		auxY.pop();
	}
}

void FollowerObject::enqueueCommand(float x, float y)
{
	xCoordinatesQueue->push(x);
	yCoordinatesQueue->push(y);
}
