/*
 * Mine.h
 *
 *  Created on: 21/10/2026
 *      Author: Victor
 *
 *  Mina de proximidade: deriva devagar e quica como os planetas, mas em
 *  vez de machucar no contato ela arma uma espoleta (600 ms) quando a nave
 *  chega perto e explode em area. A explosao detona outras minas proximas
 *  (reacao em cadeia). Header-only: a detonacao em si fica no StateGame.
 */

#ifndef MINE_H_
#define MINE_H_

#include "PlanetRed.h"

class Mine: public PlanetRed
{
	public:
		int fuse; // ms ate explodir; < 0 = desarmada

		Mine(Sprite* sprite, float x, float y, float mundoW, float mundoH, TileMap* mapa):
			PlanetRed(sprite, x, y, 1, mundoW, mundoH,
			           15, 25, 0.3f, mapa, 12), fuse(-1) {}
		virtual ~Mine() {}

		virtual void update(int dt)
		{
			PlanetRed::update(dt);
			if(fuse >= 0)
			{
				fuse -= dt;
				if(fuse < 0)
					fuse = 0; // avisa o StateGame que e hora de detonar
			}
		}
};

#endif /* MINE_H_ */
