/*
 * GameManager.h
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#ifndef GAMEMANAGER_H_
#define GAMEMANAGER_H_

#include "Sprite.h"
#include "SDLBase.h"

class GameManager
{
	private:
		Sprite *bg;

	public:
		GameManager (void);
		~GameManager (void);
		void run (void);
};

#endif /* GAMEMANAGER_H_ */
