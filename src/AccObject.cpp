/*
 * AccObject.cpp
 *
 *  Created on: 09/06/2013
 *      Author: Victor
 */

#include "AccObject.h"

AccObject::AccObject(Animation *animation, float x, float y, float w, float h, int hp): GameObject(x, y, w, h)
{
	this->animation = animation;
	box.x = x;
	box.y = y;
	box.w = animation->getClip()->w;
	box.h = animation->getClip()->h;
	speed = 0;
	acceleration = 0;
	rotation = 0;
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
	if(inputManager->getInstance().isKeyPressed(SDLK_w))
	{
		atrito = false;
		acceleration = -0.001;
	}
	else if(inputManager->getInstance().isKeyPressed(SDLK_s))
	{
		atrito = false;
		acceleration = 0.001;
	}
	else
	{
		atrito = true;
		if(speed > 0)
			acceleration = -0.0005;
		else if(speed < 0)
			acceleration = 0.0005;
	}

	if(inputManager->getInstance().isKeyPressed(SDLK_d))
		rotation -= 1;
	if(inputManager->getInstance().isKeyPressed(SDLK_a))
		rotation += 1;

	float oldSpeed = speed;

	speed = speed + acceleration * dt;

	if(speed > MAX_SPEED)
		speed = MAX_SPEED;

	if(speed < -MAX_SPEED)
		speed = -MAX_SPEED;

	if(atrito)
		if((oldSpeed > 0 && speed < 0) || (oldSpeed < 0 && speed > 0))
			speed = 0;

	x += speed * cos((rotation -90) * PI/180);
	y += speed * -sin((rotation -90) * PI/180);

	animation->update(dt);

	// Animação de boost
	if(acceleration && !atrito)
		animation->setFrame(2);

	animation->rotoZoom(rotation);
}

void AccObject::stop()
{
	speed = 0;
	acceleration = 0;
}
