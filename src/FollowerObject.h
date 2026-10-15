/*
 * FollowerObject.h
 *
 *  Created on: 31/05/2013
 *      Author: Victor
 */

#ifndef FOLLOWEROBJECT_H_
#define FOLLOWEROBJECT_H_

#include <queue>
#include "GameObject.h"
#include "Sprite.h"
#include "Point.h"

class FollowerObject: public GameObject
{
	private:
		Sprite* sprite;
		float speedX, speedY;
		float deltaX, deltaY;
		std::queue<float>* xCoordinatesQueue;
		std::queue<float>* yCoordinatesQueue;
	public:
		FollowerObject(Sprite* sprite, float x, float y);
		void update(int dt);
		void render(float cameraX, float cameraY);
		void renderQueueLines(float cameraX, float cameraY);
		void enqueueCommand(float x, float y);
};


#endif /* FOLLOWEROBJECT_H_ */
