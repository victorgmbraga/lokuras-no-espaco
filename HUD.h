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

class HUD
{
	private:
		Text *lifeCounter;
		Text *commandsText;
		Text *commandsText2;
		Text *commandsText3;
		int life;
		void displayLife();
	public:
		HUD();
		virtual ~HUD();
		void update();
		void render();
		void setLife(int life);
};

#endif /* HUD_H_ */
