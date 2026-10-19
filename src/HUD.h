/*
 * HUD.h
 *
 *  Created on: 14/06/2013
 *      Author: Victor
 */

#ifndef HUD_H_
#define HUD_H_

#include "SDLBase.h"
#include "Text.h"
#include "PlayStats.h"

class HUD
{
	private:
		Text *lifeCounter;
		Text *commandsText;
		Text *commandsText3;
		Text *destroyedText;
		Text *timerText;
		Text *pauseText;
		Text *pauseHint;
		int life;
		void displayLife();
	public:
		HUD();
		virtual ~HUD();
		void update(int timeMs, int destroyed);
		void render();
		void renderPause();
		void setLife(int life);
};

#endif /* HUD_H_ */
