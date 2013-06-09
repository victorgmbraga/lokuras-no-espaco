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
		acceleration = -0.001;
	else if(inputManager->getInstance().isKeyPressed(SDLK_s))
		acceleration = 0.001;
	else
		acceleration = 0;

	if(inputManager->getInstance().isKeyPressed(SDLK_d))
		rotation -= 1;
	if(inputManager->getInstance().isKeyPressed(SDLK_a))
		rotation += 1;

	speed = speed + acceleration * dt;

	x += speed * cos((rotation -90) * PI/180);
	y += speed * -sin((rotation -90) * PI/180);

	animation->update(dt);

	// Animação de boost
	if(acceleration)
		animation->setFrame(2);

	animation->rotoZoom(rotation);

	// Gambiarra para parar a animação quando rotacionar
	// TODO Arrumar o bug da rotação + animação.
	if(rotation)
		animation->setFrame(0);
}

void AccObject::stop()
{
	speed = 0;
	acceleration = 0;
}
