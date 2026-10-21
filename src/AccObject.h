/*
 * AccObject.h
 *
 *  Created on: 09/06/2013
 *      Author: Victor
 */

#ifndef ACCOBJECT_H_
#define ACCOBJECT_H_

#define PI 3.14159265f

// Fisica em tempo REAL: velocidade em px/ms, aceleracao em px/ms^2,
// rotacao em graus/ms (tudo multiplicado pelo dt do frame).
#define MAX_SPEED      0.35f
#define ACCELERATION   0.001f
#define FRICTION       0.0005f
#define ROTATION_SPEED 0.15f

// Power-up de turbo (aplicado no StateGame).
#define BOOST_MULT 1.6f
#define BOOST_TIME 5000

// Zona morta do analogico do gamepad (mesmos ~24% do InputManager).
#define JOY_DEAD 0.24f

#include "GameObject.h"
#include "Animation.h"

class AccObject: public GameObject
{
	protected:
		Animation *animation;
		float speed;
		float acceleration;
		float rotation;
		bool atrito;
		float boostMult;
		int boostTime;   // ms restantes de turbo
		int shieldTime;  // ms restantes de escudo (imune a planetas/lua)
	public:
		int hp;
		AccObject(Animation* animation, float x, float y, float w, float h, int hp);
		virtual ~AccObject();
		void render(float cameraX, float cameraY);
		void update(int dt);
		void stop();
		void setBoost(float mult, int durationMs);
		void setShield(int durationMs);
		int getShieldTime();
		float getRotation();
		bool isAccelerating();
		// Componentes da velocidade (px/ms) para o lookahead da Camera.
		virtual float getVelX() const;
		virtual float getVelY() const;
};

#endif /* ACCOBJECT_H_ */
