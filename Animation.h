/*
 * Animation.h
 *
 *  Created on: 05/06/2013
 *      Author: Victor
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "Sprite.h"

class Animation: public Sprite
{
	private:
		int n_sprites;
		int time;
		int frameSkip;
		int frameTime;
		SDL_Rect clipRect;
	public:
		Animation(string filename, int frameTime, int n_sprites);
		virtual ~Animation();
		void update(int dt);
		void setFrame(int frame);
		void setFrameTime(int time);
		int getframeTime();
};

#endif /* ANIMATION_H_ */
