/*
 * Asteroid.h
 *
 *  Created on: 20/10/2026
 *      Author: Victor
 *
 *  Asteroide pequeno: nasce espalhado pelo mapa, deriva devagar e quica nas
 *  bordas do mundo. E um perigo "atravessavel": a nave passa por ele, mas
 *  leva um dano bem pequeno ao encostar. Header-only porque a logica e minima.
 */

#ifndef ASTEROID_H_
#define ASTEROID_H_

#include <cmath>
#include <stdlib.h>
#include "Planet.h"

class Asteroid: public GameObject
{
	private:
		float mundoW, mundoH;

	public:
		Sprite *sprite;
		float vx, vy;

		Asteroid(Sprite *sprite, float x, float y, float worldW, float worldH):
			GameObject(x, y, (float)sprite->getRenderWidth(), (float)sprite->getRenderHeight()),
			mundoW(worldW), mundoH(worldH), sprite(sprite)
		{
			// Deriva lenta e aleatoria; o minimo evita asteroide parado.
			vx = ((rand() % 200) - 100) * 0.0003f;
			vy = ((rand() % 200) - 100) * 0.0003f;
			if(fabsf(vx) < 0.008f)
				vx = vx < 0 ? -0.008f : 0.008f;
			if(fabsf(vy) < 0.008f)
				vy = vy < 0 ? -0.008f : 0.008f;
		}
		virtual ~Asteroid() {}

		virtual void update(int dt)
		{
			x += vx * dt;
			y += vy * dt;

			// Quica nas bordas do mundo (nao usa os tiles: e atravessavel).
			if(x < 0)
			{
				x = 0;
				vx = -vx;
			}
			if(y < 0)
			{
				y = 0;
				vy = -vy;
			}
			if(x + w > mundoW)
			{
				x = mundoW - w;
				vx = -vx;
			}
			if(y + h > mundoH)
			{
				y = mundoH - h;
				vy = -vy;
			}
		}

		virtual void render(float cameraX = 0.0f, float cameraY = 0.0f)
		{
			if(sprite)
				sprite->render(x - cameraX, y - cameraY);
		}
};

#endif /* ASTEROID_H_ */
