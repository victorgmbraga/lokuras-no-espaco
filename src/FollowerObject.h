/*
 * FollowerObject.h
 *
 *  Created on: 31/05/2013
 *      Author: Victor
 */

#ifndef FOLLOWEROBJECT_H_
#define FOLLOWEROBJECT_H_

#include <queue>
#define FOLLOW_SPEED 0.0008f // fracao da distancia restante por ms
#define FOLLOW_MAX   0.4f    // limite de velocidade da fila (px/ms)
#define UFO_MAX_HP   20      // tiros necessarios para derrotar o chefe

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
		// Modo perseguidor: avanca continuamente ate o alvo (UFO da partida).
		bool autonomous;
		GameObject* target;
		float chaseSpeed;
	public:
		int hp;     // vida do chefe (UFO)
		int maxHp;
		FollowerObject(Sprite* sprite, float x, float y);
		virtual ~FollowerObject();
		void update(int dt);
		void render(float cameraX, float cameraY);
		void renderQueueLines(float cameraX, float cameraY);
		void enqueueCommand(float x, float y);
		void setAutonomous(bool value);
		void setChaseTarget(GameObject* target);
		void setChaseSpeed(float speed);
		bool isAutonomous();
};


#endif /* FOLLOWEROBJECT_H_ */
