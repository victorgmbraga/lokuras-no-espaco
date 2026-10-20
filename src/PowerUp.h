/*
 * PowerUp.h
 *
 *  Created on: 17/10/2026
 *      Author: Victor
 *
 *  Cristais coletaveis: PEITORAL verde de vida, azul de escudo e
 *  laranja de turbo. Header-only porque a logica e minima.
 */

#ifndef POWERUP_H_
#define POWERUP_H_

#include <cmath>
#include "Planet.h"

class PowerUp: public Planet
{
	public:
		enum Kind { VIDA = 0, ESCUDO = 1, TURBO = 2, FUEL = 3 };

		int kind;
		float baseY;
		float bobT;

		PowerUp(Sprite* sprite, float x, float y, int kind):
			Planet(sprite, x, y, 1), kind(kind), baseY(y), bobT(0) {}
		virtual ~PowerUp() {}

		virtual void update(int dt)
		{
			// Balanco suave de cima pra baixo para dar destaque no espaço.
			bobT += dt;
			y = baseY + sin(bobT * 0.004f) * 6;
		}
};

#endif /* POWERUP_H_ */
