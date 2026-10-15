/*
 * Timer.h
 *
 *  Created on: 12/06/2013
 *      Author: Victor
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "SDLBase.h"

class Timer
{
	private:
		int initialTime;
		int pauseTime;
		bool paused;
	public:
		Timer();
		virtual ~Timer();
		void start();
		void pause();
		void resume();
		int getTime();
};

#endif /* TIMER_H_ */
