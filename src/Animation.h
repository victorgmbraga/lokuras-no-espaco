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
		bool oneShot;
		bool finished;
	public:
		Animation(string filename, int frameTime, int n_sprites);
		virtual ~Animation();
		void update(int dt);
		void setFrame(int frame);
		void setFrameTime(int time);
		int getFrameTime();
		// Toca uma unica vez ate o ultimo quadro e para (explosao).
		void setOneShot(bool value);
		bool isFinished();
		// Volta ao primeiro quadro e habilita a animacao de novo.
		void restart();
};

#endif /* ANIMATION_H_ */
