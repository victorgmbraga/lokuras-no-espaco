/*
 * AccObject.cpp
 *
 *  Created on: 09/06/2013
 *      Author: Victor
 */

#include "AccObject.h"
#include <cmath>

AccObject::AccObject(Animation *animation, float x, float y, float w, float h, int hp): GameObject(x, y, w, h)
{
	this->animation = animation;
	speed = 0;
	acceleration = 0;
	rotation = 0;
	atrito = true;
	boostMult = 1.0f;
	boostTime = 0;
	shieldTime = 0;
	this->hp = hp;
}

AccObject::~AccObject()
{
	// TODO Auto-generated destructor stub
}

void AccObject::render(float cameraX, float cameraY)
{
	animation->render(x - cameraX, y - cameraY);
}

void AccObject::update(int dt)
{
	InputManager &im = InputManager::getInstance();

	// Teclado ou joystick: o eixo 1 (vertical) controla o acelerador
	// (para cima = frente) e o eixo 0 (horizontal) age como A/D.
	bool frente = im.isKeyPressed(SDLK_w) || im.getJoyAxis(1) < 0;
	bool tras   = im.isKeyPressed(SDLK_s) || im.getJoyAxis(1) > 0;
	int eixoX   = im.getJoyAxis(0);

	// Turbo em vigor multiplica velocidade maxima e aceleracao.
	float mult = boostTime > 0 ? boostMult : 1.0f;
	float maxSpeed = MAX_SPEED * mult;
	float acel = ACCELERATION * mult;

	if(frente)
	{
		atrito = false;
		acceleration = -acel;
	}
	else if(tras)
	{
		atrito = false;
		acceleration = acel;
	}
	else
	{
		atrito = true;
		if(speed > 0)
			acceleration = -FRICTION;
		else if(speed < 0)
			acceleration = FRICTION;
	}

	if(im.isKeyPressed(SDLK_d) || eixoX > 0)
		rotation -= ROTATION_SPEED * dt;
	if(im.isKeyPressed(SDLK_a) || eixoX < 0)
		rotation += ROTATION_SPEED * dt;

	// Normaliza o angulo (evita perder precisao com valores gigantes).
	if(rotation > 360)
		rotation -= 360;
	if(rotation < -360)
		rotation += 360;

	float oldSpeed = speed;

	speed = speed + acceleration * dt;

	if(speed > maxSpeed)
		speed = maxSpeed;

	if(speed < -maxSpeed)
		speed = -maxSpeed;

	if(atrito)
		if((oldSpeed > 0 && speed < 0) || (oldSpeed < 0 && speed > 0))
			speed = 0;

	x += speed * cos((rotation - 90) * PI / 180) * dt;
	y += speed * -sin((rotation - 90) * PI / 180) * dt;

	animation->update(dt);

	// Animação de boost
	if(acceleration && !atrito)
		animation->setFrame(2);

	animation->rotoZoom(rotation);

	// Timers dos power-ups (contam em tempo de partida).
	if(boostTime > 0)
	{
		boostTime -= dt;
		if(boostTime <= 0)
		{
			boostTime = 0;
			boostMult = 1.0f;
		}
	}
	if(shieldTime > 0)
		shieldTime -= dt;
}

void AccObject::stop()
{
	speed = 0;
	acceleration = 0;
	atrito = true;
}

void AccObject::setBoost(float mult, int durationMs)
{
	if(durationMs > 0 && mult > 1.0f)
	{
		boostMult = mult;
		boostTime = durationMs;
	}
	else
	{
		boostMult = 1.0f;
		boostTime = 0;
	}
}

void AccObject::setShield(int durationMs)
{
	shieldTime = durationMs > 0 ? durationMs : 0;
}

int AccObject::getShieldTime()
{
	return shieldTime;
}

bool AccObject::isAccelerating()
{
	return !atrito;
}
