/*
 * Timer.cpp
 *
 *  Created on: 12/06/2013
 *      Author: Victor
 */

#include "Timer.h"

Timer::Timer()
{
	initialTime = 0;
	pauseTime = 0;
	paused = true;
}

Timer::~Timer()
{
	// TODO Auto-generated destructor stub
}

void Timer::start()
{
	initialTime = SDL_GetTicks();
	paused = false;
}

void Timer::pause()
{
	pauseTime = SDL_GetTicks();
	paused = true;
}

void Timer::resume()
{
	initialTime = SDL_GetTicks() - (pauseTime - initialTime);
	paused = false;
}

int Timer::getTime()
{
	if(paused)
		return (pauseTime - initialTime);
	else
		return (SDL_GetTicks() - initialTime);
}
