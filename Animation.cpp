/*
 * Animation.cpp
 *
 *  Created on: 05/06/2013
 *      Author: Victor
 */

#include "Animation.h"

Animation::Animation(string filename, int frameTime, int n_sprites): Sprite(filename)
{
	this->frameTime = frameTime;
	this->n_sprites = n_sprites;
	frameSkip = 0;
	time = 0;

	clipRect.w = surface->w/n_sprites;
	clipRect.h = surface->h;
}

Animation::~Animation()
{
	// TODO Auto-generated destructor stub
}

void Animation::update(int dt)
{
	time += dt;
	int saltoFrame = time/frameTime;
	time = time % frameTime;

	clipRect.x += clipRect.w*saltoFrame;

	if(clipRect.x >= surface->w){
		//clipRect.x = (clipRect.x – surface->w) % surface->w;
	}

	clip(clipRect.x, clipRect.y, clipRect.w, clipRect.h);
}


